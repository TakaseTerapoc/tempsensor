#include <pigpio.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <libpq-fe.h>
#include <sstream>
#include <mutex>

#define MAX_TIMINGS 85 // タイムアウト時間
#define DHT_PIN1 4  // GPIOピン番号
#define DHT_PIN2 17  // GPIOピン番号

int failed_counter = 0; // エラーカウンタ
int total_counter = 0; // トータルカウンタ
std::mutex failed_counter_mutex; // エラーカウンタ用のミューテックス
std::mutex total_counter_mutex;
std::mutex db_mutex; // データベースアクセス用のミューテックス

void disconnect_db(PGconn *conn) {
    PQfinish(conn); // DBから切断
    exit(1);
}
bool check_sum(uint8_t j, std::vector<int>& data){
    if ((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))) {
        return true;
    } else {
        return false;
    }
}

bool read_dht_data(int dht_pin, std::vector<int>& data) {
    data.assign(5, 0);
    uint8_t laststate = PI_HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;

    // 開始信号の送信：ピンをLOWにしてセンサーに信号を送る
    gpioSetMode(dht_pin, PI_OUTPUT); //gpioSetMode()はピンのモードを設定する 0:INPUT 1:OUTPUT
    gpioWrite(dht_pin, PI_LOW); //gpioWrite()はピンに値を書き込む 0:LOW 1:HIGH
    gpioDelay(18000); // 18ms待つ
    gpioWrite(dht_pin, PI_HIGH);
    gpioDelay(40);
    gpioSetMode(dht_pin, PI_INPUT);

    // データの読み取り：センサーからデータが40ビットで送られてくるのでそれをキャッチ
    for (i = 0; i < MAX_TIMINGS; i++) {
        counter = 0;
        while (gpioRead(dht_pin) == laststate) {
            counter++;
            gpioDelay(1);
            if (counter == 255) {
                break;
            }
        }

        laststate = gpioRead(dht_pin);
        
        if (counter == 255) {
            break;
        }

        if ((i >= 4) && (i % 2 == 0)) {
            data[j / 8] <<= 1; // 1ビット左シフト
            if (counter > 16) {
                data[j / 8] |= 1; 
            }
            j++;
        }
    }
    if (check_sum(j, data)) {
        return true;
    } else {
        std::lock_guard<std::mutex> lock(failed_counter_mutex); // failed_counterへのアクセスを保護
        if (dht_pin == DHT_PIN1){
            std::cout << "センサー1のデータが不正です\n";
            failed_counter++;
            total_counter++;
            std::cout << failed_counter << "回目のエラー\n";
        } else {
            std::cout << "センサー2のデータが不正です\n";
            failed_counter++;
            total_counter++;
            std::cout << failed_counter << "回目のエラー\n";
        }

        return false;
    }
}

void insert_db(int dht_pin, const std::vector<int>& data) {
    float h = (float)((data[0] << 8) + data[1]) / 10;
    if (h > 100) {
        h = data[0];
    }
    float c = (float)(((data[2] & 0x7F) << 8) + data[3]) / 10;
    if (c > 125) {
        c = data[2];
    }
    if (data[2] & 0x80) {
        c = -c;
    }
    float f = c * 1.8f + 32;

    
    const char *conninfo;
    PGconn *conn;
    PGresult *res;
    std::stringstream ss;

    auto now = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(now);

    // PostgreSQLの接続情報を設定
    conninfo = "host=database-1.c9g8g0k0ekex.ap-northeast-1.rds.amazonaws.com port=1982 dbname=db_takase user=postgres password=postgres sslmode=require";

    // データベースに接続
    conn = PQconnectdb(conninfo);

    // 接続チェック
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection to database failed: " << PQerrorMessage(conn) << std::endl; // エラーメッセージを表示
        disconnect_db(conn); // DBから切断
    }

    if(dht_pin == DHT_PIN1) {
        std::cout << "センサー1のデータ\n";
        std::cout << "Current Time and Date: " << std::ctime(&end_time);
        std::cout << "湿度 = " << h << " %\t";
        std::cout << "温度 = " << c << " *C (" << f << " *F)" << std::endl;
        ss << "INSERT INTO dht_information (dht_id,temperature, humidity, measurement_date) VALUES (1,'" << c << "', '" << h << "',current_timestamp);";
    } else {
        std::cout << "センサー2のデータ\n";
        std::cout << "Current Time and Date: " << std::ctime(&end_time);
        std::cout << "湿度 = " << h << " %\t";
        std::cout << "温度 = " << c << " *C (" << f << " *F)" << std::endl;
        ss << "INSERT INTO dht_information (dht_id,temperature, humidity, measurement_date) VALUES (2,'" << c << "', '" << h << "',current_timestamp);";
    }

    std::lock_guard<std::mutex> lock(db_mutex); // データベースへのアクセスをシリアライズ
    res = PQexec(conn, ss.str().c_str());
    // 実行結果をチェック
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "INSERT command failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        disconnect_db(conn);
    }

    // 結果をクリア
    PQclear(res);
    // 接続を閉じる
    PQfinish(conn);
}

void sensor_thread(int dht_pin) {
    std::vector<int> data(5, 0); // スレッドごとのデータバッファを作成

    while (true) {
        if (read_dht_data(dht_pin, data)) {
            insert_db(dht_pin, data);
        }
        std::this_thread::sleep_for(std::chrono::seconds(5)); // 10秒待つ
    }
}

int main() {
    if (gpioInitialise() < 0) {
        std::cerr << "pigpioの初期化に失敗しました" << std::endl;
    }   

    std::thread th1(sensor_thread, DHT_PIN1);
    std::thread th2(sensor_thread, DHT_PIN2);

    th1.join();
    th2.join();

    gpioTerminate();

    return 0;
}

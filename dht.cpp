#include <pigpio.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <iostream>
#include <libpq-fe.h>
#include <sstream>
#include <thread>

#define MAX_TIMINGS 100 // タイムアウト時間
#define DHT_PIN1 4  // GPIOピン番号
#define DHT_PIN2 17  // GPIOピン番号

void disconnect_db(PGconn *conn) {
    PQfinish(conn);// DBから切断
    exit(1);
}

// dhtからの信号をを読み取る＆データベースにデータを挿入する関数
bool read_dht_data(int dht_pin) {

    std::vector<int> data(5, 0);
    data.assign(5, 0);
    uint8_t laststate = PI_HIGH;
    uint8_t counter = 0;
    int j = 0, i;
    std::stringstream ss;
    const char *conninfo;
    PGconn     *conn;
    PGresult   *res;
    // PostgreSQLの接続情報を設定
    conninfo = "host=database-1.c9g8g0k0ekex.ap-northeast-1.rds.amazonaws.com port=1982 dbname=db_takase user=postgres password=postgres sslmode=require";
    
    // データベースに接続
    conn = PQconnectdb(conninfo);  

    // 接続チェック
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection to database failed: " << PQerrorMessage(conn) << std::endl; // エラーメッセージを表示
        disconnect_db(conn); // DBから切断
    }
    
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
        // std::cout << "DHTPINの状態は" <<gpioRead(dht_pin)<< "です" << std::endl;
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
            data[j / 8] <<= 1;
            if (counter > 16) {
                data[j / 8] |= 1;
            }
            j++;
        }

    }
    
    if ((j >= 40) &&
        (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))) {
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
        auto now = std::chrono::system_clock::now();
        std::time_t end_time = std::chrono::system_clock::to_time_t(now);
        if(dht_pin == DHT_PIN1) {
            std::cout << "センサー1のデータが不正です\n";
        } else {
            std::cout << "センサー2のデータが不正です\n";
        }
        // 結果をクリア
        PQclear(res);
        // 接続を閉じる
        PQfinish(conn);
        return true;
    } else {
        if (dht_pin == DHT_PIN1){
            std::cerr << "センサー1のデータの読み取りに失敗しました" << std::endl;
        } else {
            std::cerr << "センサー2のデータの読み取りに失敗しました" << std::endl;
        }
        return false;
    }

}
void insert_db(int dht_pin, const std::vector<int>& data){
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
    auto now = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    const char *conninfo;
    PGconn     *conn;
    PGresult   *res;

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
    std::vector<int> data(5, 0);
    while (true) {
        if(read_dht_data(dht_pin,data)){
            insert_db(dht_pin,data);
        }
        gpioDelay(10000000); // 10秒待つ
    }
}

int main() {
    float temp1 = 0;
    float temp2 = 0;
    float hum1 = 0;
    float hum2 = 0;
    int i = 0;

    
    // pigpioの初期化
    if (gpioInitialise() < 0) {
        std::cerr << "pigpioの初期化に失敗しました" << std::endl;
        return 1;
    }
    // std::thread th1(read_dht_data,DHT_PIN1);
    // std::thread th2(read_dht_data,DHT_PIN2);

    while (1) {
        read_dht_data(DHT_PIN1);
        //     std::cerr << "センサー1のデータの読み取りに失敗しました" << std::endl;
        //     for(int i = 0; i < 5; i++){
        //         if(read_dht_data(DHT_PIN1)){
        //             break;
        //         }
        //     }
        // }
        read_dht_data(DHT_PIN2);
        gpioDelay(10000000);  // 30秒待つ
    }

    gpioTerminate(); // pigpioの終了
    return 0;
}

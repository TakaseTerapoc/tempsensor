#include <pigpio.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <iostream>
#include <libpq-fe.h>

// DBから切断する関数
void disconnect_db(PGconn *conn) {
    PQfinish(conn);// DBから切断
    exit(1);
}

int main() {
    const char *conninfo;
    PGconn     *conn;
    PGresult   *res;
    int nFields;
    int i, j;


    // PostgreSQLの接続情報を設定
    conninfo = "host=database-1.c9g8g0k0ekex.ap-northeast-1.rds.amazonaws.com port=1982 dbname=db_takase user=postgres password=postgres sslmode=require";
    // データベースに接続
    conn = PQconnectdb(conninfo);
    // 接続チェック
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection to database failed: " << PQerrorMessage(conn) << std::endl; // エラーメッセージを表示
        disconnect_db(conn); // DBから切断
    }
    // SQLコマンドを実行
    res = PQexec(conn, "SELECT * FROM dht_test;");
    // 実行結果をチェック
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "SELECT command failed: " << PQerrorMessage(conn) << std::endl; // エラーメッセージを表示
        PQclear(res); // 結果をクリア
        disconnect_db(conn); // DBから切断
    }

    // クエリ結果のフィールド数を取得
    nFields = PQnfields(res);// PQnfields()はフィールド数を返す

    // フィールド名を出力
    for (i = 0; i < nFields; i++) {
        std::cout << PQfname(res, i) << "\t"; //PQfname()はフィールド名を返す
    }
    std::cout << std::endl;

    // 各レコードのデータを出力
    for (i = 0; i < PQntuples(res); i++) { //PQntuples()はレコード数を返す
        for (j = 0; j < nFields; j++) {
            std::cout << PQgetvalue(res, i, j) << "\t"; //PQgetvalue()は指定したフィールドの値を返す
        }
        std::cout << std::endl;
    }

    // 結果をクリア
    PQclear(res);
    // 接続を閉じる
    PQfinish(conn);
    return 0;
}
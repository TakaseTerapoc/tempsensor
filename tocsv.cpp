#include <iostream>
#include <fstream>
#include <libpq-fe.h>

int main() {
    // DBに接続
    PGconn *conn = PQconnectdb("dbname=db_takase user=postgres password=postgres host=database-1.c9g8g0k0ekex.ap-northeast-1.rds.amazonaws.com port=1982");
    // 接続チェック
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Failed to connect to the database: " << PQerrorMessage(conn) << std::endl;// エラーメッセージを表示
        PQfinish(conn);
        return 1;
    }

    // クエリの実行
    PGresult *res = PQexec(conn, "SELECT * FROM dht");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Failed to execute the query: " << PQresultErrorMessage(res) << std::endl;
        PQclear(res);
        PQfinish(conn);
        return 1;
    }

    // CSVファイルを開く
    std::ofstream outputFile("output.csv");
    if (!outputFile) {
        std::cerr << "Failed to open the output file" << std::endl;
        PQclear(res);
        PQfinish(conn);
        return 1;
    }

    // CSVファイルにフィールド名を書き込む
    int numFields = PQnfields(res);
    for (int i = 0; i < numFields; i++) {
        outputFile << PQfname(res, i);
        if (i < numFields - 1) {
            outputFile << ",";
        }
    }
    outputFile << std::endl;

    // CSVファイルにデータを書き込む
    int numRows = PQntuples(res);
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numFields; j++) {
            outputFile << PQgetvalue(res, i, j);
            if (j < numFields - 1) {
                outputFile << ",";
            }
        }
        outputFile << std::endl;
    }

    // Cleanup
    outputFile.close();
    PQclear(res);
    PQfinish(conn);

    std::cout << "Data exported to output.csv" << std::endl;

    return 0;
}
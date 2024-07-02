#include <iostream>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define SERVER "180.43.182.57"

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char message[] = "Hello world!";

    // ソケットの作成
    sock = socket(AF_INET, SOCK_DGRAM, 0)
    if (sock < 0) {
        perror("ソケット作成に失敗しました");
        exit(EXIT_FAILURE);
    }

    // 構造体server_addrの初期化
    memset(&server_addr, 0, sizeof(server_addr));

    // サーバー情報の設定
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER, &server_addr.sin_addr) <= 0) {
        perror("無効なIPアドレスです");
        exit(EXIT_FAILURE);
    }

    // データの送信 引数1:socket 引数2:送るデータ 引数3:データの長さ 
    sendto(sock, (const char *)message, strlen(message), 0, (const struct sockaddr *) &server_addr, sizeof(server_addr));
    std::cout << "メッセージを送信しました." << std::endl;

    // ソケットのクローズ
    close(sock);
    return 0;
}
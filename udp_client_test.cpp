#include <iostream>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h> // close関数
#include <stdexcept>
#include <exception>
#include "UDPSender.h"


#define PORT 8080
#define SERVER "192.168.1.177"
#define MESSAGE "ラズベリーパイからのUDP通信ですよ!"

int main() {
    try {
        UDPSender sender(SERVER, PORT);
        while (true) {
            sender.send_data(MESSAGE);
            sleep(1);  // 1秒ごとにメッセージを送信
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
#ifndef UDPSENDER_H_
#define UDPSENDER_H_

#include <iostream>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h> // close関数

class UDPSender{
    private:
        int sock;
        struct sockaddr_in server_addr;
    public:
        UDPSender(const char* server_ip, int port) {
            sock = socket(AF_INET, SOCK_DGRAM, 0);
            if (sock < 0) {
                throw std::runtime_error("Socket creation failed");
            }
            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);

            if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
                close(sock);
                throw std::invalid_argument("Invalid IP address");
            }
        }
        
        ~UDPSender() {
            close(sock);
        }

        void send_data(const char* message);
};

#endif
#include "UDPSender.h"

void UDPSender::send_data(const char* message){
    if (sendto(this->sock, message, strlen(message), 0, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error("Failed to send message");
    }
    std::cout << "Message sent: " << message << std::endl;
}
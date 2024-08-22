#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    // Create a raw socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        return 1;
    }

    std::cout << "Raw socket created successfully." << std::endl;

    // Buffer to store received data
    char buffer[4096];

    while (true) {
        // Receive data
        ssize_t data_size = recv(sockfd, buffer, sizeof(buffer), 0);
        if (data_size < 0) {
            std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
            break;
        }

        // Process the received packet (in this example, we just print its size)
        std::cout << "Received packet of size: " << data_size << " bytes" << std::endl;

        // Here you would typically parse and process the packet data
    }

    // Close the socket
    close(sockfd);

    return 0;
}
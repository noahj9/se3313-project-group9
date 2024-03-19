#include <iostream>
#include <string>
#include <sstream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#define PORT 8080

void processClient(int new_socket) {
    char buffer[1024] = {0};
    std::string welcome = "Welcome to Spaceman Game Server";
    send(new_socket, welcome.c_str(), welcome.length(), 0);

    while (true) {
        memset(buffer, 0, 1024);
        int readVal = read(new_socket, buffer, 1024);
        if (readVal == 0) break; // Client closed connection

        std::string clientMessage = std::string(buffer).substr(0, readVal);
        std::cout << "Message from client: " << clientMessage << std::endl;

        // Respond to client
        if (clientMessage == "EXIT") {
            break; // Exit if client sends EXIT command
        } else {
            // Here you can add conditions to process different commands
            std::string response = "Received: " + clientMessage;
            send(new_socket, response.c_str(), response.length(), 0);
        }
    }

    close(new_socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (true) {
        std::cout << "Waiting for connections..." << std::endl;

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            continue; // Continue to the next iteration if accept fails
        }

        processClient(new_socket);
    }

    return 0;
}

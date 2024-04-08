#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

int main() {
    #ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << "\n";
        return 1;
    }
    #endif

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket.\n";
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(2003);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed.\n";
        return 1;
    }

    std::cout << "Connected to the server!\nEnter 'create' to create a game room.\nEnter 'get' to get all active rooms\n";

    std::string command;
    std::cin >> command;

    if (command == "create") {
        const std::string createRoomCmd = "CREATE_ROOM";
        send(sock, createRoomCmd.c_str(), createRoomCmd.size(), 0);

        // Now wait for the server's response
        char buffer[1024] = {0};
        int bytesRead = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesRead > 0) {
            std::cout << "Server response: \n" << std::string(buffer, bytesRead) << "\n";
        }
    } else if (command == "get") {
        const std::string createRoomCmd = "GET_ACTIVE_ROOMS";
        send(sock, createRoomCmd.c_str(), createRoomCmd.size(), 0);

        // Now wait for the server's response
        char buffer[1024] = {0};
        int bytesRead = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesRead > 0) {
            std::cout << "Server response: \n" << std::string(buffer, bytesRead) << "\n";
        }
    }

    #ifdef _WIN32
    closesocket(sock);
    WSACleanup();
    #else
    close(sock);
    #endif

    return 0;
}

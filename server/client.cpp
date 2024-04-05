#include <iostream>
#include <thread>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

constexpr int SERVER_PORT = 8080; // server SERVER_PORT

void handleServerResponse(int clientSocket)
{
    // get server response
    char buffer[1024];
    int bytesRead = read(clientSocket, buffer, sizeof(buffer));
    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0'; // parse the data and print active rooms
        std::cout << "Active Game Rooms:\n"
                  << buffer << std::endl;
    }
    else
    {
        std::cerr << "Error reading server response\n";
    }
}

int main()
{
    // new socket to connect to server
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        std::cerr << "Error creating client socket\n";
        return 1;
    }

    // make connection to server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // loopback address
    serverAddr.sin_port = htons(SERVER_PORT);            // port assignment

    if (connect(clientSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1)
    {
        std::cerr << "Error connecting to server\n";
        close(clientSocket);
        return 1;
    }

    // Request to get active game rooms
    std::string request = "GET_ACTIVE_ROOMS";
    write(clientSocket, request.c_str(), request.size());

    // handle response in separate thread
    std::thread responseThread(handleServerResponse, clientSocket);
    responseThread.join(); // join back the thread when it finishes executing

    // close the socket connection
    close(clientSocket);

    return 0;
}

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <string>
#include <atomic>
#include <future>
#include "Gameroom.h" // Include the Gameroom class header file

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

constexpr int PORT = 8080;

// Global variable for active gamerooms
std::vector<Gameroom> activeGameRooms;
int roomCounter = 0;

// Mutex for synchronizing access to activeGameRooms
std::mutex gameRoomsMutex;

std::string getActiveRooms()
{
    std::string roomList;
    std::lock_guard<std::mutex> lock(gameRoomsMutex);
    for (const auto &room : activeGameRooms)
    {
        // Assume Gameroom has a method to retrieve its name and number of players
        roomList += "Room Name: " + room.name + ", Players: " + std::to_string(room.clients.size()) + "\n";
    }
    return roomList;
}

void joinGameRoom(std::string roomName, int clientSocket)
{
    // find a room based on its name
    std::lock_guard<std::mutex> lock(gameRoomsMutex);
    for (auto &room : activeGameRooms)
    {
        if (room.name == roomName)
        {
            room.acceptClient(clientSocket);
            return;
        }
    }

    // If the desired room is not found, you can handle the error or take appropriate action
    std::cerr << "Desired room not found\n";
}

void createGameRoom(int clientSocket)
{
    // Create a new gameroom and add it to the list of active gamerooms
    std::lock_guard<std::mutex> lock(gameRoomsMutex);
    activeGameRooms.emplace_back("Room_" + roomCounter);
    roomCounter++;

    // Spawn a new thread for the game room
    std::thread roomThread(&Gameroom::acceptClient, &activeGameRooms.back(), clientSocket);
    roomThread.detach();
    std::cout << "New room created " << roomCounter - 1 << std::endl;
}

void handleClient(int clientSocket)
{
    char buffer[1024];
    int bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        std::string request(buffer, bytesRead);

        if (request == "GET_ACTIVE_ROOMS")
        {
            std::string roomList = getActiveRooms();
            send(clientSocket, roomList.c_str(), roomList.size(), 0);
        }
        else if (request.find("CREATE_ROOM") == 0)
        {
            createGameRoom(clientSocket);
        }
        else if (request.find("JOIN_ROOM") == 0)
        {
            // Extract room name from request
            size_t pos = request.find(" ");
            if (pos != std::string::npos)
            {
                std::string roomName = request.substr(pos + 1);
                joinGameRoom(roomName, clientSocket); // Pass the client socket
            }
        }
        else if (request == "LEAVE_ROOM")
        {
            // Implement leaving a gameroom
        }
        else
        {
            std::cerr << "Invalid request from client.\n";
        }
    }

#ifdef _WIN32
    closesocket(clientSocket);
#else
    close(clientSocket);
#endif
}

int main()
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }
#endif

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Error creating server socket\n";
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1)
    {
        std::cerr << "Error binding server socket\n";
#ifdef _WIN32
        closesocket(serverSocket);
#else
        close(serverSocket);
#endif
        return 1;
    }

    if (listen(serverSocket, 10) == -1)
    {
        std::cerr << "Error listening on server socket\n";
#ifdef _WIN32
        closesocket(serverSocket);
#else
        close(serverSocket);
#endif
        return 1;
    }

    std::cout << "Server listening on port: " << PORT << std::endl;

    while (true)
    {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrSize);
        if (clientSocket == -1)
        {
            std::cerr << "Error accepting client connection\n";
            continue;
        }

        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}

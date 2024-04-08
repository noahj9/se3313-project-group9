#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <string>
#include <atomic>
#include <future>
#include "Gameroom.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

constexpr int PORT = 2003;

// Global variable for active gamerooms
std::vector<Gameroom> activeGameRooms;
int roomCounter = 0;
std::unordered_map<std::string, User> globalUsers;
std::mutex usersMutex;
std::atomic<bool> stopRequested(false);
int numberOfUsers = 0;

// Mutex for synchronizing access to activeGameRooms
std::mutex gameRoomsMutex;

std::string initializeUser () {
    // Create a new user object
    // Initialize balance to $10, bet amount to 0, and inGame flag to false --> Call the damn constructor 
    // ** User id = numberOfUsers --> make it a string **
    // numberOfUsers += 1
    // Send user id back to client

    std::lock_guard<std::mutex> lock(usersMutex);

    // return user's id
}

std::string getActiveRooms() {
    std::string roomList;
    std::lock_guard<std::mutex> lock(gameRoomsMutex);
    for (const auto &room : activeGameRooms) {
        // Assume Gameroom has a method to retrieve its name and number of players
        roomList += "Room Name: " + room.name + ", Players: " + std::to_string(room.clients.size()) + "";

        // Testing
        roomList += "Player names: ";
        for (int clientSocket : room.clients) {
            roomList += std::to_string(clientSocket) + " ";
        }
        roomList += "\n";
        // End testing
    }
    std::cout << "Active rooms response: " << roomList << std::endl;
    return roomList;
}

void joinGameRoom(std::string roomName, std::string userId) {
    // Joining an ALREADY created room (existing)
    // If the desired room is found, add the client to the room by adding their USER ID to the room's clients list

    // find a room based on its name
    std::lock_guard<std::mutex> lock(gameRoomsMutex);
    for (auto &room : activeGameRooms) {
        if (room.name == roomName) {
            // TODO: Change this to be the USER ID
            room.acceptClient(userId);
            return;
        }
    }

    // If the desired room is not found, you can handle the error or take appropriate action
    std::cerr << "Desired room not found\n";
}

void leaveRoom(std::string roomName, std::string userId) {
    // Leaving an ALREADY created room (existing)
    // If the desired room is found, remove the client from the room by removing their USER ID from the room's clients list

    // find a room based on its name
    std::lock_guard<std::mutex> lock(gameRoomsMutex);
    for (auto &room : activeGameRooms) {
        if (room.name == roomName) {
            // TODO: Change this to be the USER ID
            room.removeClient(userId);
            return;
        }
    }

    // If the desired room is not found, you can handle the error or take appropriate action
    std::cerr << "Desired room not found\n";
}

void createGameRoom(int clientSocket) {
    // Create a new gameroom and add it to the list of active gamerooms
    std::lock_guard<std::mutex> lock(gameRoomsMutex);
    std::string roomName = "Room_" + std::to_string(roomCounter++);
    activeGameRooms.emplace_back(roomName);

    // Spawn a new thread for the game room
    std::thread roomThread(&Gameroom::acceptClient, &activeGameRooms.back(), clientSocket);
    roomThread.detach();
    std::cout << "New room created " << roomCounter - 1 << std::endl;
}

void handleClient(int clientSocket) {
    char buffer[1024];
    int bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        std::string request(buffer, bytesRead);

        if (request == "GET_ACTIVE_ROOMS") {
            std::string roomList = getActiveRooms();
            send(clientSocket, roomList.c_str(), roomList.size(), 0);
        } else if (request.find("CREATE_ROOM") == 0) {
            createGameRoom(clientSocket);
        } else if (request.find("JOIN_ROOM") == 0) {
            // Extract room name from request
            size_t pos = request.find(" ");
            if (pos != std::string::npos) {
                std::string roomName = request.substr(pos + 1);
                std::string userId = request.substr(pos + 2);
                // TODO: Change to the user ID
                joinGameRoom(roomName, userId); // Pass the client socket
            }
        }
        else if (request == "LEAVE_ROOM") {
            // TODO: Implement leave room method
            // We would expect this to be something like the below:

            size_t pos = request.find(" ");
            if (pos != std::string::npos) {
                std::string roomName = request.substr(pos + 1);
                std::string userId = request.substr(pos + 2);
                // TODO: Change to the user ID
                leaveRoom(roomName, userId)
            }
        } else {
            std::cerr << "Invalid request from client.\n";
        }
    }

    #ifdef _WIN32
        closesocket(clientSocket);
    #else
        close(clientSocket);
    #endif
}

int main() {
    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed\n";
            return 1;
        }
    #endif

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating server socket\n";
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding server socket\n";
        #ifdef _WIN32
                closesocket(serverSocket);
        #else
                close(serverSocket);
        #endif
        return 1;
    }

    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error listening on server socket\n";
        #ifdef _WIN32
                closesocket(serverSocket);
        #else
                close(serverSocket);
        #endif
        return 1;
    }

    std::cout << "Server listening on port: " << PORT << std::endl;

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrSize);
        if (clientSocket == -1) {
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

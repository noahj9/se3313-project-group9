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
std::atomic<bool> serverRunning(true);
int roomCounter = 0;
std::unordered_map<std::string, User> globalUsers;
std::mutex usersMutex;
std::atomic<bool> stopRequested(false);
int numberOfUsers = 0;

// Mutex for synchronizing access to activeGameRooms
std::mutex gameRoomsMutex;

void printActiveThreads() // log all active threasds
{
    std::lock_guard<std::mutex> lock(gameRoomsMutex);
    std::cout << "Active Threads:" << std::endl;
    for (const auto &thread : activeGameRooms)
    {
        std::cout << "Thread ID: " << thread.name << std::endl;
    }
}

std::string initializeUser(int clientSocket)
{
    std::lock_guard<std::mutex> lock(usersMutex);
    std::string userId = std::to_string(numberOfUsers++); // Convert integer ID to string

    User newUser(userId, clientSocket);
    globalUsers[userId] = newUser;
    return userId;
}

std::string getActiveRooms()
// then we need to parse this into string list
// Room name: Room_1, Players: 2 : example
{
    std::string roomList;
    std::lock_guard<std::mutex> lock(gameRoomsMutex);
    for (const auto &room : activeGameRooms)
    {
        // Assume Gameroom has a method to retrieve its name and number of players
        roomList += room.name + " ";
    }
    std::cout << "Active rooms response: " << roomList << std::endl;
    return roomList;
}

void joinGameRoom(std::string roomName, std::string userId)
{
    // Joining an ALREADY created room (existing)
    // If the desired room is found, add the client to the room by adding their USER ID to the room's clients list
    // we should make sure the room exists
    // we should send a confirmation message back to the client "you have connected to room_# successfully"
    std::lock_guard<std::mutex> lock(gameRoomsMutex);
    int clientSocket = globalUsers[userId].socket; // must make sure this works

    for (auto &room : activeGameRooms)
    {
        if (room.name == roomName)
        {
            // Assuming you have a function to send a message to a client by userId
            room.acceptClient(userId);
            std::cout << "Client joined room: " << roomName << std::endl;

            // std::string joinResponse = "You have successfully joined " + roomName;
            // send(clientSocket, joinResponse.c_str(), joinResponse.size(), 0);
            return;
        }
    }
    std::cerr << "Desired room not found for joinGameRoom function. Room name given: " << roomName << ". \n";
}

void cashout(std::string roomName, std::string userId, std::string multiplier)
{
    // find a room based on its name
    std::lock_guard<std::mutex> lock(gameRoomsMutex);
    for (auto &room : activeGameRooms)
    {
        if (room.name == roomName)
        {
            room.cashoutForUser(userId, std::stod(multiplier));
            return;
        }
    }

    // If the desired room is not found, you can handle the error or take appropriate action
    std::cerr << "Desired room not found for cashout function. Room name: " << roomName << "\n";
}

void bet(std::string roomName, std::string userId, std::string betAmount)
{
    // find a room based on its name
    std::lock_guard<std::mutex> lock(gameRoomsMutex);
    for (auto &room : activeGameRooms)
    {
        if (room.name == roomName)
        {
            room.placeUserBet(userId, std::stod(betAmount));
            return;
        }
    }

    // If the desired room is not found, you can handle the error or take appropriate action
    std::cerr << "Desired room not found for bet function. Room name: " << roomName << "\n";
}

void leaveRoom(std::string roomName, std::string userId)
{
    // Leaving an ALREADY created room (existing)
    // If the desired room is found, remove the client from the room by removing their USER ID from the room's clients list
    // we should send a message to the client that "you have left room_# successfully"

    // find a room based on its name
    std::lock_guard<std::mutex> lock(gameRoomsMutex);
    for (auto &room : activeGameRooms)
    {
        if (room.name == roomName)
        {
            room.removeClient(userId);
            return;
        }
    }

    // If the desired room is not found, you can handle the error or take appropriate action
    std::cerr << "Desired room not found for leaveRoom function\n";
}

void gameRoomThread(Gameroom &room)
{
    // Run the game room operations
    room.gameCaller();
}

void createGameRoom(std::string userId)
{
    // Create a new gameroom and add it to the list of active gamerooms
    printActiveThreads();
    std::lock_guard<std::mutex>
        lock(gameRoomsMutex);
    std::string roomName = "Room_" + std::to_string(roomCounter++);
    activeGameRooms.emplace_back(roomName);

    std::cout << "New room created " << roomName << std::endl;

    // Start a new thread for the game room
    std::thread roomThread(gameRoomThread, std::ref(activeGameRooms.back()));
    roomThread.detach(); // Detach the thread to run independently
}

void handleClient(std::string userId)
{
    int clientSocket = globalUsers[userId].socket; // must make sure this works
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
            createGameRoom(userId); // changes clientSocket to userId
        }
        else if (request.find("JOIN_ROOM") == 0)
        {
            size_t pos = request.find(" ");
            size_t firstSpacePos = request.find(" ", pos + 1);
            size_t secondSpacePos = request.find(" ", firstSpacePos + 1);

            std::cout << "Joining room request: " << request << std::endl;
            if (pos != std::string::npos && firstSpacePos != std::string::npos)
            {
                std::string roomName = request.substr(pos + 1, firstSpacePos - pos - 1);
                std::string userId = request.substr(firstSpacePos + 1, secondSpacePos - firstSpacePos - 1);

                std::cout << "Joining room: " << roomName << " with user ID: " << userId << std::endl;

                joinGameRoom(roomName, userId); // Pass the client socket
            }
        }
        else if (request.find("CASHOUT") == 0)
        {
            size_t pos = request.find(" ");
            size_t firstSpacePos = request.find(" ", pos + 1);
            size_t secondSpacePos = request.find(" ", firstSpacePos + 1);

            if (pos != std::string::npos && firstSpacePos != std::string::npos)
            {
                std::string roomName = request.substr(pos + 1, firstSpacePos - pos - 1);
                std::string userId = request.substr(firstSpacePos + 1, secondSpacePos - firstSpacePos - 1);
                std::string multiplier = request.substr(secondSpacePos + 1);
                cashout(roomName, userId, multiplier); // Pass the client socket
            }
        }
        else if (request.find("BET") == 0)
        {
            size_t pos = request.find(" ");
            size_t firstSpacePos = request.find(" ", pos + 1);
            size_t secondSpacePos = request.find(" ", firstSpacePos + 1);
            size_t thirdSpacePos = request.find(" ", secondSpacePos + 1);

            if (pos != std::string::npos && firstSpacePos != std::string::npos)
            {
                std::string roomName = request.substr(pos + 1, firstSpacePos - pos - 1);
                std::string userId = request.substr(firstSpacePos + 1, secondSpacePos - firstSpacePos - 1);
                std::string betAmount = request.substr(secondSpacePos + 1);

                std::cout << "Placing bet in room: " << roomName << " with user ID: " << userId << " and bet amount: " << betAmount << std::endl;

                bet(roomName, userId, betAmount); // Pass the client socket
            }
        }
        else if (request == "LEAVE_ROOM")
        {
            // We would expect this to be something like the below:

            size_t pos = request.find(" ");
            size_t nextSpacePos = request.find(" ", pos + 1);

            if (pos != std::string::npos && nextSpacePos != std::string::npos)
            {
                std::string roomName = request.substr(pos + 1, nextSpacePos - pos - 1);
                std::string userId = request.substr(nextSpacePos + 1);
                leaveRoom(roomName, userId);
            }
        }
        else if (request == "INITIALIZE_USER")
        {
            // Send the userId back to the client
            std::string initResponse = userId;
            send(clientSocket, initResponse.c_str(), initResponse.length(), 0);
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

void acceptClients(int serverSocket)
{

    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    while (serverRunning)
    {
        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrSize);
        if (clientSocket == -1)
        {
            if (!serverRunning)
            {
                std::string msg = "SHUTDOWN";
                send(clientSocket, msg.c_str(), msg.length(), 0);
                std::cerr << "Server shutting down." << std::endl;
                break; // Exit loop if server is stopping
            }
            std::cerr << "Error accepting client connection\n";
            continue;
        }
        std::string userId = initializeUser(clientSocket);
        std::thread clientThread(handleClient, userId);
        clientThread.detach();
    }
}

int main()
{
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

    /* while (true)
    {
        sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrSize);
        if (clientSocket == -1)
        {
            std::cerr << "Error accepting client connection\n";
            continue;
        }
        // Initialize user and get their userId
        std::string userId = initializeUser(clientSocket);

        // Pass userId to the thread handling client communication
        std::thread clientThread(handleClient, userId);

        clientThread.detach();
    }  */

    std::thread acceptThread(acceptClients, serverSocket);

    // Main loop waits for the shutdown command
    char command = '\0';
    while (command != 'e')
    {
        command = std::getchar();
    }

    // Signal server to stop and close the server socket
    serverRunning = false;

    // Shutdown the listening socket to ensure no new accept calls can be made
    shutdown(serverSocket, SHUT_RDWR);
    close(serverSocket); // Will cause accept to return with an error

    // Wait for the accept thread to finish
    acceptThread.join();

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
#include <iostream>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

constexpr int PORT = 8080;
struct GameRoom
{
    std::string name; // Changed int to std::string
    int numPlayers;
};

std::vector<GameRoom> activeGameRooms;

std::string getActiveRooms()
{
    std::string roomList;
    for (const auto &room : activeGameRooms)
    {
        roomList += "Room Name: " + room.name + ", Players: " + std::to_string(room.numPlayers) + "\n";
    }
    return roomList;
}

GameRoom createGameRoom(std::string roomName)
{
    GameRoom newRoom;
    newRoom.name = roomName;
    newRoom.numPlayers = 1;
    activeGameRooms.push_back(newRoom);
    return newRoom;
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
            size_t pos = request.find(" ");
            if (pos != std::string::npos)
            {
                std::string roomName = request.substr(pos + 1);
                GameRoom newRoom = createGameRoom(roomName);
                newRoom.numPlayers = 1;
                activeGameRooms.push_back(newRoom);
            }
        }
        else if (request.find("JOIN_ROOM") == 0)
        {
            size_t pos = request.find(" ");
            if (pos != std::string::npos)
            {
                std::string roomName = request.substr(pos + 1);
            }
        }
        else if (request == "LEAVE_ROOM")
        {
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
        socklen_t clientAddrSize = sizeof(clientAddr);
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

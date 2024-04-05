#include <iostream>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

constexpr int PORT = 8080;

struct GameRoom
{
    int name;
    int numPlayers;
};

std::vector<GameRoom> activeGameRooms; // vector list to store all active game rooms

// get all active game rooms to send to client
std::string getActiveRooms()
{
    std::string roomList;
    for (const auto &room : activeGameRooms)
    {
        roomList += "Room ID: " + std::to_string(room.id) + ", Players: " + std::to_string(room.numPlayers) +
                    "/" + std::to_string(room.maxCapacity) + "\n";
    }
    return roomList;
}

// handle incoming client connections
void handleClient(int clientSocket)
{
    char buffer[1024];
    int bytesRead;

    while ((bytesRead = read(clientSocket, buffer, sizeof(buffer))) > 0)
    {
        std::string request(buffer, bytesRead);

        if (request == "GET_ACTIVE_ROOMS") // respond to client request for a list of open game rooms
        {
            std::string roomList = getActiveRooms();
            write(clientSocket, roomList.c_str(), roomList.size());
        }
        else if
        {
            // logic for other requests
        }
    }

    // Cclose the socker connection
    close(clientSocket);
}

int main()
{
    // create a dedicated socket for clients to join
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Error creating server socket\n";
        return 1;
    }

    // bind socket to port 8080
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1)
    {
        std::cerr << "Error binding server socket\n";
        close(serverSocket);
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 10) == -1)
    {
        std::cerr << "Error listening on server socket\n";
        close(serverSocket);
        return 1;
    }

    std::cout << "Server listening on port: " << PORT << std::endl;

    // loop to continually accept new connections
    while (true)
    {
        // accept a new client
        sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrSize);
        if (clientSocket == -1)
        {
            std::cerr << "Error accepting client connection\n";
            continue;
        }

        // spawn a client thread to handle the new connection
        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach(); // Detach the thread to allow it to run independently
    }

    // Close the server socket (this code is unreachable in this example)
    close(serverSocket);

    return 0;
}

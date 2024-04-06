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
        roomList += "Room Name: " + std::to_string(room.name) + ", Players: " + std::to_string(room.numPlayers) + "\n";
    }
    return roomList;
}

GameRoom createGameRoom(std::string roomName) // this needs to create a new thread, send the client to it, and new instance of spaceman game
{
    GameRoom newRoom;
    newRoom.name = roomName;
    newRoom.numPlayers = 1;
    activeGameRooms.push_back(newRoom);
    return newRoom;
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
        else if (request.find("CREATE_ROOM") == 0) // create a new game room
        {
            // parse the room name from the request
            size_t pos = request.find(" "); // find first space
            if (pos != std::string::npos)
            {
                std::string roomName = request.substr(pos + 1); // find room name after space

                // add new room to the list of active game rooms
                GameRoom newRoom = createGameRoom(roomName);
                // call function to make player join the new game room
                // joinRoom(roomName, clientSocket); //joing the player into the newwly created room
                newRoom.numPlayers = 1;
                activeGameRooms.push_back(newRoom);
            }
        }
        else if (request.find("JOIN_ROOM") == 0)
        {
            // parse the room name from the request
            size_t pos = request.find(" "); // find first space
            if (pos != std::string::npos)
            {
                std::string roomName = request.substr(pos + 1); // find room name after space

                // call function to handle join room request
                // joinRoom(roomName, clientSocket);
            }
        }
        else if (request == "LEAVE_ROOM")
        {
            // needs to know which room the client wants to leave
        }
        else
        {
            std::cerr << "Invalid request from client.\n";
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
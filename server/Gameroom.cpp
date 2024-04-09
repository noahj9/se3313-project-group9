#include "Gameroom.h"
#include "User.h"
#include "Utils.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring> // For memset
#include <string>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#ifdef _WIN32
#include <winsock2.h> // For send
#else
#include <unistd.h> // For write
#endif

extern std::unordered_map<std::string, User> globalUsers;
extern std::mutex usersMutex;
extern std::atomic<bool> stopRequested;

// Constructor with parameter
Gameroom::Gameroom() : gameInProgress(false), name("") {}
Gameroom::Gameroom(const std::string &roomName) : gameInProgress(false), name(roomName) {}
Gameroom::Gameroom(Gameroom &&other) : gameInProgress(false), name(other.name), clients(other.clients) {}

// This function WAS to ensure a second thread was used to ensure the COMMAND LINE game could be played.
// Likely do not need this function in the final implementation.
void handleUserInput(std::atomic<bool> &stopRequested, std::function<void(std::string)> stopFunction)
{
    std::string userInput;
    while (!stopRequested)
    {
        if (std::getline(std::cin, userInput))
        { // This will still block, but now it's in a separate thread
            std::istringstream iss(userInput);
            std::string command, userId;
            iss >> command >> userId;
            if (command == "stop")
            {
                stopFunction(userId);
            }
        }
    }
}

void Gameroom::acceptClient(std::string userId)
{
    std::lock_guard<std::mutex> lock(gameMutex); // Lock for thread safety
    // add userID to list of users in this game room
    clients.push_back(userId);

    std::cout << "Clients in the game room: " << name << std::endl;
    for (const auto &client : clients)
    {
        std::cout << client << std::endl;
    }
}

void Gameroom::removeClient(std::string userId)
{
    std::lock_guard<std::mutex> lock(gameMutex); // Lock for thread safety

    // similar logic to acceptClient, but remove the client socket from the list of userIds in the game room
    // Find the userID in the vector
    auto it = std::find(clients.begin(), clients.end(), userId);
    clients.erase(it);
}

void Gameroom::cashoutForUser(std::string userId, double multiplier)
{
    std::lock_guard<std::mutex> lock(gameMutex); // Lock for thread safety

    // Check if this userId is in the list of user IDs of this gameroom
    auto it = std::find(clients.begin(), clients.end(), userId);
    if (it == clients.end())
    {
        // If userId is not in this game room's list of clients, just return;
        std::cerr << "User " << userId << " not in the game room: " << name << std::endl;
        return;
    }

    // Safely access the globalUsers map to get the user's details
    std::lock_guard<std::mutex> usersLock(usersMutex);
    auto userIt = globalUsers.find(userId);
    if (userIt == globalUsers.end())
    {
        std::cerr << "User " << userId << " not found in global users." << std::endl;
        return;
    }

    // Calculate the amount won and update the user's balance
    User &user = userIt->second; // Get a reference to the user to modify directly
    double amountWon = user.betAmount * multiplier;
    user.balance += amountWon;

    // Reset the user's bet amount to indicate they've cashed out
    user.betAmount = 0;

    std::cout << "User " << userId << " cashed out for a value of $" << amountWon << std::endl;
    std::cout << "User " << userId << " now has a total balance of $" << user.balance << std::endl;
}

// Use this to check if a user is in a game
bool Gameroom::anyUserInGame() const
{
    return clients.size() > 0;
}

// Starts a new game, resetting necessary components and starting the multiplier increase
void Gameroom::startGame()
{
    std::cout << gameInProgress << " " << anyUserInGame() << std::endl;
    if (!gameInProgress && anyUserInGame())
    {
        std::cout << "Starting a new game..." << std::endl;
        gameInProgress = true;
        stopRequested = false;

        for (const auto &userId : clients)
        {
            // Debugging possible segmentation fault here, index out of bounds?
            // TODO
            auto it = globalUsers.find(userId);
            if (it != globalUsers.end())
            {
                int clientSocket = it->second.socket;

                send(clientSocket, "START_GAME", strlen("START_GAME"), 0);
                std::cout << "Sent START_GAME message to user " << userId << " at client socket: " << clientSocket << " for room: " << name << std::endl;
            }
            else
            {
                std::cerr << "User " << userId << " not found in global users." << std::endl;
            }
        }
    }
}

void Gameroom::gameCaller()
{
    sleep(10);
    startGame();
    while (gameInProgress)
    {
        int num = rand() % 100;
        sleep(1);
        if ((num < 5))
        {
            endGame();
            break;
        }
    }
    stopRequested = true;
    gameInProgress = false;
    gameCaller();
}

// Ends the current game, calculating and updating user balances
void Gameroom::endGame()
{
    std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
    for (auto &[id, user] : globalUsers)
    {
        if (user.inGame)
        {
            user.reset(); // Reset user state for the next game
        }
    }
    gameInProgress = false;
    for (const auto &userId : clients)
    {
        int clientSocket = globalUsers[userId].socket;
        send(clientSocket, "END_GAME", strlen("END_GAME"), 0);
        send(clientSocket, ("BALANCE " + std::to_string(globalUsers[userId].balance)).c_str(),
             ("BALANCE " + std::to_string(globalUsers[userId].balance)).size(), 0);
    }
    std::cout << "Game ended." << std::endl;
}

// Returns whether a game is currently in progress
bool Gameroom::isGameInProgress() const
{
    return gameInProgress;
}

// Allows a user to place a bet, given their ID and bet amount
void Gameroom::placeUserBet(const std::string &userId, double betAmount)
{
    std::lock_guard<std::mutex> lock(usersMutex); // Ensure thread safety when accessing globalUsers

    auto it = globalUsers.find(userId);
    if (it == globalUsers.end())
    {
        std::cout << "User " << userId << " not found." << std::endl;
        return;
    }

    if (gameInProgress)
    {
        std::cout << "Cannot place bet, the game is currently in progress." << std::endl;
        return;
    }

    // Check if the user has enough balance and if the bet amount is positive
    if (betAmount <= 0)
    {
        std::cout << "User " << userId << " cannot place a bet of $" << betAmount << ". Bet must be greater than $0." << std::endl;
        return;
    }

    if (globalUsers[userId].balance < betAmount)
    {
        std::cout << "User " << userId << " does not have enough balance to place a bet of $" << betAmount << "." << std::endl;
        std::cout << "Current balance: $" << globalUsers[userId].balance << std::endl;
        return;
    }

    // At this point, all checks have passed, and the user can place a bet
    globalUsers[userId].balance -= betAmount;   // Assuming you want to deduct the bet amount from the user's balance
    globalUsers[userId].betAmount += betAmount; // Track the total bet amount for this game
    globalUsers[userId].inGame = true;          // Mark the user as participating in the game
    std::cout << "User " << userId << " placed a bet of $" << betAmount << ". New balance: $" << globalUsers[userId].balance << std::endl;
}

// List all users in the gameroom
void Gameroom::listAllUsers() const
{
    // change to list all users IN THIS GAME ROOM, rather than all users connected to the server
    std::lock_guard<std::mutex> lock(usersMutex);

    if (globalUsers.empty())
    {
        std::cout << "There are no users in the game room." << std::endl;
        return;
    }

    std::cout << "Listing all users in the game room:" << std::endl;
    for (const auto &pair : globalUsers)
    {
        const auto &userId = pair.first;
        const auto &user = pair.second;
        std::cout << "User ID: " << userId
                  << ", Balance: $" << user.balance
                  << ", In Game: " << (user.inGame ? "Yes" : "No")
                  << std::endl;
    }
}

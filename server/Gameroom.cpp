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
Gameroom::Gameroom() : multiplier(1.0), gameInProgress(false), name("") {}
Gameroom::Gameroom(const std::string &roomName) : multiplier(1.0), gameInProgress(false), name(roomName) {}
Gameroom::Gameroom(Gameroom &&other) : multiplier(1.0), gameInProgress(false), name(other.name), clients(other.clients) {}

// TODO: Change this to accept the client's BUTTON PRESS ("CASHOUT")
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

// TODO: Change to accept user ID
void Gameroom::acceptClient(std::string userId)
{
    std::lock_guard<std::mutex> lock(gameMutex); // Lock for thread safety
    // add userID to list of users in this game room
    clients.push_back(userId);

    // TODO: Get the client's socket from the "globalUsers"
    int clientSocket = globalUsers[userId].socket;

    // send a socket connection msg to client saying they joined properly
    std::string message = "You have successfully joined the game room: " + name + "\n";
    if (send(clientSocket, message.c_str(), message.length(), 0) == -1)
    {
        std::cerr << "Error sending join message to client\n";
    }
}

// TODO: Change to accept user ID
void Gameroom::removeClient(std::string userId)
{
    std::lock_guard<std::mutex> lock(gameMutex); // Lock for thread safety

    // similar logic to acceptClient, but remove the client socket from the list of userIds in the game room
    // Find the userID in the vector
    auto it = std::find(clients.begin(), clients.end(), userId);
    clients.erase(it);
}

void Gameroom::cashoutForUser(std::string userId)
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
}

// Use this to check if a user is in a game
bool Gameroom::anyUserInGame() const
{
    return clients.size() > 0;
}

// Starts a new game, resetting necessary components and starting the multiplier increase
void Gameroom::startGame()
{
    if (!gameInProgress && anyUserInGame())
    {
        std::cout << "Starting a new game... Type 'stop <userID>' to secure your bet at the current multiplier." << std::endl;
        gameInProgress = true;
        multiplier = 1.0;
        stopRequested = false;

        for (const auto &userId : clients)
        {
            int clientSocket = globalUsers[userId].socket;
            send(clientSocket, "START_GAME", strlen("START_GAME"), 0);
            std::cout << "Sent START_GAME message to user " << userId << "at client socket: " << clientSocket << std::endl;
        }

        // Pass the member function and the object for which it will be called
        std::future<void> userInputHandler = std::async(std::launch::async, handleUserInput, std::ref(stopRequested), [this](std::string userId)
                                                        { this->userStops(userId); });

        // Game loop starts here
        while (gameInProgress)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));           // Simulate time passing
            multiplier += 0.1;                                              // Increment the multiplier over time
            std::cout << "Current multiplier: " << multiplier << std::endl; // Display the current multiplier

            // The game continues running here. Since std::getline is blocking, we removed it from this loop.
            // The handling of 'stop' commands is done asynchronously in handleUserInput.

            if ((rand() % 100) < 5)
            {              // There's a random chance to end the game
                endGame(); // End the game if the random condition is met
                break;     // Exit the while loop since the game has ended
            }
        }

        stopRequested = true;   // Signal the user input thread to stop
        userInputHandler.get(); // Wait for the user input handling thread to finish

        gameInProgress = false; // Mark the game as no longer in progress
    }
    else
    {
        if (gameInProgress)
        {
            std::cout << "Cannot start a new game. A game is already in progress." << std::endl;
        }
        else
        {
            std::cout << "Cannot start a new game. No globalUsers are ready." << std::endl;
        }
    }
}

// THIS TO THE CASHOUT FUNCTION **
// TODO: When the user clicks the cashout button, this function is called
void Gameroom::userStops(const std::string &userId)
{
    std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
    auto it = globalUsers.find(userId);
    if (it != globalUsers.end() && it->second.inGame)
    {
        // User decides to stop, calculate their earnings based on the current multiplier
        double earnings = roundDown(it->second.betAmount * multiplier);
        it->second.balance = roundDown(it->second.balance + earnings);
        it->second.reset(); // Reset user's game state
        std::cout << "User " << userId << " stopped and secured " << earnings << std::endl;
    }
    else if (it == globalUsers.end())
    {
        std::cout << "User " << userId << " not found." << std::endl;
    }
    else
    {
        std::cout << "User " << userId << " is not currently in a game." << std::endl;
    }
}

// Ends the current game, calculating and updating user balances
void Gameroom::endGame()
{
    std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
    for (auto &[id, user] : globalUsers)
    {
        if (user.inGame)
        {
            // user.balance += user.betAmount * multiplier; // Update balance based on final multiplier
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
    std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
    // Use find() to check if the user exists in the map
    auto it = globalUsers.find(userId);
    if (it != globalUsers.end() && !gameInProgress)
    { // Check if user exists and game is not in progress
        // Check if user has sufficient balance and is not already in a game
        if (it->second.balance >= betAmount && betAmount > 0)
        {
            it->second.placeBet(betAmount); // Place bet using the found user
            std::cout << "User " << userId << " placed a bet of $" << betAmount << std::endl;
        }
        else if (betAmount <= 0)
        {
            std::cout << "User " << userId << " cannot place a bet of $" << betAmount << " (bet must be greater than $0)." << std::endl;
        }
        else
        {
            std::cout << "User " << userId << " does not have enough balance to place a bet of $" << betAmount << std::endl;
        }
    }
    else if (it == globalUsers.end())
    {
        std::cout << "User " << userId << " not found." << std::endl;
    }
    else if (gameInProgress)
    {
        std::cout << "Cannot place bet, the game is currently in progress." << std::endl;
    }
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

// Retrieves the current game multiplier
double Gameroom::getCurrentMultiplier() const
{
    return multiplier;
}

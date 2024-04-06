#include "Gameroom.h"
#include "User.h"
#include "Utils.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring> // For memset
#include <string>
#include <sstream>

#ifdef _WIN32
#include <winsock2.h> // For send
#else
#include <unistd.h> // For write
#endif

extern std::unordered_map<std::string, User> globalUsers;                                                     // Use the external global globalUsers map
extern std::mutex usersMutex;                                                                                 // Use the external global mutex for globalUsers
extern std::atomic<bool> stopRequested;                                                                       // Use the external stopRequested

// Constructor with parameter 
Gameroom::Gameroom() : multiplier(1.0), gameInProgress(false), name("") {}
Gameroom::Gameroom(const std::string &roomName) : multiplier(1.0), gameInProgress(false), name(roomName) {}
Gameroom::Gameroom(Gameroom&& other) : multiplier(1.0), gameInProgress(false), name(other.name), clients(other.clients) {}

void handleUserInput(std::atomic<bool>& stopRequested, std::function<void(std::string)> stopFunction) {
    std::string userInput;
    while (!stopRequested) {
        if (std::getline(std::cin, userInput)) { // This will still block, but now it's in a separate thread
            std::istringstream iss(userInput);
            std::string command, userId;
            iss >> command >> userId;
            if (command == "stop") {
                stopFunction(userId);
            }
        }
    }
}

void Gameroom::acceptClient(int clientSocket)
{
    // add client socket to list of managed sockets by this game room
    clients.push_back(clientSocket);

    // send a socket connection msg to client saying they joined properly
    std::string message = "You have successfully joined the game room: " + name + "\n";
    if (send(clientSocket, message.c_str(), message.length(), 0) == -1)
    {
        std::cerr << "Error sending join message to client\n";
    }

    // print out the clients in the game room
    std::cout << "Clients in the game room " << name << ":\n" << std::endl;
    for (int clientSocket : clients)
    {
        std::cout << "Client socket: " << clientSocket << std::endl;
    }
}

// Adds a user to the game or updates the user's initial balance if they already exist
void Gameroom::addUser(const std::string &userId, double initialBalance)
{
    std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
    if (initialBalance < 0)
    {
        std::cout << "Cannot add user " << userId << " with a negative balance." << std::endl;
        return;
    }

    // Directly use the global globalUsers map
    auto result = globalUsers.emplace(userId, User(userId, roundDown(initialBalance)));
    if (!result.second)
    {
        std::cout << "User " << userId << " already exists. No new user added." << std::endl;
    }
    else
    {
        std::cout << "User " << userId << " has been added with an initial balance of $" << initialBalance << "." << std::endl;
    }
}

bool Gameroom::anyUserInGame() const
{
    std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
    return std::any_of(globalUsers.begin(), globalUsers.end(), [](const auto &entry)
                       { return entry.second.inGame; });
}

// Starts a new game, resetting necessary components and starting the multiplier increase
void Gameroom::startGame()
{
    std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
    std::cout << "startGame called!" << std::endl;
    std::cout << "gameInProgress: " << gameInProgress << std::endl;
    std::cout << "anyUserInGame(): " << anyUserInGame() << std::endl;

    if (!gameInProgress && anyUserInGame())
    {
        std::cout << "Starting a new game... Type 'stop <userID>' to secure your bet at the current multiplier." << std::endl;
        gameInProgress = true;
        multiplier = 1.0;
        stopRequested = false;

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
    std::cout << "Game ended." << std::endl;
}

// Returns whether a game is currently in progress
bool Gameroom::isGameInProgress() const
{
    std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
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

// Retrieves the current game multiplier
double Gameroom::getCurrentMultiplier() const
{
    return multiplier;
}

void Gameroom::removeUser(const std::string &userId)
{
    std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
    globalUsers.erase(userId);                    // Remove the user from the game
}

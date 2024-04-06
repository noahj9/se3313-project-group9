#ifndef GAMEROOM_H
#define GAMEROOM_H

#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <future>
#include <atomic>
#include <vector>
#include "User.h"

class Gameroom
{
private:
    double multiplier;            // Current game multiplier
    bool gameInProgress;          // Flag to check if a game is currently in progress
    mutable std::mutex gameMutex; // Mutex for synchronizing access to game state

public:
    std::string name;
    std::vector<User> users;

    // Constructor to initialize the game
    Gameroom();

    // Method declarations
    void acceptClient(int clientSocket);
    void addUser(const std::string &userId, double initialBalance);
    bool anyUserInGame() const;
    void startGame();
    void userStops(const std::string &userId);
    void endGame();
    bool isGameInProgress() const;
    void placeUserBet(const std::string &userId, double betAmount);
    double getCurrentMultiplier() const;
    void removeUser(const std::string &userId);
};

#endif

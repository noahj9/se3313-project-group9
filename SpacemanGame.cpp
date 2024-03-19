#include <unordered_map>
#include <string>
#include <vector>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()
#include <iostream>
#include <thread>  // For sleep_for
#include <chrono>  // For seconds

class User {
public:
    std::string id;
    double balance;
    double betAmount;
    bool inGame;

    User(const std::string& id, double balance) : id(id), balance(balance), betAmount(0), inGame(false) {}
    void placeBet(double amount) {
        if (amount <= balance && !inGame) { // Check if user has enough balance and is not already in a game
            betAmount = amount;
            balance -= amount;
            inGame = true;
        }
    }
    void reset() {
        inGame = false;
        betAmount = 0;
    }
};

class SpacemanGame {
private:
    double multiplier;
    bool gameInProgress;
    std::unordered_map<std::string, User> users;

public:
    SpacemanGame() : multiplier(1.0), gameInProgress(false) {}

    void addUser(const std::string& userId, double initialBalance) {
        users[userId] = User(userId, initialBalance);
    }

    void startGame() {
        gameInProgress = true;
        multiplier = 1.0;
        srand(time(0)); // Seed random number generator

        while (gameInProgress) {
            std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate time passing
            multiplier += 0.1; // Increase multiplier over time

            if ((rand() % 100) < 5) { // 5% chance each second to end the game
                endGame();
            }
        }
    }

    void endGame() {
        for (auto& [id, user] : users) {
            if (user.inGame) {
                user.balance += user.betAmount * multiplier; // Pay out based on final multiplier
                user.reset();
            }
        }
        gameInProgress = false;
    }

    bool isGameInProgress() const {
        return gameInProgress;
    }

    void placeUserBet(const std::string& userId, double betAmount) {
        if (users.find(userId) != users.end() && !gameInProgress) {
            users[userId].placeBet(betAmount);
        }
    }

    double getCurrentMultiplier() const {
        return multiplier;
    }
};

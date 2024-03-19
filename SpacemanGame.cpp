#include <unordered_map>
#include <string>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()
#include <iostream>
#include <thread>  // For sleep_for
#include <chrono>  // For seconds

// Class to manage user details and actions within the game
class User {
public:
    std::string id; // User identifier
    double balance; // User's current balance
    double betAmount; // Amount bet by the user for the current game
    bool inGame; // Flag to check if the user is currently in a game

    // Constructor to initialize user with id and balance
    User(const std::string& id, double balance) : id(id), balance(balance), betAmount(0), inGame(false) {}

    // Method to allow a user to place a bet
    void placeBet(double amount) {
        // Ensure the user has enough balance and is not already in a game
        if (amount <= balance && !inGame) {
            betAmount = amount;
            balance -= amount;
            inGame = true;
        }
    }

    // Resets the user's game state after each round
    void reset() {
        inGame = false;
        betAmount = 0;
    }

    void deposit(double amount) {
        if (amount > 0) {
            balance += amount;
        }
    }

    bool withdraw(double amount) {
        if (amount <= balance && amount > 0) {
            balance -= amount;
            return true;
        }
        return false;
    }
};

// Class to manage the Spaceman game state and interactions
class SpacemanGame {
private:
    double multiplier; // Current game multiplier
    bool gameInProgress; // Flag to check if a game is currently in progress
    std::unordered_map<std::string, User> users; // Mapping from user IDs to user objects

public:
    // Constructor to initialize the game
    SpacemanGame() : multiplier(1.0), gameInProgress(false) {
        srand(time(0)); // Seed the random number generator
    }

    // Adds a user to the game or updates the user's initial balance if they already exist
    void addUser(const std::string& userId, double initialBalance) {
        // Attempt to insert the new user into the map
        auto result = users.emplace(userId, User(userId, initialBalance));

        // Check if the insertion took place
        if (!result.second) {
            // The user already exists, so we could update the user's balance, log a message, or take some other action.
            // For now, let's just print a message:
            std::cout << "User " << userId << " already exists. No new user added." << std::endl;
        }
    }

    void listAllUsers() const {
        std::cout << "List of all users and their balances:" << std::endl;
        for (const auto& [id, user] : users) {
            std::cout << "User: " << id << ", Balance: $" << user.balance << std::endl;
        }
    }

    bool anyUserInGame() const {
        for (const auto& [id, user] : users) {
            if (user.inGame) {
                return true;
            }
        }
        return false;
    }

    // Starts a new game, resetting necessary components and starting the multiplier increase
    void startGame() {
        if (!gameInProgress && anyUserInGame()) { // Only start a new game if one is not already in progress and there are users ready to play
            std::cout << "Starting a new game..." << std::endl;
            gameInProgress = true;
            multiplier = 1.0;
            
            // Game loop
            while (gameInProgress) {
                std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate time passing
                multiplier += 0.1; // Increase the multiplier over time

                if ((rand() % 100) < 5) { // Randomly decide if the game should end
                    endGame();
                }
            }
        } else {
            std::cout << "Cannot start a new game. Either a game is already in progress or no users are ready." << std::endl;
        }
    }

    // Ends the current game, calculating and updating user balances
    void endGame() {
        for (auto& [id, user] : users) {
            if (user.inGame) {
                user.balance += user.betAmount * multiplier; // Update balance based on final multiplier
                user.reset(); // Reset user state for the next game
            }
        }
        gameInProgress = false;
    }

    // Returns whether a game is currently in progress
    bool isGameInProgress() const {
        return gameInProgress;
    }

    // Allows a user to place a bet, given their ID and bet amount
    void placeUserBet(const std::string& userId, double betAmount) {
        // Use find() to check if the user exists in the map
        auto it = users.find(userId);
        if (it != users.end() && !gameInProgress) { // Check if user exists and game is not in progress
            it->second.placeBet(betAmount); // Place bet using the found user
        } else if (it == users.end()) {
            std::cout << "User " << userId << " not found." << std::endl;
        } else if (gameInProgress) {
            std::cout << "Cannot place bet, the game is currently in progress." << std::endl;
        }
    }

    // Retrieves the current game multiplier
    double getCurrentMultiplier() const {
        return multiplier;
    }
};

int main() {
    SpacemanGame game;

    std::string command;
    std::string userId;
    double amount;

    while (true) {
        std::cout << "Enter command (add, bet, start, list, deposit, withdraw, exit): ";
        std::cin >> command;

        if (command == "add") {
            std::cout << "Enter user ID and initial balance: ";
            std::cin >> userId >> amount;
            game.addUser(userId, amount);
        } else if (command == "bet") {
            std::cout << "Enter user ID and bet amount: ";
            std::cin >> userId >> amount;
            game.placeUserBet(userId, amount);
        } else if (command == "start") {
            game.startGame();
        } else if (command == "list") {
            game.listAllUsers();
        } else if (command == "deposit") {
            std::cout << "Enter user ID and deposit amount: ";
            std::cin >> userId >> amount;
            // You need to implement deposit logic similar to placeUserBet in terms of finding the user
        } else if (command == "withdraw") {
            std::cout << "Enter user ID and withdrawal amount: ";
            std::cin >> userId >> amount;
            // Implement withdrawal logic here, similar to deposit
        } else if (command == "exit") {
            break;
        } else {
            std::cout << "Unknown command." << std::endl;
        }
    }

    return 0;
}

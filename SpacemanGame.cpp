#include <unordered_map>
#include <string>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()
#include <iostream>
#include <thread>  // For sleep_for
#include <chrono>  // For seconds
#include <sstream>
#include <mutex>
#include <future>
#include <atomic>
#include <functional> // For std::function

std::atomic<bool> stopRequested(false);
std::mutex userStopMutex;

double roundDown(double value) {
    return std::floor(value * 100.0) / 100.0;
}

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
        // Ensure the user has enough balance, is not already in a game, and the bet is a positive number
        if (amount > 0 && amount <= balance && !inGame) {
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
            balance = roundDown(balance + amount);
        }
    }

    bool withdraw(double amount) {
        if (amount <= balance && amount > 0) {
            balance = roundDown(balance - amount);
            return true;
        }
        return false;
    }
};

// Assuming this function is declared above the SpacemanGame class or in another header file included before SpacemanGame
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
        // Check if the initial balance is negative
        if (initialBalance < 0) {
            std::cout << "Cannot add user " << userId << " with a negative balance of $" << initialBalance << "." << std::endl;
            return; // Exit the function if the balance is negative
        }

        initialBalance = roundDown(initialBalance);

        // Attempt to insert the new user into the map
        auto result = users.emplace(userId, User(userId, initialBalance));

        // Check if the insertion took place
        if (!result.second) {
            // The user already exists, so we could update the user's balance, log a message, or take some other action.
            // For now, let's just print a message:
            std::cout << "User " << userId << " already exists. No new user added." << std::endl;
        } else {
            std::cout << "User " << userId << " has been added with an initial balance of $" << initialBalance << "." << std::endl;
        }
    }


    void listAllUsers() const {
        if (users.empty()) {
            std::cout << "There are no users." << std::endl;
        } else {
            std::cout << "List of all users and their balances:" << std::endl;
            for (const auto& [id, user] : users) {
                std::cout << "User: " << id << ", Balance: $" << user.balance << std::endl;
            }
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

    void userDeposit(const std::string& userId, double amount) {
        auto it = users.find(userId);
        if (it != users.end()) {
            if (amount > 0) {
                it->second.deposit(amount);
                std::cout << "User " << userId << " has deposited $" << amount << ". New balance: $" << it->second.balance << std::endl;
            } else {
                std::cout << "Invalid deposit amount. Please enter a positive number." << std::endl;
            }
        } else {
            std::cout << "User " << userId << " not found." << std::endl;
        }
    }

    void userWithdraw(const std::string& userId, double amount) {
        auto it = users.find(userId);
        if (it != users.end()) {
            if (amount > 0) {
                bool success = it->second.withdraw(amount);
                if (success) {
                    std::cout << "User " << userId << " has withdrawn $" << amount << ". New balance: $" << it->second.balance << std::endl;
                } else {
                    std::cout << "User " << userId << " cannot withdraw $" << amount << " due to insufficient balance." << std::endl;
                }
            } else {
                std::cout << "Invalid withdrawal amount. Please enter a positive number." << std::endl;
            }
        } else {
            std::cout << "User " << userId << " not found." << std::endl;
        }
    }

    // Starts a new game, resetting necessary components and starting the multiplier increase
    void startGame() {
        if (!gameInProgress && anyUserInGame()) {
            std::cout << "Starting a new game... Type 'stop <userID>' to secure your bet at the current multiplier." << std::endl;
            gameInProgress = true;
            multiplier = 1.0;
            stopRequested = false;

            // Pass the member function and the object for which it will be called
            std::future<void> userInputHandler = std::async(std::launch::async, handleUserInput, std::ref(stopRequested), [this](std::string userId){ this->userStops(userId); });

            // Game loop starts here
            while (gameInProgress) {
                std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate time passing
                multiplier += 0.1; // Increment the multiplier over time
                std::cout << "Current multiplier: " << multiplier << std::endl; // Display the current multiplier

                // The game continues running here. Since std::getline is blocking, we removed it from this loop.
                // The handling of 'stop' commands is done asynchronously in handleUserInput.

                if ((rand() % 100) < 5) { // There's a random chance to end the game
                    endGame(); // End the game if the random condition is met
                    break; // Exit the while loop since the game has ended
                }
            }

            stopRequested = true; // Signal the user input thread to stop
            userInputHandler.get(); // Wait for the user input handling thread to finish

            gameInProgress = false; // Mark the game as no longer in progress
        } else {
            if (gameInProgress) {
                std::cout << "Cannot start a new game. A game is already in progress." << std::endl;
            } else {
                std::cout << "Cannot start a new game. No users are ready." << std::endl;
            }
        }
    }

    void userStops(const std::string& userId) {
        auto it = users.find(userId);
        if (it != users.end() && it->second.inGame) {
            // User decides to stop, calculate their earnings based on the current multiplier
            double earnings = roundDown(it->second.betAmount * multiplier);
            it->second.balance = roundDown(it->second.balance + earnings);
            it->second.reset(); // Reset user's game state
            std::cout << "User " << userId << " stopped and secured " << earnings << std::endl;
        } else if (it == users.end()) {
            std::cout << "User " << userId << " not found." << std::endl;
        } else {
            std::cout << "User " << userId << " is not currently in a game." << std::endl;
        }
    }

    // Ends the current game, calculating and updating user balances
    void endGame() {
        for (auto& [id, user] : users) {
            if (user.inGame) {
                // user.balance += user.betAmount * multiplier; // Update balance based on final multiplier
                user.reset(); // Reset user state for the next game
            }
        }
        gameInProgress = false;
        std::cout << "Game ended." << std::endl;
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
            // Check if user has sufficient balance and is not already in a game
            if (it->second.balance >= betAmount && betAmount > 0) {
                it->second.placeBet(betAmount); // Place bet using the found user
                std::cout << "User " << userId << " placed a bet of $" << betAmount << std::endl;
            } else if (betAmount <= 0) {
                std::cout << "User " << userId << " cannot place a bet of $" << betAmount << " (bet must be greater than $0)." << std::endl;
            } else {
                std::cout << "User " << userId << " does not have enough balance to place a bet of $" << betAmount << std::endl;
            }
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
            game.userDeposit(userId, amount);
        } else if (command == "withdraw") {
            std::cout << "Enter user ID and withdrawal amount: ";
            std::cin >> userId >> amount;
            game.userWithdraw(userId, amount);
        } else if (command == "exit") {
            break;
        } else {
            std::cout << "Unknown command." << std::endl;
        }
    }\

    return 0;
}

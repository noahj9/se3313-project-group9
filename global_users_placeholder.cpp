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
#include <vector>
#include <condition_variable>
#include <cmath>  // For floor
#include <map>    // For room management

std::atomic<bool> stopRequested(false);
// std::mutex userStopMutex;

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
};

// Users are global because this will be updated by the socket rather than by me :)
std::unordered_map<std::string, User> users; // Mapping from user IDs to user objects
std::mutex usersMutex; // Global mutex for synchronizing access to the users

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
    mutable std::mutex gameMutex; // Mutex for synchronizing access to game state

public:
    // Constructor to initialize the game
    SpacemanGame() : multiplier(1.0), gameInProgress(false) {}

    // Adds a user to the game or updates the user's initial balance if they already exist
    void addUser(const std::string& userId, double initialBalance) {
        std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
        if (initialBalance < 0) {
            std::cout << "Cannot add user " << userId << " with a negative balance." << std::endl;
            return;
        }

        // Directly use the global users map
        auto result = users.emplace(userId, User(userId, roundDown(initialBalance)));
        if (!result.second) {
            std::cout << "User " << userId << " already exists. No new user added." << std::endl;
        } else {
            std::cout << "User " << userId << " has been added with an initial balance of $" << initialBalance << "." << std::endl;
        }
    }


    void listAllUsers() const {
        std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
        if (users.empty()) {
            std::cout << "There are no users." << std::endl;
            return;
        }
        std::cout << "List of all users and their balances:" << std::endl;
        for (const auto& [id, user] : users) {
            std::cout << "User: " << id << ", Balance: $" << user.balance << std::endl;
        }
    }

    bool anyUserInGame() const {
        std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
        return std::any_of(users.begin(), users.end(), [](const auto& entry) { return entry.second.inGame; });
    }

    // Starts a new game, resetting necessary components and starting the multiplier increase
    void startGame() {
        std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
        std::cout << "startGame called!" << std::endl;
        std::cout << "gameInProgress: " << gameInProgress << std::endl;
        std::cout << "anyUserInGame(): " << anyUserInGame() << std::endl;

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
        std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
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
        std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
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
        std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
        return gameInProgress;
    }

    // Allows a user to place a bet, given their ID and bet amount
    void placeUserBet(const std::string& userId, double betAmount) {
        std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
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

    void removeUser(const std::string& userId) {
        std::lock_guard<std::mutex> lock(usersMutex); // Lock for thread safety
        users.erase(userId); // Remove the user from the game
    }
};

class GameRoomManager {
private:
    std::unordered_map<std::string, std::unique_ptr<SpacemanGame>> rooms;
    std::unordered_map<std::string, std::string> userRooms; // Map from userID to roomName
    mutable std::mutex managerMutex;

public:
    void addRoom(const std::string& roomName) {
        // std::lock_guard<std::mutex> lock(managerMutex); --> Already locked in the joinOrCreateRoom function
        rooms.emplace(roomName, std::make_unique<SpacemanGame>());
    }

    // void addUserToRoom(const std::string& userId, const std::string& roomName, double initialBalance) {
    //     std::lock_guard<std::mutex> lock(managerMutex);
    //     if (rooms.find(roomName) != rooms.end()) {
    //         rooms[roomName]->addUser(userId, initialBalance);
    //         userRooms[userId] = roomName;
    //     } else {
    //         std::cout << "Room does not exist." << std::endl;
    //     }
    // }

    void listRooms() {
        std::lock_guard<std::mutex> lock(managerMutex);
        if (rooms.empty()) {
            std::cout << "There are no rooms currently created!" << std::endl;
        } else {
            for (const auto& room : rooms) {
                std::cout << room.first << std::endl; // Print room names
            }
        }
    }

    void startGameInRoom(const std::string& roomName) {
        std::lock_guard<std::mutex> lock(managerMutex);
        if (roomName.empty()) {
            std::cout << "User is not currently in a room." << std::endl;
        } else if (rooms.find(roomName) != rooms.end()) {
            std::cout << "Starting game in room: " << roomName << std::endl;
            rooms[roomName]->startGame();
        } else {
            std::cout << "Room does not exist." << std::endl;
        }
    }

    void userBet(const std::string& userId, double betAmount) {
        std::lock_guard<std::mutex> lock(managerMutex);
        if (userRooms.find(userId) != userRooms.end()) {
            std::string roomName = userRooms[userId];
            rooms[roomName]->placeUserBet(userId, betAmount);
        } else {
            std::cout << "User is not in any room." << std::endl;
        }
    }

    void userLeave(const std::string& userId) {
        std::lock_guard<std::mutex> lock(managerMutex);
        if (userRooms.find(userId) != userRooms.end()) {
            std::string roomName = userRooms[userId];
            rooms[roomName]->removeUser(userId);
            userRooms.erase(userId);
            std::cout << "You have left the room." << std::endl;
        } else {
            std::cout << "User is not in any room." << std::endl;
        }
    }

    void showAvailableActions(const std::string& userId) {
        std::lock_guard<std::mutex> lock(managerMutex);
        if (userRooms.find(userId) == userRooms.end()) {
            // User is not in any room
            std::cout << "Available commands: listRooms, join, exit\n";
        } else {
            // User is in a room
            std::cout << "Available commands: bet, start, status, leave, stop (if in a game)\n";
        }
    }

    // Function for joining a room, checks if user is already in a different room
    void joinOrCreateRoom(const std::string& userId, const std::string& roomName) {
        std::lock_guard<std::mutex> lock(managerMutex);
        std::lock_guard<std::mutex> usersLock(usersMutex);

        // Check if the user exists
        if (users.find(userId) == users.end()) {
            std::cout << "Error: User " << userId << " does not exist globally. Please register before joining a room." << std::endl;
            return;
        }

        // Check if the user is already in a room
        auto currentRoom = userRooms.find(userId);
        if (currentRoom != userRooms.end() && currentRoom->second != roomName) {
            std::cout << "Error: " << userId << " is already in room " << currentRoom->second << ". Please leave the current room before joining another." << std::endl;
            return;
        }
        
        // If room doesn't exist, create new one
        if (rooms.find(roomName) == rooms.end()) {
            std::cout << "Creating new room: " << roomName << std::endl;
            addRoom(roomName);
        }

        // Add user to the room (this could be a new join or a switch from the default room)
        // rooms[roomName]->addUser(userId, users[userId].balance);
        rooms.at(roomName)->addUser(userId, users.at(userId).balance);
        userRooms[userId] = roomName;
        std::cout << "User " << userId << " joined room: " << roomName << std::endl;
    }

    // Function to show the status of the user's current room
    void showRoomStatus(const std::string& userId) {
        std::lock_guard<std::mutex> lock(managerMutex);
        const auto& roomName = userRooms.find(userId);
        if (roomName != userRooms.end()) {
            std::cout << "You are in room: " << roomName->second << std::endl;
            // If we want to show more details like who else is in the room, game status etc., we could expand this part.
            // For instance, list all users in the same room:
            rooms[roomName->second]->listAllUsers();
        } else {
            std::cout << "You are not currently in any room." << std::endl;
        }
    }
    
    // Remove user from their current room
    void leaveRoom(const std::string& userId) {
        userLeave(userId);
    }

    // This method returns the room name for a specific user
    std::string getUserRoom(const std::string& userId) {
        std::lock_guard<std::mutex> lock(managerMutex);
        if (userRooms.find(userId) != userRooms.end()) {
            return userRooms[userId];
        } else {
            return ""; // User is not in a room
        }
    }

    bool isUserInGame(const std::string& userId) {
        std::lock_guard<std::mutex> lock(managerMutex);
        std::lock_guard<std::mutex> usersLock(usersMutex);

        auto it = users.find(userId);
        if (it != users.end()) {
            return it->second.inGame;
        }
        return false;
    }

    // Adds a user to the global list without adding them to any specific room
    void addUserToGlobalList(const std::string& userId, double initialBalance) {
        std::lock_guard<std::mutex> lock(managerMutex);
        std::lock_guard<std::mutex> usersLock(usersMutex);

        // Check if the user already exists in users map
        if (users.find(userId) == users.end()) {
            // User not found, add new user
            users.emplace(userId, User(userId, initialBalance));
            std::cout << "Global user added with ID: " << userId << " and balance: $" << initialBalance << std::endl;
        } else {
            // User already exists, do not add and notify
            std::cout << "User with ID: " << userId << " already exists in the global list." << std::endl;
        }
    }
};

void clearExcessConsole() {
    // Clear any errors and ignore any leftover input after reading user ID.
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main() {
    srand(time(0)); // Initialize random seed once, at the start of the program
    GameRoomManager manager;

    std::string command, userId, roomName;
    double amount;

    while (true) {
        std::cout << "Enter your user ID: ";
        std::cin >> userId;

        clearExcessConsole();

        // Check if the user exists. If not, then add them to the global list with an initial balance of $10
        if (users.find(userId) == users.end()) {
            manager.addUserToGlobalList(userId, 10);
        }

        // Now that userId is initialized, show available actions based on user status
        manager.showAvailableActions(userId);

        std::cout << "Enter command: ";
        std::cin >> command;

        clearExcessConsole();

        if (command == "listRooms") {
            manager.listRooms();
        } else if (command == "status") {
            manager.showRoomStatus(userId);
        } else if (command == "join") {
            std::cout << "Enter room name: ";
            std::cin >> roomName;
            manager.joinOrCreateRoom(userId, roomName);
        } else if (command == "leave") {
            manager.leaveRoom(userId);
        } else if (command == "bet") {
            if (!manager.getUserRoom(userId).empty()) {
                std::cout << "Enter bet amount: ";
                std::cin >> amount;
                if (amount <= 0) {
                    std::cout << "Invalid amount. Please enter a positive number." << std::endl;
                    continue; // Skip the rest of the loop iteration if the amount is invalid
                }
                manager.userBet(userId, amount);
            } else {
                std::cout << "You can only place a bet if you are in an ongoing game room." << std::endl;
            }
        } else if (command == "start") {
            if (!manager.getUserRoom(userId).empty()) {
                if (manager.isUserInGame(userId)) {
                    manager.startGameInRoom(manager.getUserRoom(userId));
                } else {
                    std::cout << "You can only start a game if you have placed a bet." << std::endl;
                }
            } else {
                std::cout << "You need to be in a room to start a game." << std::endl;
            }
        } else if (command == "exit") {
            if (manager.isUserInGame(userId)) {
                std::cout << "You cannot exit while in a game. Please stop the game first." << std::endl;
            } else {
                break; // Exit the loop and end the program
            }
        } else {
            std::cout << "Unknown command. Please try again." << std::endl;
        }
    }

    return 0;
}

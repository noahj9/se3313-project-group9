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
#include <map>    // For room management
#include "User.h"
#include "Gameroom.h"
#include "Utils.h"

std::atomic<bool> stopRequested(false);
// std::mutex userStopMutex;

// Users are global because this will be updated by the socket rather than by me :)
std::unordered_map<std::string, User> globalUsers;
std::mutex usersMutex; // Global mutex for synchronizing access to the users

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

class GameRoomManager {
private:
    std::unordered_map<std::string, std::unique_ptr<Gameroom>> rooms;
    std::unordered_map<std::string, std::string> userRooms; // Map from userID to roomName
    mutable std::mutex managerMutex;

public:
    void addRoom(const std::string& roomName) {
        // std::lock_guard<std::mutex> lock(managerMutex); --> Already locked in the joinOrCreateRoom function
        rooms.emplace(roomName, std::make_unique<Gameroom>());
    }

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
        // std::lock_guard<std::mutex> usersLock(usersMutex);

        // Check if the user exists
        if (globalUsers.find(userId) == globalUsers.end()) {
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
        // rooms[roomName]->addUser(userId, globalUsers[userId].balance);
        rooms.at(roomName)->addUser(userId, globalUsers.at(userId).balance);
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
        auto roomIter = userRooms.find(userId);
        if (roomIter != userRooms.end()) { // Check if the user is in any room
            std::lock_guard<std::mutex> usersLock(usersMutex); // Ensure thread safety when accessing global users
            auto userIter = globalUsers.find(userId);
            if (userIter != globalUsers.end()) {
                // Check the user's inGame flag directly from the global users map
                return userIter->second.inGame;
            }
        }
        return false; // User is not in a game if none of the above conditions are true
    }

    bool doesUserExists(const std::string& userId) const {
        std::lock_guard<std::mutex> lock(managerMutex);
        return globalUsers.find(userId) != globalUsers.end();
    }

    // Adds a user to the global list without adding them to any specific room
    void addUserToGlobalList(const std::string& userId, double initialBalance) {
        std::lock_guard<std::mutex> lock(managerMutex);
        // Check if the user already exists in globalUsers map
        if (globalUsers.find(userId) == globalUsers.end()) {
            // User not found, add new user
            globalUsers.emplace(userId, User(userId, initialBalance));
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
        if (!manager.doesUserExists(userId)) {
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

#ifndef USER_H
#define USER_H

#include <string>

// Class to manage user details and actions within the game
class User {
public:
    std::string id; // User identifier
    double balance; // User's current balance
    double betAmount; // Amount bet by the user for the current game
    bool inGame; // Flag to check if the user is currently in a game

    // Constructor
    User(const std::string& id, double balance);

    void placeBet(double amount);
    void reset();
};

#endif

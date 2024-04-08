#include "User.h"

// Constructor to initialize user with id and balance
User::User(const std::string& id, double balance, int socket) : id(id), socket(socket), balance(balance), betAmount(0), inGame(false) {}

// Method to allow a user to place a bet
void User::placeBet(double amount) {
    // Ensure the user has enough balance, is not already in a game, and the bet is a positive number
    if (amount > 0 && amount <= balance && !inGame) {
        betAmount = amount;
        balance -= amount;
        inGame = true;
    }
}

// Resets the user's game state after each round
void User::reset() {
    inGame = false;
    betAmount = 0;
}

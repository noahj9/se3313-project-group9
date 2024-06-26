### Notes

- server.cpp: method to connect with AWS
- SpacemanGame.cpp: Actual game logic

Building the game:

- g++ -std=c++17 -pthread SpacemanGame.cpp -o SpacemanGame

Running the game:

- ./SpacemanGame

## Design and Implementation

1. Base class for spaceman game

- holding the following functions: (TRISTAN/SCOTT)
  - attach a user: (accept a user thread and take ownership of the socket to communicate directly with client, set status to attached)
  - detach a user: (give ownership of socket back to client and set status of thread to unattached)
  - upon detach: store users balance back into server so it retains when moving to a new game
  - each game thread will hold a vector list of attached users, if all users leave the game will end
  - all functions for gameplay: already done for the most part by Tristan

2. User Thread (TRISTAN/NOAH)

- spawned upon socket connection from new user
- initializes the state of the user to unattached
- a user thread is created when a new connection joins the server
  - points them to the server where they can select a game to join or create a new game
  - initializes users balance to $100
  - biderectional communication with server
  - NO deposit or withdrawals!

3. Server (NOAH/JAMES)

- manage connections and passing of socket between game threads and user threads
- a thread that is always listening for incoming connections (new users)
  - when a new user joins, spawns a user thread and opens the socket connection
- the server will hold a vector list of all users that are attached, and users that are detached
- will also hold a vector list of all current game instances

4. GUI Side

- GUI interface for server: create a game, connect to existing game
- GUI for game: make bet, cashout, spaceman flying, show multiplier on screen, crash animation, display balance/winnings/loss
  - leave game button
  -

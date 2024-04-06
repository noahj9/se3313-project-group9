### Notes
- server.cpp: method to connect with AWS
- SpacemanGame.cpp: Actual game logic

Building the game:
- g++ -std=c++17 -pthread SpacemanGame.cpp -o SpacemanGame

Running the game:
- ./SpacemanGame

This branch is currently working on implementing mutexes. Please run the below command to test:
g++ -std=c++17 -pthread placeholder.cpp -o SecondSpacemanGame

./SecondSpacemanGame
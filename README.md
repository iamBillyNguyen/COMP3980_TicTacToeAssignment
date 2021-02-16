# COMP3980_Assignment1_TicTacToe

By: Iris Law, Tiffany Gu, Billy Nguyen

## 1. Installing dcfsm Library
Please install dcfsm library from D'Arcy's dcfsm repo https://github.com/darcy-bcit/dcfsm/blob/main/demo/demo.c

Use the following commands to install onto the machine (if has not done yet):
```
mkdir build

cd build

cmake ..

cmake --build .

sudo cmake --install .
```
## 2. Running the server
Make sure you have CMAKE version 3.17 or above installed on your Linux machine. First, git clone this repo to a preferred folder. Then, in the Terminal, locate the folder where you cloned this project to, and change your directory. If you will be playing with your local machine, you can go to the client.cpp file and update the IP address to "127.0.0.1" and save.Type ```cmake ..``` and hit enter. Then, type ```cmake --build .``` and hit enter. Finally, type ```./server``` to run the server. 

**If you will be using the existing IP address, please contact Tiffany Gu (Discord ID: Tiffany Gu#1093) to start the server for you.**

## 3. Connecting the client to remote server
On your terminal, run the following command to connect to the Tic Tac Toe server
```
gcc client.cpp -o client -lstdc++
```
Then, type ```./client``` to run the client. It will be connected to the remote server. 

## 4. Some other important things
The server is written in C and is compatible with Linux system. The client is written in C++ and is compatible with Linux system, too.  
The IP address of the server and the clients is hardcoded. To test connection, you can type
```telnet <IP address> <port>``` to check if the remote server is available.

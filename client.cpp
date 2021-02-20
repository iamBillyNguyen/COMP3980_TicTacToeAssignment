/*
 *
 * This is the client side code to connect to a tic-tac-toe game server.
 *
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "tictac.h"
#include "shared.h"

//#define SERV_HOST_ADDR "23.16.22.78"
#define SERV_HOST_ADDR "127.0.0.1"
#define BOARD_SIZE 3
using namespace std;

// gcc client.cpp -o client -lstdc++

void reset_board(char b[][3]) {
    for (int  i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (i == 0){
                b[i][j] = 'A' + j;
            } else if (i == 1) {
                b[i][j] = 'D' + j;
            } else {
                b[i][j] = 'G' + j;
            }
        }
    }
}

void check_opt(char choice, char b[][3]) {
    while (1) {
        if (choice == 'q') {
            exit(EXIT_SUCCESS);
        } else if (choice == 'r') {
            reset_board(b);
            break;
        } else {
            memset(&choice, 0, sizeof(choice));
            printf("Invalid choice! Please re-enter\n");
            printf("Type q to quit\nOR\nType r to replay\n>> ");
            cin >> choice;
        }
    }
}

/**
 * Main loop to drive the client program
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    int sockfd,  n, connectfd, bytes_sent;
    struct sockaddr_in serv_addr;
    char x[1];
    int count = 0;
    char buffer[1], server_buffer[1];
    char playBoard[3][3] = {{'A','B','C'},
                            {'D','E','F'},
                            {'G','H','I'}};

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("Sorry. Socket could not be created!");
        return 1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    connectfd = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (connectfd == -1)
    {
        perror("Sorry. Could not connect to server.");
        return 1;
    }

    display();
    while (1)
    {
        memset(&buffer, 0, sizeof(buffer));
        memset(&server_buffer, 0, sizeof(server_buffer));
        
        // either response code or position
        recv(sockfd, &server_buffer, sizeof(server_buffer), 0);
        puts(server_buffer);
        switch (server_buffer[0])
        {
            case '0':
                printf("Invalid move! Re-enter: ");
                cin >> buffer;
                std::cout << "sent " << buffer << " to server!" << endl;
                bytes_sent = send(sockfd, &buffer, sizeof(buffer), 0);

                if (bytes_sent == -1)
                {
                    perror("Bytes could not be sent!");
                    return 1;
                }
                break;
            case '1':
                break;
            case '2':
                printf("----- YOU WON! -----\n");
                printf("Type q to quit\nOR\nType r to replay\n>> ");
                cin >> x;
                check_opt(x[0], playBoard);
                bytes_sent = send(sockfd, &x, sizeof(x), 0);

                if (bytes_sent == -1)
                {
                    perror("Bytes could not be sent!");
                    return 1;
                }
                count = 0;
                break;
            case '3':
                printf("----- YOU LOST! -----\n");
                printf("Type q to quit\nOR\nType r to replay\n>> ");
                cin >> x;
                check_opt(x[0], playBoard);
                bytes_sent = send(sockfd, &x, sizeof(x), 0);

                if (bytes_sent == -1)
                {
                    perror("Bytes could not be sent!");
                    return 1;
                }
                count = 0;
                break;
            case '4':
                printf("----- TIE -----\n");
                printf("Type q to quit\nOR\nType r to replay\n>> ");
                cin >> x;
                check_opt(x[0], playBoard);
                count = 0;
                break;
            case '5':
                printf("Your turn, place your move: ");
                cin >> buffer;

                std::cout << "sent " << buffer << " to server!" << endl;
                bytes_sent = send(sockfd, &buffer, sizeof(buffer), 0);

                if (bytes_sent == -1)
                {
                    perror("Bytes could not be sent!");
                    return 1;
                }
                count++; // TO KEEP TRACK OF 'X' & 'O'
                break;
            case '6':
                printf("Please wait for your turn\n");
                count++; // TO KEEP TRACK OF 'X' & 'O'
                break;
            case '7':
                std::cout << "----- GAME BEGINS -----" << endl;
                break;
            default:
                break;
        }
        /** UPDATE BOARD */
        if (server_buffer[0] >= 'A' && server_buffer[0] <= 'I') {
                char sym = (count % 2 != 0) ? 'X' : 'O';
                update_board(server_buffer[0], playBoard, sym);
        }
        fflush(STDIN_FILENO);
    }

    std::cout << endl
         << "Thank You for playing Tic-tac-Toe" << endl;
    close(sockfd);
    return 0;
}
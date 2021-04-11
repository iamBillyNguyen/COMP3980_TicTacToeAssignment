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
#include "./shared.h"

// #define SERV_HOST_ADDR "23.16.22.78"
#define SERV_HOST_ADDR "127.0.0.1"
#define BOARD_SIZE 3
using namespace std;

// gcc client.cpp -o client -lstdc++

void reset_board(char b[][3]) {
    for (int  i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (i == 0){
                b[i][j] = 0 + j;
            } else if (i == 1) {
                b[i][j] = 3 + j;
            } else {
                b[i][j] = 6 + j;
            }
        }
    }
}

void check_opt(uint8_t choice, char b[][3]) {
    while (true) {
        if (choice == 'q') {
            exit(EXIT_SUCCESS);
        } else if (choice == 'r') {
            reset_board(b);
            printf("Awaiting player to join ...\n");
            break;
        } else {
            memset(&choice, 0, sizeof(choice));
            printf("Invalid choice! Please re-enter\n");
            printf("Type q to quit\nOR\nType r to replay\n>> ");
            cin >> choice;
        }
    }
    display();
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
    int count = 0;
    bool connected = false, close_conn = false;
    auto *uid = (uint8_t*) calloc(4, sizeof(uint8_t));
    char playBoard[9];
    char this_player, other_player;

    for (int i = 0; i < 9; i++)
        playBoard[i] = ' ';

    uint8_t *req;
    req = (uint8_t *) malloc(4 * sizeof(uint8_t));
    uint8_t *res;
    res = (uint8_t *) malloc(4 * sizeof(uint8_t));
    uint8_t choice[2];

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
    std::cout<<"\nWelcome to the Tic-Tac-Toe Game!";
    display();
    while (!close_conn) {
        memset(req, 0, sizeof(req));
        memset(res, 0, sizeof(res));

        if (!connected) {
            req[0] = 1;
            req[1] = 1;
            req[2] = 1;
            req[3] = 1;
            req[4] = TIC_TAC_TOE;
            // response from server
//        recv(sockfd, &server_buffer, sizeof(server_buffer), 0);

            send(sockfd, req, sizeof(req), 0);
            recv(sockfd, res, sizeof(res), 0);
            printf("Your uid: ");
            for (int i = 3; i < 7; i++) {
                printf("%x", res[i]);
            }
            cout << endl;
            connected = true;
        } else {
            recv(sockfd, res, sizeof(res), 0);
            memset(choice, 0, sizeof(choice));
            switch (res[MSG_TYPE]) {
                case SUCCESS:
                    if (CONFIRMATION) {
                        for (int i = 3; i < 7; i++) {
                            uid[i - 3] = res[i];
                        }
                    }
                    break;
                case UPDATE:
                    switch (res[CONTEXT]) {
                        case START_GAME:
                            if (res[PAYLOAD] == X) {
                                req[MSG_TYPE] = 4;      // Game action
                                req[CONTEXT] = 1;       // Make a move
                                req[PAYLOAD_LEN] = 1;
                                printf("Your turn, place your move: ");
                                cin >> choice;
                                cout << "sent " << choice[0] << " to server!" << endl;
                                choice[0] -= '0';
                                req[PAYLOAD] = choice[0];
                                bytes_sent = send(sockfd, req, sizeof(req), 0);
                                update_board(req[PAYLOAD], playBoard, (count % 2 == 0 ? 'X' : 'O'));

                                if (bytes_sent == -1)
                                {
                                    perror("Bytes could not be sent!");
                                    return 1;
                                }
                                count++; // TO KEEP TRACK OF 'X' & 'O'
                                this_player = 'X';
                                other_player = 'O';
                            }
                            if (res[PAYLOAD] == O) {
                                printf("Please wait for your turn\n");
                                count--;
                                this_player = 'O';
                                other_player = 'X';
                            }

                            break;
                        case MOVE_MADE: // receiving other player's move
                            display();
                            printf("Player %d has played\n", count % 2 == 0 ? 2 : 1);
                            choice[0] = res[PAYLOAD];
                            update_board(choice[0], playBoard, other_player);
                            count++; // TO KEEP TRACK OF 'X' & 'O'

                            req[MSG_TYPE] = 4;      // Game action
                            req[CONTEXT] = 1;       // Make a move
                            req[PAYLOAD_LEN] = 1;
                            printf("Your turn, place your move: ");
                            cin >> choice;
                            cout << "sent " << choice[0] << " to server!" << endl;
                            choice[0] -= '0';
                            req[PAYLOAD] = choice[0];
                            bytes_sent = send(sockfd, req, sizeof(req), 0);
                            update_board(req[PAYLOAD], playBoard, this_player);

                            if (bytes_sent == -1)
                            {
                                perror("Bytes could not be sent!");
                                return 1;
                            }

                            break;
                        case END_GAME:
                            switch (res[PAYLOAD]) {
                                case WIN:
                                    printf("----- YOU WON! -----\n");
                                    break;
                                case LOSS:
                                    printf("----- YOU LOSS! -----\n");
                                    break;
                            }
                            close_conn = true;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
//        for (int i = 0; i < 7; i++){
//            printf("%d", server_buffer[i]);
//        }
//        cout << "len " << sizeof(server_buffer) << endl;
            // switch (server_buffer[0])
            // {
            //     case '0':
            //         printf("Invalid move! Re-enter: ");
            //         cin >> buffer;
            //         std::cout << "sent " << buffer << " to server!" << endl;
            //         bytes_sent = send(sockfd, &buffer, sizeof(buffer), 0);

            //         if (bytes_sent == -1)
            //         {
            //             perror("Bytes could not be sent!");
            //             return 1;
            //         }
            //         break;
            //     case '1':
            //         break;
            //     case '2':
            //         printf("----- YOU WON! -----\n");
            //         printf("Type q to quit\nOR\nType r to replay\n>> ");
            //         cin >> x;
            //         check_opt(x[0], playBoard);
            //         bytes_sent = send(sockfd, &x, sizeof(x), 0);

            //         if (bytes_sent == -1)
            //         {
            //             perror("Bytes could not be sent!");
            //             return 1;
            //         }
            //         count = 0;
            //         break;
            //     case '3':
            //         printf("----- YOU LOST! -----\n");
            //         printf("Type q to quit\nOR\nType r to replay\n>> ");
            //         cin >> x;
            //         check_opt(x[0], playBoard);
            //         bytes_sent = send(sockfd, &x, sizeof(x), 0);

            //         if (bytes_sent == -1)
            //         {
            //             perror("Bytes could not be sent!");
            //             return 1;
            //         }
            //         count = 0;
            //         break;
            //     case '4':
            //         printf("----- TIE -----\n");
            //         printf("Type q to quit\nOR\nType r to replay\n>> ");
            //         cin >> x;
            //         check_opt(x[0], playBoard);
            //         count = 0;
            //         break;
            //     case '5':
            //         printf("Your turn, place your move: ");
            //         cin >> buffer;

            //         std::cout << "sent " << buffer << " to server!" << endl;
            //         bytes_sent = send(sockfd, &buffer, sizeof(buffer), 0);

            //         if (bytes_sent == -1)
            //         {
            //             perror("Bytes could not be sent!");
            //             return 1;
            //         }
            //         count++; // TO KEEP TRACK OF 'X' & 'O'
            //         break;
            //     case '6':
            //         printf("Please wait for your turn\n");
            //         count++; // TO KEEP TRACK OF 'X' & 'O'
            //         break;
            //     case '7':
            //         count++;
            //         break;
            //     default:
            //         break;
            // }
        }

        fflush(STDIN_FILENO);
    }
    free(res);
    free(req);
    std::cout << endl
              << "Thank You for playing Tic-tac-Toe" << endl;
    close(sockfd);
    return 0;
}

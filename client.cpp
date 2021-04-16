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
//#define SERV_HOST_ADDR "karelc.com"
#define SERV_HOST_ADDR "127.0.0.1"
#define BOARD_SIZE 3
using namespace std;

// gcc client.cpp -o client -lstdc++

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
    int count = 0, game_id;
    bool connected = false, close_conn = false, accepted = false;
    auto *uid = (uint8_t*) calloc(4, sizeof(uint8_t));
    char playBoard[9];
    char this_player, other_player;

    for (int i = 0; i < 9; i++)
        playBoard[i] = ' ';

    uint8_t *req, confirm_req[9];
    req = (uint8_t *) malloc(8 * sizeof(uint8_t));
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
    std::cout<<"\nWelcome to the BIT Arcade Server!" << endl;

    while (!close_conn) {
        memset(req, 0, sizeof(req));
        memset(res, 0, sizeof(res));

        if (!connected) {
            memset(confirm_req, 0, sizeof(confirm_req));
            confirm_req[REQ_TYPE] = CONFIRMATION;
            confirm_req[REQ_CONTEXT] = CONFIRM_RULESET;
            confirm_req[REQ_PAYLOAD_LEN] = 2;
            confirm_req[REQ_PAYLOAD] = 1;   // Version number

            int answer = get_game_option();
            switch (answer) {
                case TIC_TAC_TOE:
                    confirm_req[REQ_PAYLOAD + 1] = TIC_TAC_TOE;   // Game ID
                    display_ttt();
                    game_id = TIC_TAC_TOE;
                    break;
                case ROCK_PAPER_SCISSOR:
                    confirm_req[REQ_PAYLOAD + 1] = ROCK_PAPER_SCISSOR;   // Game ID
                    display_rps();
                    game_id = ROCK_PAPER_SCISSOR;
                    break;
                default:
                    break;
            }

            // response from server
//        recv(sockfd, &server_buffer, sizeof(server_buffer), 0);

            send(sockfd, confirm_req, sizeof(confirm_req), 0);
            recv(sockfd, res, sizeof(res), 0);
            printf("Your uid: ");
            for (int i = 3; i < 7; i++) {
                uid[i - 3] = res[i];
                printf("%x", res[i]);
            }
            cout << endl;
            connected = true;
        } else {
            recv(sockfd, res, sizeof(res), 0);
            memset(choice, 0, sizeof(choice));
            switch (res[MSG_TYPE]) {
                case SUCCESS:
                    if (res[CONTEXT] == CONFIRMATION) {
                        if (!accepted) {
                            for (int i = 3; i < 7; i++)
                                uid[i - 3] = res[i];
                            accepted = true;
                        } else {
                            printf("SUCCESS move\n");
                            break;
                        }
                    }
                    break;
                case UPDATE:
                    switch (res[CONTEXT]) {
                        case START_GAME:
                            if (game_id == TIC_TAC_TOE) {
                                if (res[PAYLOAD] == X) {
                                    for (int i = 0; i < 4; i++)
                                        req[i] = uid[i];
                                    req[REQ_TYPE] = GAME_ACTION;        // Game action
                                    req[REQ_CONTEXT] = MAKE_MOVE;       // Make a move
                                    req[REQ_PAYLOAD_LEN] = 1;
                                    printf("Your turn, place your move: ");
                                    cin >> choice;
                                    cout << "sent " << choice[0] << " to server!" << endl;
                                    choice[0] -= '0';
                                    req[REQ_PAYLOAD] = choice[0];
//                                    for (int i = 0; i < 8; i++)
//                                        printf("%x ", req[i]);
//                                    cout << endl;
                                    bytes_sent = send(sockfd, req, sizeof(req), 0);
                                    update_board(req[REQ_PAYLOAD], playBoard, (count % 2 == 0 ? 'X' : 'O'));

                                    if (bytes_sent == -1) {
                                        perror("Bytes could not be sent!");
                                        return 1;
                                    }
                                    count++; // TO KEEP TRACK OF 'X' & 'O'
                                    this_player = 'X';
                                    other_player = 'O';

                                    printf("Awaiting for player 2\n");
                                }
                                if (res[PAYLOAD] == O) {
                                    printf("Please wait for your turn\n");
                                    count--;
                                    this_player = 'O';
                                    other_player = 'X';
                                }
                            } else {
                                for (int i = 0; i < 4; i++)
                                    req[i] = uid[i];
                                req[REQ_TYPE] = GAME_ACTION;          // Game action
                                req[REQ_CONTEXT] = MAKE_MOVE;       // Make a move
                                req[REQ_PAYLOAD_LEN] = 1;
                                printf("Place your move: ");
                                cin >> choice;
                                cout << "sent " << choice[0] << " to server!" << endl;
                                choice[0] -= '0';
                                req[REQ_PAYLOAD] = choice[0];
//                                for (int i = 0; i < 8; i++)
//                                    printf("%x ", req[i]);
                                bytes_sent = send(sockfd, req, sizeof(req), 0);

                                if (bytes_sent == -1)
                                {
                                    perror("Bytes could not be sent!");
                                    return 1;
                                }
                            }

                            break;
                        case MOVE_MADE: // !!! Only for TTT
                            display_ttt();
                            printf("Player %d has played\n", count % 2 == 0 ? 2 : 1);
                            choice[0] = res[PAYLOAD];
                            update_board(choice[0], playBoard, other_player);
                            count++; // TO KEEP TRACK OF 'X' & 'O'

                            for (int i = 0; i < 4; i++)
                                req[i] = uid[i]; // uid
                            req[REQ_TYPE] = GAME_ACTION;      // Game action
                            req[REQ_CONTEXT] = MAKE_MOVE;       // Make a move
                            req[PAYLOAD_LEN] = 1;
                            printf("Your turn, place your move: ");
                            cin >> choice;
                            cout << "sent " << choice[0] << " to server!" << endl;
                            choice[0] -= '0';
                            req[REQ_PAYLOAD] = choice[0];
                            bytes_sent = send(sockfd, req, sizeof(req), 0);
                            update_board(req[REQ_PAYLOAD], playBoard, this_player);
                            printf("Awaiting for player %d\n", count % 2 == 0 ? 1 : 2);
                            if (bytes_sent == -1)
                            {
                                perror("Bytes could not be sent!");
                                return 1;
                            }

                            break;
                        case END_GAME:
                            switch (res[PAYLOAD]) {
                                case WIN:
                                    printf("Your winning move %x\n", res[PAYLOAD + 1]);
                                    printf("----- YOU WON! -----\n");
                                    break;
                                case LOSS:
                                    count++;
                                    choice[0] = res[PAYLOAD + 1];
                                    printf("Player %d has played move %x\n", count % 2 == 0 ? 1 : 2, choice[0]);
                                    if (game_id == TIC_TAC_TOE)
                                        update_board(choice[0], playBoard, other_player);
                                    printf("----- YOU LOSS! -----\n");
                                    break;
                                case TIE:
                                    printf("----- GAME TIES! -----\n");
                                    break;
                                default:
                                    break;
                            }
                            close_conn = true;
                            if (game_id == TIC_TAC_TOE)
                                std::cout << endl
                                          << "Thank You for playing BIT Arcade's Tic-tac-Toe" << endl;

                            if (game_id == ROCK_PAPER_SCISSOR)
                                std::cout << endl
                                          << "Thank You for playing BIT Arcade's Rock Paper Scissors" << endl;
                            break;
                        case OPPONENT_DISCONNECTED:
                            printf("Your opponent has disconnected\nThe game will end here!\n");
                            close_conn = true;
                            break;
                        default:
                            break;
                    }
                    break;
                case INVALID_TYPE:
                    printf("Invalid type\n");
                    break;
                case INVALID_CONTEXT:
                    printf("Invalid Context\n");
                    break;
                case INVALID_PAYLOAD:
                    printf("Invalid payload\n");
                    break;
                case INVALID_REQUEST:
                    printf("Invalid request\n");
                    break;
                case INVALID_UID:
                    printf("Invalid UID\n");
                    break;
                case INVALID_ACTION:
                    switch (res[CONTEXT]) {
                        case GAME_ACTION:
                            printf("Invalid move! Re-enter: ");
                            cin >> choice;
                            for (int i = 0; i < 4; i++)
                                req[i] = uid[i]; // uid
                            req[REQ_TYPE] = GAME_ACTION;      // Game action
                            req[REQ_CONTEXT] = MAKE_MOVE;       // Make a move
                            req[PAYLOAD_LEN] = 1;
                            cout << "sent " << choice[0] << " to server!" << endl;
                            choice[0] -= '0';
                            req[REQ_PAYLOAD] = choice[0];
                            bytes_sent = send(sockfd, req, sizeof(req), 0);

                            if (bytes_sent == -1)
                            {
                                perror("Bytes could not be sent!");
                                return 1;
                            }
                            if (game_id == TIC_TAC_TOE)
                                update_board(choice[0], playBoard, this_player);
                            break;
                        default:
                            break;
                    }

                    break;
                default:
                    break;
            }
        }
        fflush(STDIN_FILENO);
    }
    free(res);
    free(req);
    close(sockfd);
    return 0;
}

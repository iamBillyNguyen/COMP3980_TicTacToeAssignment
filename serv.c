/*
 * Copyright 2021 D'Arcy Smith + the BCIT CST Datacommunications Option students.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dcfsm/fsm.h>
#include <dc/stdio.h>
#include <dc/unistd.h>
#include <dc/sys/socket.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "shared.h"

#define BACKLOG 2
#define TOTAL_TURNS 9

static int init_server(Environment *env);
static int read_input(Environment *env);
static int validate(Environment *env);
static int prompt(Environment *env);
static int error(Environment *env);
static void update_board(char c, char playBoard[][3], char player, Environment *env);
char check (char playBoard[][3]);

typedef enum
{
    INIT_SERV = FSM_APP_STATE_START,  // 2
    READ,                             // 3
    VALIDATE,                         // 4
    TIE_GAME,                         // 5
    ERROR,                            // 6
} States;


typedef struct
{
    Environment common;
    char c, player_c;
    int code;
    bool player2_turn;
    char buff[3][2];
    int player[BACKLOG];
    struct sockaddr_in addr, player_addr[BACKLOG];
    int sfd, slen, client_num, turn;
} TTTEnvironment;

char playBoard[3][3] = {{'A','B','C'},
                        {'D','E','F'},
                        {'G','H','I'}};


int main()
{
    TTTEnvironment env;
    StateTransition transitions[] =
            {
                    { FSM_INIT,    INIT_SERV,     &init_server  },
                    { INIT_SERV,    READ,         &read_input   },
                    { READ,       ERROR,          &error        },
                    { READ,       VALIDATE,       &validate     },
                    {VALIDATE, ERROR,             &error        },
                    {VALIDATE, READ,              &read_input   },
                    {ERROR,     READ,             &read_input   },
                    {VALIDATE, INIT_SERV,         &init_server  },
                    { FSM_IGNORE, FSM_IGNORE, NULL      },
            };

    env.addr.sin_family = AF_INET;
    env.addr.sin_port = htons(PORT);
    env.addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(&(env.addr.sin_zero), 8);
    env.slen = sizeof(struct sockaddr_in);
    env.sfd = dc_socket(AF_INET, SOCK_STREAM, 0);

    env.client_num = 0;
    env.code = P1_TURN;
    env.turn = 0;
    env.buff[1][0] = '-';
    env.buff[2][0] = '-';

    int code;
    int start_state;
    int end_state;

    start_state = FSM_INIT;
    end_state   = INIT_SERV;
    code = fsm_run((Environment *)&env, &start_state, &end_state, transitions);

    if(code != 0)
    {
        fprintf(stderr, "Cannot move from %d to %d\n", start_state, end_state);

        return EXIT_FAILURE;
    }

//    fprintf(stderr, "Exiting state %d\n", start_state);
    dc_close(env.player[0]);
    dc_close(env.player[1]);
    return EXIT_SUCCESS;

}

static int init_server(Environment *env) {
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;

    dc_bind(game_env->sfd, (struct sockaddr *)&game_env->addr, sizeof(struct sockaddr_in));
    printf("WELCOME TO BIT SERVER'S TIC TAC TOE GAME\n");
    printf("Waiting for Players to join ...\n");
    while (game_env->client_num < BACKLOG) {
        dc_listen(game_env->sfd, BACKLOG);
        game_env->player[game_env->client_num] = dc_accept(game_env->sfd, (struct sockaddr *)&game_env->player_addr[game_env->client_num], &game_env->slen);
        //game_env->player[game_env->client_num] = dc_accept(game_env->sfd, 0, 0);
        game_env->client_num++;

        printf("%d/2 Player has joined\n", game_env->client_num);

        if (game_env->client_num == BACKLOG) {
            char *mess = "----- GAME BEGINS -----\n";
            printf("%s", mess);
            send(game_env->player[0], mess, strlen(mess), 0); // send message to player 1
            send(game_env->player[1], mess, strlen(mess), 0); // send message to player 2
            game_env->player2_turn = false;
            game_env->player_c = 'X';
        }
    }
    return READ;
}

static int read_input(Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;
    char *mess1 = "It's your turn, please place your move: ";
    char *mess2 = "Please wait for your turn\n";
    int turn = (game_env->player2_turn) ? 2 : 1;


    // will not read if buffer is not "reset" or "empty"
    if (game_env->buff[1][0] == '-' || game_env->buff[2][0] == '-') {
        /*if (turn) {
            send(game_env->player[game_env->player2_turn], mess1, strlen(mess1), 0);
            send(game_env->player[!game_env->player2_turn], mess2, strlen(mess2), 0);
        } else {
            send(game_env->player[!game_env->player2_turn], mess1, strlen(mess1), 0);
            send(game_env->player[game_env->player2_turn], mess2, strlen(mess2), 0);
        }*/
        if (!recv(game_env->player[game_env->player2_turn], game_env->buff[turn], 2, 0))
            perror("recv");
        game_env->turn++;
        printf("Player %d wrote: %s", turn, game_env->buff[turn]);
    }
    return VALIDATE;
}

static int validate(Environment *env) {
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *) env;
    game_env->c = game_env->buff[(game_env->player2_turn) ? 2 : 1][0];
    char *win = "----- You won! -----\n";
    char *lose = "----- You lose! -----\n";

    if (game_env->c < 'A' || game_env->c > 'I') {
        printf("error\n");
        game_env->code = INVALID_MOVE;
        game_env->turn--;
        return ERROR;
    }
    if (game_env->turn == TOTAL_TURNS) {
        char* mess = "----- GAME TIES -----\n";
        send(game_env->player[0], mess, strlen(mess), 0);
        send(game_env->player[1], mess, strlen(mess), 0);
        return INIT_SERV;
    }
    // VALID MOVE
    if (game_env->player2_turn){
        game_env->player_c = 'O';
//        char str[] = "O";
//        strncat(str, &(game_env->c), 1);
//        printf("%s\n", str);
//        send(game_env->player[game_env->player2_turn], str, sizeof(str), 0);
//        send(game_env->player[!game_env->player2_turn], str, sizeof(str), 0);
        game_env->player2_turn = false; // switch to player 1
    } else {
        game_env->player_c = 'X';
//        char str[] = "X";
//        strncat(str, &(game_env->c), 1);
//        printf("%s\n", str);
//        send(game_env->player[game_env->player2_turn], str, sizeof(str), 0);
//        send(game_env->player[!game_env->player2_turn], str, sizeof(str), 0);
        game_env->player2_turn = true;
    }

    update_board(game_env->c, playBoard, game_env->player_c, env);
    char key = check(playBoard);
    if (key == 'X') {
        game_env->code = P1_WIN;
        send(game_env->player[0], win, strlen(win), 0);
        send(game_env->player[1], lose, strlen(lose), 0);
        printf("----- Player 1 won! -----\n");
        return INIT_SERV;
    } else if (key == 'O') {
        game_env->code = P2_WIN;
        send(game_env->player[1], win, strlen(win), 0);
        send(game_env->player[0], lose, strlen(lose), 0);
        printf("----- Player 2 won! -----\n");
        return INIT_SERV;
    }
    /** RESET BUFFER */
    if (game_env->player2_turn) {
        game_env->buff[1][0] = '-';
        game_env->buff[2][0] = '-';
    }
    return READ;
}
static void update_board(char c, char board[][3], char player, Environment *env) {
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *) env;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (c == board[i][j]) {
                board[i][j] = player;
                break;
            }
        }
    }
    char b[] = "ABCDEFG";
//    strncat(b, &(board[0][0]), 1);
//    strncat(b, &(board[0][1]), 1);
//    strncat(b, &(board[0][2]), 1);
//    strncat(b, &(board[1][0]), 1);
//    strncat(b, &(board[1][1]), 1);
//    strncat(b, &(board[1][2]), 1);
//    strncat(b, &(board[2][0]), 1);
//    strncat(b, &(board[2][1]), 1);
//    strncat(b, &(board[2][2]), 1);
    send(game_env->player[0], b, sizeof(b), 0);
    send(game_env->player[1], b, sizeof(b), 0);

    /*printf("\n _________________\n");
    printf("|_____|_____|_____|\n");
    printf("|     |     |     |\n");
    printf("|  %c  |  %c  |  %c  |\n", board[0][0], board[0][1], board[0][2]);
    printf("|_____|_____|_____|\n");
    printf("|     |     |     |\n");
    printf("|  %c  |  %c  |  %c  |\n", board[1][0], board[1][1], board[1][2]);
    printf("|_____|_____|_____|\n");
    printf("|     |     |     |\n");
    printf("|  %c  |  %c  |  %c  |\n", board[2][0], board[2][1], board[2][2]);
    printf("|_____|_____|_____|\n");*/
    printf("%s", b);
}

char check(char playBoard[][3]) {
    int i;
    char key = ' ';

    // Check Rows
    for (i=0; i<3;i++)
        if (playBoard [i][0] == playBoard [i][1] && playBoard [i][0] == playBoard [i][2] && playBoard [i][0]) key = playBoard [i][0];
    // check Columns
    for (i=0; i<3;i++)
        if (playBoard [0][i] == playBoard [1][i] && playBoard [0][i] == playBoard [2][i] && playBoard [0][i]) key = playBoard [0][i];
    // Check Diagonals
    if (playBoard [0][0] == playBoard [1][1] && playBoard [1][1] == playBoard [2][2] && playBoard [1][1]) key = playBoard [1][1];
    if (playBoard [0][2] == playBoard [1][1] && playBoard [1][1] == playBoard [2][0] && playBoard [1][1]) key = playBoard [1][1];

    return key;
}

static int error(Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *) env;

    if (game_env->code == INVALID_MOVE) {
        char *mess = "Invalid move, place again!\n";
        send(game_env->player[game_env->player2_turn], mess, strlen(mess), 0);
    }
    return READ;
}

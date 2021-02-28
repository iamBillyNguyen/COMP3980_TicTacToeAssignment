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
#include <sys/time.h>
#include <netinet/in.h>
#include "shared.h"

#define BACKLOG 2
#define TOTAL_TURNS 9

static int init_server(Environment *env);
static int awaiting_new_player(Environment *env);
static int read_input(Environment *env);
static int validate(Environment *env);
static int error(Environment *env);
static void update_board(char c, char playBoard[][3], char player, Environment *env);
char check(char playBoard[][3]);
static void check_user_choice(Environment *env);

/**
 * NOTE:
 * - Server is not able to update current board to newly joined player... they can only
 * tell if the move is invalid or not. :(
 */

typedef enum
{
    INIT_SERV = FSM_APP_STATE_START, // 2
    READ,                            // 3
    VALIDATE,                        // 4
    ERROR,                           // 5
    MIDGAME_QUIT                             // 6
} States;

typedef struct
{
    Environment common;
    char c, player_c;
    char code[1];
    bool player2_turn;
    char buff[3][2];
    int player[BACKLOG];
    struct sockaddr_in addr, player_addr[BACKLOG];
    int sfd, slen, client_num, turn;
    int flag;
} TTTEnvironment;

char playBoard[3][3];
bool occupy[3][3];
char *mess1 = YES_TURN;
char *mess2 = NO_TURN;

int main()
{
    TTTEnvironment env;
    StateTransition transitions[] =
        {
            {FSM_INIT, INIT_SERV, &init_server},
            {INIT_SERV, READ, &read_input},
            {READ, ERROR, &error},
            {READ, VALIDATE, &validate},
            {VALIDATE, ERROR, &error},
            {VALIDATE, READ, &read_input},
            {ERROR, READ, &read_input},
            {VALIDATE, INIT_SERV, &init_server},
            {VALIDATE, MIDGAME_QUIT, &awaiting_new_player},
            {READ, MIDGAME_QUIT, &awaiting_new_player},
            {MIDGAME_QUIT, READ, &read_input},
            {FSM_IGNORE, FSM_IGNORE, NULL},
        };

    env.addr.sin_family = AF_INET;
    env.addr.sin_port = htons(PORT);
    env.addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(&(env.addr.sin_zero), 8);
    env.slen = sizeof(struct sockaddr_in);
    env.sfd = dc_socket(AF_INET, SOCK_STREAM, 0);
    dc_bind(env.sfd, (struct sockaddr *)&env.addr, sizeof(struct sockaddr_in));
    int enable = 1;
    if(setsockopt(env.sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR)");
    }

    env.client_num = 0; // VERY START OF GAME

    /** INIT FSM */
    int code;
    int start_state;
    int end_state;

    start_state = FSM_INIT;
    end_state = INIT_SERV;
    code = fsm_run((Environment *)&env, &start_state, &end_state, transitions);

    if (code != 0)
    {
        fprintf(stderr, "Cannot move from %d to %d\n", start_state, end_state);

        return EXIT_FAILURE;
    }

    //    fprintf(stderr, "Exiting state %d\n", start_state);
    dc_close(env.player[0]);
    dc_close(env.player[1]);
    return EXIT_SUCCESS;
}

void reset_game(Environment *env) {
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;

    game_env->code[0] = YES_TURN[0];
    game_env->turn = 0;
    game_env->buff[1][0] = '-';
    game_env->buff[2][0] = '-';
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i == 0) {
                playBoard[i][j] = (char) ('A' + j);
            } else if (i == 1) {
                playBoard[i][j] = (char) ('D' + j);
            } else {
                playBoard[i][j] = (char) ('G' + j);
            }
            occupy[i][j] = false;
        }
    }
}

void start_game(Environment *env) {
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;

    char* mess = GAME_BEGIN;
    printf("----- GAME BEGINS -----\n");
    send(game_env->player[0], mess, strlen(mess), 0); // send message to player 1
    send(game_env->player[1], mess, strlen(mess), 0); // send message to player 2
    game_env->player2_turn = false;
    game_env->player_c = 'X';

    send(game_env->player[0], mess1, strlen(mess1), 0);
    send(game_env->player[1], mess2, strlen(mess2), 0);
}

static int init_server(Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;
    fd_set readfds;
    int max_sd, retval;

    reset_game(env);

    printf("----- BIT SERVER'S TIC TAC TOE GAME -----\n");
    while (1) {
        FD_ZERO(&readfds);
        // Adding server socket to set
        FD_SET(game_env->sfd, &readfds);
        max_sd = game_env->sfd;
        // Adding child socket to set
//        while (game_env->client_num < BACKLOG) {
        if (game_env->client_num != BACKLOG) {
            printf("Waiting for Players to join ...\n");
            dc_listen(game_env->sfd, BACKLOG);

//
//            FD_SET(game_env->player[game_env->client_num], &readfds);
//            game_env->client_num++;
//            printf("%d/2 Player has joined\n", game_env->client_num);
//
//        }
//        if (game_env->client_num % 2 == 0 && game_env->client_num != 0) {
//            start_game(env);
//        }
            retval = select(max_sd + 1, &readfds, NULL, NULL, NULL);
            if (retval == -1) {
                perror("select");
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < BACKLOG; i++) {
                if (FD_ISSET(game_env->sfd, &readfds)) {
                    FD_SET(game_env->player[i], &readfds); // Adding players to socket
                    game_env->player[i] = dc_accept(game_env->sfd,
                                                    (struct sockaddr *) &game_env->player_addr[game_env->client_num],
                                                    &game_env->slen);
                }
                game_env->client_num++;
            }
        }
        start_game(env);
        return READ;
    }
    return READ;
}

static int awaiting_new_player(Environment *env) {
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;
    while (game_env->client_num < BACKLOG) {
        char* mess = MIDGAME_QUIT;
        dc_listen(game_env->sfd, BACKLOG);
        game_env->player[game_env->client_num] = dc_accept(game_env->sfd,(struct sockaddr *) &game_env->player_addr[game_env->client_num],
                                                           &game_env->slen);
        game_env->client_num++;
        printf("%d/2 Player has joined\n", game_env->client_num);
        if (game_env->player2_turn)
        {
            game_env->player2_turn = false; // switch to player 1
            send(game_env->player[0], mess1, strlen(mess1), 0);
            send(game_env->player[1], mess2, strlen(mess2), 0);

        }
        else
        {
            game_env->player2_turn = true;
            send(game_env->player[1], mess1, strlen(mess1), 0);
            send(game_env->player[0], mess2, strlen(mess2), 0);

        }
    }
    return READ;
}

static int read_input(Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;
    
    int turn = (game_env->player2_turn) ? 2 : 1;

    // will not read if buffer is not "reset" or "empty"
    if (game_env->buff[1][0] == '-' || game_env->buff[2][0] == '-')
    {   
        if (!recv(game_env->player[game_env->player2_turn], game_env->buff[turn], 2, 0)) {
            printf("A player has quit!\nAwaiting for new player to connect as Player %d\n",
                   (int) (game_env->player2_turn + 1));
            game_env->client_num--;
            return MIDGAME_QUIT;
        } else {
            game_env->turn++;
            printf("Player %d wrote: %c\n", turn, game_env->buff[turn][0]);
        }
    }
    return VALIDATE;
}

static int validate(Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;
    game_env->c = game_env->buff[(game_env->player2_turn) ? 2 : 1][0];

    if (game_env->c == 0 || game_env->c == EOF || game_env->c == '-') { // If player quits midway
        printf("Player quits midgame\n");
        // Accept a new player
        //game_env->player[game_env->player2_turn] = dc_accept(game_env->sfd, (struct sockaddr *)&game_env->player_addr[game_env->player2_turn], &(game_env->slen));
        return MIDGAME_QUIT;
    }
    if (game_env->c < 'A' || game_env->c > 'I')
    {
        game_env->code[0] = INVALID_MOVE[0];
        game_env->turn--;
        return ERROR;
    }
    int index = (int)(game_env->c) - 65;
    if (index < 3)
    {
        if (occupy[0][index])
        {
            game_env->code[0] = INVALID_MOVE[0];
            game_env->turn--;
            return ERROR;
        }
    }
    else if (index > 2 && index < 6)
    {
        if (occupy[1][index-3])
        {
            game_env->code[0] = INVALID_MOVE[0];
            game_env->turn--;
            return ERROR;
        }
    }
    else if (index > 5)
    {
        if (occupy[2][index-6])
        {
            game_env->code[0] = INVALID_MOVE[0];
            game_env->turn--;
            return ERROR;
        }
    }
    if (game_env->player2_turn)
    {
        game_env->player_c = 'O';
    }
    else
    {
        game_env->player_c = 'X';
    }

    update_board(game_env->c, playBoard, game_env->player_c, env);
    /** SEND POSITION/CHOICE TO CLIENTS */
    char choice[1];
    choice[0] = game_env->c;
    send(game_env->player[0], choice, sizeof(choice), 0);
    send(game_env->player[1], choice, sizeof(choice), 0);

    char key = check(playBoard);
    if (key == 'X') {
        game_env->code[0] = WIN[0];
        send(game_env->player[1], LOSE, strlen(LOSE), 0);
        send(game_env->player[0], WIN, strlen(WIN), 0);
        printf("----- Player 1 won! -----\n");
        check_user_choice(env);
        return INIT_SERV;
    } else if (key == 'O') {
         game_env->code[0] = WIN[0];
         send(game_env->player[1], WIN, strlen(WIN), 0);
         send(game_env->player[0], LOSE, strlen(LOSE), 0);
         printf("----- Player 2 won! -----\n");
         check_user_choice(env);
         return INIT_SERV;
    }

    if (game_env->turn == TOTAL_TURNS)
    {
        printf("----- GAME TIES -----\n");
        game_env->code[0] = TIE[0];
        send(game_env->player[1], TIE, strlen(TIE), 0);
        send(game_env->player[0], TIE, strlen(TIE), 0);
        check_user_choice(env);
        return INIT_SERV;
    }

    // SWITCH TURN AFTER DONE VALIDATION
    if (game_env->player2_turn)
    {
        game_env->player2_turn = false; // switch to player 1
        send(game_env->player[0], mess1, strlen(mess1), 0);
        send(game_env->player[1], mess2, strlen(mess2), 0);
        
    }
    else
    {
        game_env->player2_turn = true;
        send(game_env->player[1], mess1, strlen(mess1), 0);
        send(game_env->player[0], mess2, strlen(mess2), 0);
        
    }
    
    /** RESET BUFFER */
    if (game_env->player2_turn)
    {
        game_env->buff[1][0] = '-';
        game_env->buff[2][0] = '-';
    }
    
    return READ;
}
static void update_board(char c, char board[][3], char player, Environment *env)
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (c == board[i][j])
            {
                board[i][j] = player;
                occupy[i][j] = true;
                break;
            }
        }
    }

    printf("    %c  %c  %c\n", board[0][0], board[0][1], board[0][2]);
    printf("    %c  %c  %c\n", board[1][0], board[1][1], board[1][2]);
    printf("    %c  %c  %c\n", board[2][0], board[2][1], board[2][2]);
}

/** CHECK FOR WIN, LOSE, OR TIE */
char check(char playBoard[][3])
{
    int i;
    char key = ' ';

    // Check Rows
    for (i = 0; i < 3; i++)
        if (playBoard[i][0] == playBoard[i][1] && playBoard[i][0] == playBoard[i][2])
            key = playBoard[i][0];
    // check Columns
    for (i = 0; i < 3; i++)
        if (playBoard[0][i] == playBoard[1][i] && playBoard[0][i] == playBoard[2][i])
            key = playBoard[0][i];
    // Check Diagonals
    if (playBoard[0][0] == playBoard[1][1] && playBoard[1][1] == playBoard[2][2])
        key = playBoard[1][1];
    if (playBoard[0][2] == playBoard[1][1] && playBoard[1][1] == playBoard[2][0])
        key = playBoard[1][1];

    return key;
}

static int error(Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;

    if (game_env->code[0] == INVALID_MOVE[0])
    {
        char* mess3 = INVALID_MOVE;
        char* mess4 = WAIT;
        printf("Invalid move player %d, place again!\n", game_env->player2_turn ? 2 : 1);
        send(game_env->player[game_env->player2_turn], mess3, strlen(mess3), 0);
        send(game_env->player[!game_env->player2_turn], mess4, strlen(mess4), 0);
    }
    return READ;
}

static void check_user_choice(Environment *env) {
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;
    int player_num = 2;

    recv(game_env->player[0], game_env->buff[1], 2, 0);
    recv(game_env->player[1], game_env->buff[2], 2, 0);
    if (game_env->buff[1][0] != 'r') {
        memset(&(game_env->player[0]), 0, sizeof(game_env->player[0]));
        player_num--;
    }
    if (game_env->buff[2][0] != 'r') {
        memset(&(game_env->player[1]), 0, sizeof(game_env->player[1]));
        player_num--;
    }
    if (player_num == 1) {
        game_env->client_num--;
    } else if (player_num == 0){
        game_env->client_num = 0;
    }
}

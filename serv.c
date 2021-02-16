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
static int write_code(Environment *env);
static int validate(Environment *env);
static int prompt(Environment *env);
static int read_error(Environment *env);
static int write_error(Environment *env);

typedef enum
{
    INIT_SERV = FSM_APP_STATE_START,  // 2
    READ,
    WRITE,                       // 3
    VALIDATE,                    // 4
    TIE_GAME,                         // 7
    ERROR,                       // 8
} States;


typedef struct
{
    Environment common;
    char c;
    int code;
    bool player2_turn;
    char buff[3][2];
    int player[BACKLOG];
    struct sockaddr_in addr, player_addr[BACKLOG];
    int sfd, slen, client_num, turn;
} TTTEnvironment;


int main()
{
    TTTEnvironment env;
//    env.c = 'A';
//    env.error_code = 0;
//    env.player2_turn = false;
    StateTransition transitions[] =
            {
                    { FSM_INIT,    INIT_SERV,     &init_server   },
                    { INIT_SERV,    READ,     &read_input   },
                    { READ,       ERROR,      &read_error   },
                    { READ,       VALIDATE,   &validate   },
                    {VALIDATE, ERROR,          &write_error},
                    {VALIDATE, READ, &read_input},
                    {ERROR,     READ,           &read_input},
                    { TIE,       FSM_EXIT,NULL      },
                    { WRITE,      ERROR,      &write_error  },
                    { WRITE,      READ,       &read_input   },
                    { FSM_IGNORE, FSM_IGNORE, NULL  },
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
        if (game_env->client_num == 1) {
            char *mess = "Awaiting for Player 2 ... \n";
            send(game_env->player[0], mess, strlen(mess), 0); // send message to player 1
        }

        printf("%d/2 Player has joined\n", game_env->client_num);

        if (game_env->client_num == BACKLOG) {
            char *mess = "----- GAME BEGINS -----\n";
            printf("%s", mess);
            send(game_env->player[0], mess, strlen(mess), 0); // send message to player 1
            send(game_env->player[1], mess, strlen(mess), 0); // send message to player 2
            game_env->player2_turn = false;
        }
    }
    return READ;
}

static int read_input(Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;
    char *mess = "It's your turn, please place your move: ";
    int turn = (game_env->player2_turn) ? 2 : 1;

    send(game_env->player[game_env->player2_turn], mess, strlen(mess), 0);
    if (!recv(game_env->player[game_env->player2_turn], game_env->buff[turn], 2, 0))
        perror("recv");
    game_env->turn++;
    printf("Player wrote %s", game_env->buff[turn]);

    return VALIDATE;
}

static int validate(Environment *env) {
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *) env;
    game_env->c = game_env->buff[(game_env->player2_turn) ? 2 : 1][0];
    /*if ((game_env->c != 'A') || (game_env->c != 'B') || game_env->c != 'C' ||
        game_env->c != 'D' || game_env->c != 'E' || game_env->c != 'F' ||
        game_env->c != 'G' || game_env->c != 'H' || game_env->c != 'I') {
        game_env->code = INVALID_MOVE;
        game_env->turn--;
        return ERROR;
    } else */
    if (game_env->turn == TOTAL_TURNS) {
        char *mess = "4";
        send(game_env->player[0], mess, strlen(mess), 0);
        send(game_env->player[0], mess, strlen(mess), 0);
        return TIE_GAME;
    }
    if (game_env->player2_turn){
        game_env->player2_turn = false; // switch to player 2
    } else {
        game_env->player2_turn = true;
    }

    return READ;
}

static int write_code(Environment *env)
{
    TTTEnvironment *echo_env;
    int              ret_val;

    echo_env = (TTTEnvironment *)env;
    //ret_val = putchar(echo_env->c);

    /*if(ret_val == EOF)
    {
        return ERROR;
    }*/

    return READ;
}

static int prompt(Environment *env)
{
    char* prompt = "Prompt: Place a move\n";
    TTTEnvironment *echo_env;
    int              ret_val;

    echo_env = (TTTEnvironment *)env;
    /*ret_val = echo_env->code;
    if (moves == 5) {
        printf("==========TIE==========\n");
        return FSM_EXIT;
    }
    switch (ret_val) {
        case 400:
            printf("Invalid player\n");
            break;
        case 401:
            printf("Renter your move\n");
            break;
        case 402:
            printf("Renter your move\n");
            break;
        default:
            printf("%s", prompt);
            break;
    }
    echo_env->code = 0;*/
    return READ;
}

static int read_error(Environment *env)
{
    TTTEnvironment *echo_env;
    int              ret_val;

    echo_env = (TTTEnvironment *)env;
    //ret_val = echo_env->code;
    return READ;
}

static int write_error(Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;
    char *mesg = "";
    switch (game_env->code) {
        case INVALID_MOVE:
            mesg = "Invalid move! Please enter again\n";
            break;
        default:
            break;
    }

    return READ;
}

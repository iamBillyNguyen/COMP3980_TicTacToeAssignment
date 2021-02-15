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

static int init_server(Environment *env);
static int read_input(Environment *env);
static int write_code(Environment *env);
static int validate(Environment *env);
static int prompt(Environment *env);
static int read_error(Environment *env);
static int write_error(Environment *env);

int id[BACKLOG], player[BACKLOG];
int response_code = 0;
char a[2][40];
int sfd[BACKLOG], client_num = 0;
char x[4];

typedef enum
{
    INIT_SERV = FSM_APP_STATE_START,  // 2
    READ,
    WRITE,                       // 3
    VALIDATE,                    // 4
    TIE,                         // 7
    ERROR,                       // 8
} States;


typedef struct
{
    Environment common;
    char c;
    int error_code;
    bool player2_turn;
} TTTEnvironment;


int main()
{
    TTTEnvironment env;
    env.c = 'A';
    env.error_code = 0;
    env.player2_turn = false;
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
    dc_close(player[0]);
    dc_close(player[1]);
    return EXIT_SUCCESS;

}

static int init_server(Environment *env) {
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;
    printf("WELCOME TO BIT SERVER'S TIC TAC TOE GAME\n");
    printf("Waiting for Players to join ...\n");
    strcpy(a[0],"Waiting for the other Player to join\n");
    struct sockaddr_in addr;


    sfd[0] = dc_socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dc_bind(sfd[0], (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

    while (client_num < BACKLOG) {
        dc_listen(sfd[0], BACKLOG);

        player[client_num] = dc_accept(sfd[0], 0, 0);
        client_num++;
        if (client_num == 1) {
            strcpy(a[1], "0");
            dc_write(player[0], a, sizeof(a));
            dc_read(player[0], x, sizeof(x)); // read from player 1
            id[0] = atoi(x);
        }
        printf("Player %d has joined\n", client_num);

        if (client_num == BACKLOG) {
            strcpy(a[0],"-------- GAME BEGINS --------\n");
            strcpy(a[1],"1");
            dc_write(player[0],a,sizeof(a));
            strcpy(a[1],"2");
            dc_write(player[1],a,sizeof(a)); // read from player 2
            dc_read(player[1],x,sizeof(x));
            id[1]=atoi(x);
        }
    }

    // if (fork() == 0) {
        // printf("in fork\n");
        game_env->player2_turn = true;
        printf("hello?%d", game_env->player2_turn);
        return READ;
    // }
    // return READ;
}

static int read_input(Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;
    char c[1];
    dc_read(player[game_env->player2_turn], c, sizeof(c));
    game_env->c = c[0];
    printf("Player wrote %c", game_env->c);

    if(game_env->c == EOF)
    {
        if(ferror(stdin))
        {
            return ERROR;
        }
        printf("EOF\n");
        return FSM_EXIT;
    }

    return VALIDATE;
}

static int validate(Environment *env) {
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *) env;

    if (game_env->c != '1' || game_env->c != '2' || game_env->c != '3' ||
        game_env->c != '4' || game_env->c != '5' || game_env->c != '6' ||
        game_env->c != '7' || game_env->c != '8' || game_env->c != '9') {
        game_env->error_code = 0;
        return ERROR;
    } else {
        game_env->player2_turn = true; // switch to player 2
        return READ;
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
    switch (game_env->error_code) {
        case 0:
            mesg = "Invalid move! Please enter again\n";
            break;
        default:
            break;
    }
    dc_write(player[game_env->player2_turn], mesg, strlen(mesg));

    return READ;
}

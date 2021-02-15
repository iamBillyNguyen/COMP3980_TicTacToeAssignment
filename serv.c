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

static int read_input(Environment *env);
static int write_code(Environment *env);
static int validate(Environment *env);
static int prompt(Environment *env);
static int read_error(Environment *env);
static int write_error(Environment *env);

int moves = 0;
int id[BACKLOG], player[BACKLOG];
int response_code = 0;
char a[2][40];

typedef enum
{
    READ = FSM_APP_STATE_START,  // 2
    WRITE,                       // 3
    VALIDATE,                    // 4
    WIN,                         // 5
    LOSE,                        // 6
    TIE,                         // 7
    ERROR,                       // 8
} States;


typedef struct
{
    Environment common;
    int c, error_code;
    bool player2_turn;
} TTTEnvironment;


int main(int argc, char *argv[])
{
    TTTEnvironment env;
    StateTransition transitions[] =
            {
                    { FSM_INIT,    READ,     &read_input   },
                    { READ,       ERROR,      &read_error   },
                    { READ,       VALIDATE,   &validate   },
                    {VALIDATE, ERROR, &write_error},
                    {ERROR, READ, &read_input},
                    { READ,       TIE,        &prompt       },
                    { TIE,       FSM_EXIT,NULL      },
                    { READ,       FSM_EXIT,   NULL  },
                    { WRITE,      ERROR,      &write_error  },
                    { WRITE,      READ,       &read_input   },
                    { FSM_IGNORE, FSM_IGNORE, NULL          },
            };
    char board[3][3] =   {{'A','B','C'},
                          {'D','E','F'},
                          {'G','H','I'}};

    //init_server();
    printf("WELCOME TO BIT SERVER'S TIC TAC TOE GAME\n");
    printf("Waiting for Players to join ...\n");
    strcpy(a[0],"Waiting for the other Player to join\n");
    struct sockaddr_in addr;
    int sfd[BACKLOG], client_num = 0;
    char x[4];

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
        printf("%d\n", client_num);
        printf("Awaiting for player 2 ... \n");
        if (client_num == 1) {
            strcpy(a[1], "0");
            dc_write(player[0], a, sizeof(a));
            dc_read(player[0], x, strlen(x));
            id[0] = atoi(x);
        }
        printf("Player %d has joined\n", client_num);

        if (client_num == BACKLOG) {
            strcpy(a[0],"-------- GAME BEGINS --------");
            strcpy(a[1],"1");
            write(player[0],a,sizeof(a));
            strcpy(a[1],"2");
            write(player[1],a,sizeof(a));
            read(player[1],x,sizeof(x));
            id[1]=atoi(x);
        }
    }

    if (fork() == 0) {
        env.player2_turn = false;
        int code;
        int start_state;
        int end_state;

        start_state = FSM_INIT; // 0
        end_state   = READ;
        code = fsm_run((Environment *)&env, &start_state, &end_state, transitions);
        printf("In fork\n");
        if(code != 0)
        {
            fprintf(stderr, "Cannot move from %d to %d\n", start_state, end_state);

            return EXIT_FAILURE;
        }
    }


//    fprintf(stderr, "Exiting state %d\n", start_state);
    dc_close(player[0]);
    dc_close(player[1]);

    return EXIT_SUCCESS;
}

static int read_input(Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;

    dc_read(player[game_env->player2_turn], game_env->c, sizeof(game_env));
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
    game_env = (TTTEnvironment *)env;
    char pos[9] = {'A', 'B', 'C',
                   'D', 'E', 'F',
                   'G', 'H', 'I'};
    for (int i = 0; i < sizeof(pos); i++) {
        if (game_env->c != pos[i])
            return ERROR;
        else {
            game_env->player2_turn = true; // switch to player 2
            return READ;
        }
    }
}

static int write_code(Environment *env)
{
    TTTEnvironment *echo_env;
    int              ret_val;

    echo_env = (TTTEnvironment *)env;
    ret_val = putchar(echo_env->c);

    if(ret_val == EOF)
    {
        return ERROR;
    }

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

}

static int write_error(Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;
    char *mesg;
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

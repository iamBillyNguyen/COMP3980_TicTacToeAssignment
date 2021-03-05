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
#include <dc/stdlib.h>
#include <dc/stdio.h>
#include <dc/unistd.h>
#include <dc/sys/socket.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>
#include "shared.h"
#include "TTTGame.c"

#define BACKLOG 2
#define N 10

typedef struct {
    Environment common;
    struct sockaddr_in addr;
    int sfd, slen, max_sd, size, index, client_num, len;
    fd_set readfds, workingfds;
    int *player_socket;
    TTTEnvironment *game_list;
} ServEnvironment;

static int init_server(Environment *env);
static int server_error(Environment *env);
static int accept_serv(Environment *env);
static int bind_serv(Environment *env);
static int listen_serv(Environment *env);
static int clean_up_serv(Environment *env);


/**
 * NOTE:
 * - Server is not able to update current board to newly joined player... they can only
 * tell if the move is invalid or not. :(
 */

typedef enum
{
    INIT_SERV = FSM_APP_STATE_START, // 2
    BIND,                            // 3
    LISTEN,                          // 4
    ACCEPT,                          // 5
    CLEAN_UP,                        // 6
    ERROR_SERV                       // 7
} States;

int main()
{
    ServEnvironment env;
    StateTransition transitions[] =
            {
                    {FSM_INIT,   INIT_SERV, &init_server},
                    {INIT_SERV, BIND, &bind_serv},
                    {BIND, LISTEN, &listen_serv},
                    {LISTEN, ACCEPT, &accept_serv},
                    {INIT_SERV,  ERROR_SERV,     &server_error},
                    {BIND,  ERROR_SERV,     &server_error},
                    {ACCEPT,  ERROR_SERV,     &server_error},
                    {ACCEPT, CLEAN_UP, &clean_up_serv},
                    {ERROR_SERV, FSM_EXIT, NULL},
                    {CLEAN_UP, FSM_EXIT, NULL},
                    {FSM_IGNORE, FSM_IGNORE, NULL},
            };

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
    free(env.player_socket);
    free(env.game_list);
    return EXIT_SUCCESS;
}

static int init_server(Environment *env) {
    ServEnvironment *serv_env;
    serv_env = (ServEnvironment *) env;

    serv_env->slen = sizeof(struct sockaddr_in);
    memset(&serv_env->addr, 0, sizeof(serv_env->slen));
    serv_env->addr.sin_family = AF_INET;
    serv_env->addr.sin_port = htons(PORT);
    serv_env->addr.sin_addr.s_addr = htonl(INADDR_ANY);

    serv_env->size = 2;
    serv_env->index = 0;
    serv_env->client_num = 0;
    serv_env->game_list = (TTTEnvironment *)dc_malloc(sizeof(TTTEnvironment));
    serv_env->player_socket = (int *)dc_malloc(2 * sizeof(int));
    printf("%d\n", serv_env->player_socket[1]);
    return BIND;
}

static int bind_serv(Environment *env) {
    ServEnvironment *serv_env;
    serv_env = (ServEnvironment *) env;
    int enable = 1;

    serv_env->sfd = dc_socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(serv_env->sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
    }
    dc_bind(serv_env->sfd, (struct sockaddr *) &serv_env->addr, sizeof(struct sockaddr_in));
    return LISTEN;
}

static int listen_serv(Environment *env) {
    ServEnvironment *serv_env;
    serv_env = (ServEnvironment *) env;

    dc_listen(serv_env->sfd, N);
    printf("----- BIT SERVER'S TIC TAC TOE GAME -----\n");
    printf("Waiting for Players to join ...\n");

    return ACCEPT;
}

void assign_player(ServEnvironment *serv_env) {
    // Assigning 2 new players to a game
    int a = (serv_env->player_socket[serv_env->client_num - 2] - serv_env->sfd - 1) / 2;

    if (serv_env->client_num == serv_env->size) {
        for (int j = (serv_env->client_num - 2), x = 0; j < serv_env->client_num, x < 2; j++, x++) {
            serv_env->game_list[a].player[x] = serv_env->player_socket[j];
        }
        serv_env->game_list[a].started = true;
    }
}

void set_new_game(ServEnvironment *serv_env) {
    TTTEnvironment *game_env;

    game_env = dc_malloc(sizeof(TTTEnvironment));
    init_game((Environment*)game_env);
    serv_env->game_list[serv_env->index] = *game_env;

    assign_player(serv_env);

    send(serv_env->game_list[serv_env->index].player[0], YES_TURN, strlen(YES_TURN), 0);
    send(serv_env->game_list[serv_env->index].player[1], NO_TURN, strlen(NO_TURN), 0);

    serv_env->size += NUM_PLAYER_PER_GAME;
    serv_env->index++;
}

static int accept_serv(Environment *env) {
    ServEnvironment *serv_env;
    serv_env = (ServEnvironment *) env;
    int rc, retval, desc, len, i, new_sd, close_conn;

    char buffer[2];

    FD_ZERO(&(serv_env->readfds));
    FD_SET(serv_env->sfd, &(serv_env->readfds));
    serv_env->max_sd = serv_env->sfd;

    while (1) {
        memcpy(&(serv_env->workingfds), &(serv_env->readfds), sizeof(serv_env->readfds));
        retval = select(serv_env->max_sd + 1, &(serv_env->workingfds), NULL, NULL, NULL);
        if (retval < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }
        if (retval == 0) {
            break;
        }
        printf("retval %d\n", retval);
        desc = retval;
        for (i = 0; i <= serv_env->max_sd && desc > 0; ++i) {
            if (FD_ISSET(i, &(serv_env->workingfds))) {
                desc -= 1;
                //if (serv_env->client_num <= BACKLOG) {
                    if (i == serv_env->sfd) {
                        printf("  Listening socket is readable\n");
                        do {
                            new_sd = dc_accept(serv_env->sfd, NULL, NULL);
                            if (new_sd < 0) {
                                perror("  accept() failed");
                                return ERROR_SERV;
                            }
                            printf("  New incoming connection - %d\n", new_sd);
                            FD_SET(new_sd, &(serv_env->readfds));
                            if (serv_env->client_num + 1 == NUM_PLAYER_PER_GAME) {
                                TTTEnvironment *tmp = (TTTEnvironment *)realloc(serv_env->game_list, NUM_PLAYER_PER_GAME * sizeof(TTTEnvironment));
                                int *tmp2 = (int *)realloc(serv_env->player_socket, sizeof(int));

                                if (!tmp || !tmp2) {
                                    perror("realloc\n");
                                    return ERROR_SERV;
                                }
                                serv_env->game_list = tmp;
                                serv_env->player_socket = tmp2;
                            }

                            serv_env->player_socket[serv_env->client_num] = new_sd;
                            serv_env->client_num++;
                            if (new_sd > serv_env->max_sd)
                                serv_env->max_sd = new_sd;
                            int a = (new_sd - serv_env->sfd - 1) / 2;
                            if (new_sd % 2 == 0) {
                                if (serv_env->game_list[a].started) { // midgame quit
                                    send(new_sd, YES_TURN, strlen(YES_TURN), 0);
                                    assign_player(serv_env);
                                } else {
                                    send(new_sd, WAIT, strlen(WAIT), 0);
                                }
                            } else {
                                if (serv_env->game_list[a].player2_turn) { // midgame quit
                                    assign_player(serv_env);
                                    send(new_sd, O, strlen(O), 0);
                                    send(new_sd, YES_TURN, strlen(YES_TURN), 0);
                                } else {
                                    set_new_game(serv_env);
                                }
                            }
                            break;
                        } while (new_sd != -1);
                    } else {
                        printf("  Descriptor %d is readable\n", i);
                        close_conn = 0;
                        do {
                            if (i > serv_env->sfd) {

                                // check if the connection has closed or not
                                // close connection if yes
                                if (!recv(i, buffer, sizeof(buffer), 0)) {
                                    printf("A player has quit!\nAwaiting for new player to connect\n");
                                    serv_env->client_num--;
                                    close_conn = 1;
                                    break;
                                } else {
                                    for (int z = 0; z < serv_env->index + 1; z++) {
                                        for (int y = 0; y < 2; y++) {
                                            if (serv_env->game_list[z].player[y] == i) {
                                                serv_env->game_list[z].turn++;
                                                serv_env->game_list[z].c = buffer[0];
                                                printf("Player %d wrote: %c\n", (i - serv_env->sfd),
                                                       serv_env->game_list[z].c);
                                                serv_env->game_list[z].c = buffer[0];
                                                handle_move(&(serv_env->game_list[z]));
                                                break;
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        } while (1);
                        for (int z = 0; z < serv_env->client_num / 2; z++) {
                            if (buffer[0] == 'r') {
                                printf("replay\n");
                                if (i % 2 == 0) {
                                    send(i, YES_TURN, strlen(YES_TURN), 0);
                                    serv_env->game_list[z].player[0] = i;
                                } else {
                                    send(i, NO_TURN, strlen(NO_TURN), 0);
                                    serv_env->game_list[z].player[1] = i;
                                    init_game((Environment*)(&serv_env->game_list[z]));
                                }
                            }
                        }
                        if (close_conn) {
                            dc_close(i);
                            FD_CLR(i, &(serv_env->readfds));
                            if (i == serv_env->max_sd)
                            {
                                while (FD_ISSET(serv_env->max_sd, &(serv_env->readfds)) == 0)
                                    serv_env->max_sd -= 1;
                            }
                        }
                    }
                }
            }
        }
    return CLEAN_UP;
}

static int clean_up_serv(Environment *env) {
    ServEnvironment *serv_env;
    serv_env = (ServEnvironment *)env;

    for (int i=0; i <= serv_env->max_sd; ++i)
    {
        if (FD_ISSET(i, &(serv_env->readfds)))
            dc_close(i);
    }

    return FSM_EXIT;
}

static int server_error(Environment *env) {
    ServEnvironment *serv_env;
    serv_env = (ServEnvironment *)env;

    char *message;
    char *str;

    message = strerror(errno);

    // 1 is the # of digits for the states +
    // 3 is " - " +
    // strlen is the length of the system message
    // 1 is null +
    str = malloc(1 + strlen(message) + 3 + 1);
    sprintf(str, "%d - %s", serv_env->common.current_state_id, str);
    perror(str);

    return FSM_EXIT;
}

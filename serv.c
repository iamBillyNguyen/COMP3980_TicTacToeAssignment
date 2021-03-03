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

#define BACKLOG 4


typedef struct {
    Environment common;
    struct sockaddr_in addr;
    int sfd, slen, max_sd;
    fd_set readfds, workingfds;
    int player_socket[BACKLOG];
    TTTEnvironment *game_list[BACKLOG];
} ServEnvironment;

static int init_server(Environment *env);
//static int awaiting_new_player(Environment *env);
static int server_error(Environment *env);
static int accept_serv(Environment *env);
static int bind_serv(Environment *env);
static int listen_serv(Environment *env);


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
    ERROR_SERV                       // 6
} States;


char *mess1 = YES_TURN;
char *mess2 = NO_TURN;

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
    return EXIT_SUCCESS;
}

static int init_server(Environment *env) {
    ServEnvironment *serv_env;
    serv_env = (ServEnvironment *) env;

//    rc = ioctl(game_env->sfd, FIONBIO, (char *) &enable);
//    if (rc < 0) {
//        perror("ioctl() failed");
//        close(game_env->sfd);
//        exit(-1);
//    }

    serv_env->slen = sizeof(struct sockaddr_in);
    memset(&serv_env->addr, 0, sizeof(serv_env->slen));
    serv_env->addr.sin_family = AF_INET;
    serv_env->addr.sin_port = htons(PORT);
    serv_env->addr.sin_addr.s_addr = htonl(INADDR_ANY);

    for (int i = 0; i < BACKLOG; i++) {
        serv_env->player_socket[i] = 0;
    }

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

    dc_listen(serv_env->sfd, BACKLOG);
    printf("----- BIT SERVER'S TIC TAC TOE GAME -----\n");
    printf("Waiting for Players to join ...\n");

    TTTEnvironment *game_env;
    game_env = dc_malloc(sizeof(TTTEnvironment));
    set_game((Environment*)game_env);
    serv_env->game_list[0] = game_env;
    return ACCEPT;
}

static int accept_serv(Environment *env) {
    ServEnvironment *serv_env;
    serv_env = (ServEnvironment *) env;
    int rc, retval, desc, len, i, new_sd, close_conn, client_num = 0;
    char buffer[2];

    FD_ZERO(&(serv_env->readfds));
    FD_SET(serv_env->sfd, &(serv_env->readfds));
    serv_env->max_sd = serv_env->sfd;

    while (1) {
        memcpy(&(serv_env->workingfds), &(serv_env->readfds), sizeof(serv_env->readfds));
        // Adding child socket to set
//        if (game_env->client_num == BACKLOG) {
//            send(game_env->player[game_env->player2_turn], mess1, strlen(mess1), 0);
//            send(game_env->player[!game_env->player2_turn], mess2, strlen(mess2), 0);
//        }
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
                if (client_num <= BACKLOG) {
                    if (i == serv_env->sfd) {
                        printf("  Listening socket is readable\n");
                        do {
                            new_sd = dc_accept(serv_env->sfd, NULL, NULL);
                            if (new_sd < 0) {
                                perror("  accept() failed");
                                break;
                            }
                            printf("  New incoming connection - %d\n", new_sd);
                            FD_SET(new_sd, &(serv_env->readfds));
                            serv_env->player_socket[client_num] = new_sd;
                            client_num++;
                            if (new_sd > serv_env->max_sd)
                                serv_env->max_sd = new_sd;
                            //send(i, GAME_BEGIN, strlen(GAME_BEGIN), 0); // send message to player 1
                            if (new_sd % 2 == 0)
                                send(new_sd, YES_TURN, strlen(YES_TURN), 0);
                            else
                                send(new_sd, NO_TURN, strlen(NO_TURN), 0);
                            break;

                        } while (new_sd != -1);
                    } else {
                        printf("  Descriptor %d is readable\n", i);
                        close_conn = 0;
                        do {
                            if (i > serv_env->sfd) {
//                                rc = recv(i, buffer, sizeof(buffer), 0);
//                                if (buffer[0] == EOF || buffer[0] == '\n') {
//                                    dc_close(i);
//                                    FD_CLR(i, &(serv_env->readfds));
//                                    break;
//                                }
//                                //game_env->player2_turn = !game_env->player2_turn;
//                                if (rc < 0) {
//                                    perror("recv\n");
//                                    break;
//                                }
//                                if (rc == 0) {
//                                    printf("  Connection closed\n");
//                                    close_conn = 1;
//                                    client_num -= 1;
//                                    break;
//                                }
//                                printf("  %d bytes received\n", rc);
//                                len = rc;
//                                rc = send(i, buffer, len, 0);
//                                if (rc < 0) {
//                                    perror("  send() failed");
//                                    close_conn = 1;
//                                    break;
//                                }
                                if (!recv(i, buffer, sizeof(buffer), 0)) {
                                    printf("A player has quit!\nAwaiting for new player to connect\n");
                                    client_num--;
                                    //return MIDGAME_QUIT;
                                } else {
                                    serv_env->game_list[0]->turn++;
                                    serv_env->game_list[0]->c = buffer[0];
                                    printf("Player %d wrote: %c\n", (i - serv_env->sfd), serv_env->game_list[0]->c);
                                    serv_env->game_list[0]->c = buffer[0];
//                                    send(i, NO_TURN, strlen(NO_TURN), 0);
//                                    send(i+1, YES_TURN, strlen(YES_TURN), 0);
                                }

                                if (client_num > 1) {
                                    for (int j = 0; j < 2; j++) {
                                        serv_env->game_list[0]->player[j] = serv_env->player_socket[j];
                                    }
                                    handle_move(serv_env->game_list[0]);
                                }
                                break;
                            }
                        } while (1);
                        // Assigning every 2 player to a TTT game
//                        int fd = serv_env->sfd + 1;
//                        for (i = 0; i < client_num / 2; i++) {
//                            TTTEnvironment *game_env;
//                            game_env = dc_malloc(sizeof(TTTEnvironment));
//                            for (int j = 0; j < 2; j++) {
//                                game_env->player[j] = serv_env->player_socket[i];
//                                fd++;
//                            }
//                            serv_env->game_list[i] = game_env;
//                        }

                        if (close_conn) {
                            close(i);
                            FD_CLR(i, &(serv_env->readfds));
                            if (i == serv_env->max_sd) {
                                while (FD_ISSET(serv_env->max_sd, &(serv_env->readfds)) == 0)
                                    serv_env->max_sd -= 1;
                            }
                        }
                    }
                }
//            if (FD_ISSET(game_env->sfd, &readfds)) {
//                if (game_env->player[i] == 0) {
//                    game_env->player[i] = dc_accept(game_env->sfd,
//                                                    (struct sockaddr *) &game_env->player_addr[game_env->client_num],
//                                                    &game_env->slen);
//                    FD_SET(game_env->player[i], &readfds); // Adding players to socket
//                    send(game_env->player[game_env->player2_turn], mess1, strlen(mess1), 0);
//                    send(game_env->player[1], mess2, strlen(mess2), 0);
//                    break;
//                }
//            }

            }
        }
    }
}

//static int handle_move(Environment *env) {
//    TTTEnvironment *game_env;
//    game_env = (TTTEnvironment *)env;
//
//
//}

/*static int awaiting_new_player(Environment *env) {
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
}*/



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

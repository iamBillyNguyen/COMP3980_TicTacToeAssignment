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

#include <errno.h>
#include "server.h"
#include "shared.h"
#include "modules/utils.h"

#define N 10
#define UID_SIZE 4

/**
 * TODO: Record other clients testing on this server
 * TODO: UDP go go
 */

static int init_server(Environment *env);
static int server_error(Environment *env);
static int client_error(Environment *env);
static int accept_serv(Environment *env);
static int bind_serv(Environment *env);
static int listen_serv(Environment *env);
static int clean_up_serv(Environment *env);

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
                    {ACCEPT,  ERROR_CLIENT,     &client_error},
                    {ACCEPT, CLEAN_UP, &clean_up_serv},
                    {ERROR_SERV, FSM_EXIT, NULL},
                    {ERROR_CLIENT, FSM_EXIT, NULL},
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
    free(env.ttt_player_socket);
    free(env.rps_player_socket);
    free(env.ttt_game_list);
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
    serv_env->ttt_index = 0;
    serv_env->rps_index = 0;
    serv_env->ttt_client_num = 0;
    serv_env->rps_client_num = 0;
    serv_env->ttt_game_list = (TTTEnvironment *)dc_malloc(sizeof(TTTEnvironment) * N);
    serv_env->rps_game_list = (RPSEnvironment *)dc_malloc(sizeof(RPSEnvironment) * N);
    serv_env->ttt_player_socket = (int *)dc_malloc(2 * N * sizeof(int));
    serv_env->rps_player_socket = (int *)dc_malloc(2 * N * sizeof(int));
    serv_env->req = dc_malloc(sizeof(uint8_t)  * 8);
    serv_env->cur_ordering = 0;
    return BIND;
}

static int bind_serv(Environment *env) {
    ServEnvironment *serv_env;
    serv_env = (ServEnvironment *) env;
    int enable = 1;

    /** TCP socket */
    serv_env->sfd = dc_socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(serv_env->sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
    }
    dc_bind(serv_env->sfd, (struct sockaddr *) &serv_env->addr, sizeof(struct sockaddr_in));

    /** UDP socket */
    serv_env->udpfd = dc_socket(AF_INET, SOCK_DGRAM, 0);
    if (setsockopt(serv_env->udpfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
    }
    dc_bind(serv_env->udpfd, (struct sockaddr *) &serv_env->addr, sizeof(struct sockaddr_in));

    return LISTEN;
}

static int listen_serv(Environment *env) {
    ServEnvironment *serv_env;
    serv_env = (ServEnvironment *) env;

    dc_listen(serv_env->sfd, N);
    printf("----- BIT SERVER ARCADE -----\n");
    printf("Waiting for Players to join ...\n");

    return ACCEPT;
}

static int accept_serv(Environment *env) {
    ServEnvironment *serv_env;
    serv_env = (ServEnvironment *) env;
    int rc, retval, desc, len, i, new_sd, close_conn;

    FD_ZERO(&(serv_env->readfds));
    FD_SET(serv_env->sfd, &(serv_env->readfds));
    FD_SET(serv_env->udpfd, &(serv_env->readfds));
    serv_env->max_sd = (serv_env->sfd >= serv_env->udpfd ? serv_env->sfd : serv_env->udpfd);

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
//            if (FD_ISSET(serv_env->sfd, &(serv_env->workingfds))) {
                memset(serv_env->req, 0, sizeof(serv_env->req));
                memset(serv_env->res_4_bytes, 0, sizeof(serv_env->res_4_bytes));
                memset(serv_env->buffer, 0, sizeof(serv_env->buffer));
                if (FD_ISSET(i, &(serv_env->workingfds))) {
                    desc -= 1;
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

                            if (serv_env->ttt_client_num % 2 == 0 && serv_env->ttt_client_num != 0) {
                                TTTEnvironment *tmp = (TTTEnvironment *) realloc(serv_env->ttt_game_list,
                                                                                 N * sizeof(TTTEnvironment));
                                int *tmp2 = (int *) realloc(serv_env->ttt_player_socket,
                                                            (NUM_PLAYER_PER_GAME * N) * sizeof(int));

                                if (!tmp || !tmp2) {
                                    perror("realloc\n");
                                    return ERROR_SERV;
                                }
                                serv_env->ttt_game_list = tmp;
                                serv_env->ttt_player_socket = tmp2;
                            }
                            if (serv_env->rps_client_num % 2 == 0 && serv_env->rps_client_num != 0) {
                                TTTEnvironment *tmp = (TTTEnvironment *) realloc(serv_env->rps_game_list,
                                                                                 N * sizeof(TTTEnvironment));
                                int *tmp2 = (int *) realloc(serv_env->rps_game_list,
                                                            (NUM_PLAYER_PER_GAME * N) * sizeof(int));

                                if (!tmp || !tmp2) {
                                    perror("realloc\n");
                                    return ERROR_SERV;
                                }
                                serv_env->rps_game_list = tmp;
                                serv_env->rps_player_socket = tmp2;
                            }
                            // ONLY USE BUFFER FOR RECV
                            recv(new_sd, serv_env->buffer, sizeof(serv_env->buffer), 0);
                            if (serv_env->buffer[REQ_TYPE] == CONFIRMATION) {
                                // only confirm game_id at the beginning
                                if (serv_env->buffer[REQ_CONTEXT] == CONFIRM_RULESET) {
                                    if (!confirm_protocol_version(serv_env->buffer[REQ_PAYLOAD])) {
                                        serv_env->res_4_bytes[MSG_TYPE] = INVALID_PAYLOAD;
                                        serv_env->res_4_bytes[CONTEXT] = INFORMATION;
                                        serv_env->res_4_bytes[PAYLOAD_LEN] = 0;
                                        return ERROR_CLIENT;
                                    }
                                    uint8_t *uid;
                                    size_t index;
                                    memset(serv_env->res_8_bytes, 0, sizeof(serv_env->res_8_bytes));
                                    switch (serv_env->buffer[REQ_PAYLOAD + 1]) {
                                        case TIC_TAC_TOE:
                                            printf("TIC_TAC_TOE\n");
                                            serv_env->res_8_bytes[MSG_TYPE] = SUCCESS;
                                            serv_env->res_8_bytes[CONTEXT] = CONFIRMATION;

                                            uid = convert_uid_to_4_bytes(new_sd);
                                            index = PAYLOAD;
                                            for (int x = 0; x < UID_SIZE; x++) {
                                                serv_env->res_8_bytes[index] = uid[x];
                                                index++;
                                            }
                                            serv_env->res_8_bytes[PAYLOAD_LEN] = UID_SIZE;
                                            send(new_sd, serv_env->res_8_bytes, sizeof(serv_env->res_8_bytes), 0);
                                            //val = convert_uid_to_1byte(uid);
                                            serv_env->ttt_player_socket[serv_env->ttt_client_num] = new_sd;
                                            serv_env->ttt_client_num++;

                                            if (serv_env->ttt_client_num % 2 == 0) // 2 players
                                                set_new_ttt_game(serv_env);
                                            printf("Response: %d %d %d %d %d %d %d %d\n", serv_env->res_8_bytes[MSG_TYPE], serv_env->res_8_bytes[CONTEXT], serv_env->res_8_bytes[PAYLOAD_LEN], serv_env->res_8_bytes[PAYLOAD], uid[0], uid[1], uid[2], uid[3]);
                                            break;
                                        case ROCK_PAPER_SCISSOR:
                                            printf("ROCK_PAPER_SCISSOR\n");
                                            serv_env->res_8_bytes[MSG_TYPE] = SUCCESS;
                                            serv_env->res_8_bytes[CONTEXT] = CONFIRMATION;

                                            uid = convert_uid_to_4_bytes(new_sd);
                                            index = PAYLOAD;
                                            for (int x = 0; x < UID_SIZE; x++) {
                                                serv_env->res_8_bytes[index] = uid[x];
                                                index++;
                                            }
                                            serv_env->res_8_bytes[PAYLOAD_LEN] = UID_SIZE;
                                            send(new_sd, serv_env->res_8_bytes, sizeof(serv_env->res_8_bytes), 0);
                                            //val = convert_uid_to_1byte(uid);
                                            serv_env->rps_player_socket[serv_env->rps_client_num] = new_sd;
                                            serv_env->rps_client_num++;

                                            if (serv_env->rps_client_num % 2 == 0) // 2 players
                                                set_new_rps_game(serv_env);
                                            printf("Response: %d %d %d %d\n", serv_env->res_8_bytes[MSG_TYPE], serv_env->res_8_bytes[CONTEXT], serv_env->res_8_bytes[PAYLOAD_LEN], serv_env->res_8_bytes[PAYLOAD]);
                                            break;
                                        default:
                                            printf("Server only supports TTT and RPS!\n");
                                            break;
                                    }

                                    if (new_sd > serv_env->max_sd)
                                        serv_env->max_sd = new_sd;

                                    break;
                                }
                            }
                        }while (new_sd != -1);
                    } else {
                        printf("  Descriptor %d is readable\n", i);
                        close_conn = 0;
                        do {
                            if (i > serv_env->sfd) {
                                // check if the connection has closed or not
                                // close connection if yes
                                memset(serv_env->res_3_bytes, 0, sizeof(serv_env->res_3_bytes));
                                if (!recv(i, serv_env->req, sizeof(serv_env->req), 0)) {
                                    bool ttt_closed = false, rps_closed = false;
                                    printf("A player has quit!\n");
                                    serv_env->res_3_bytes[MSG_TYPE] = UPDATE;
                                    serv_env->res_3_bytes[CONTEXT] = OPPONENT_DISCONNECTED;
                                    serv_env->res_3_bytes[PAYLOAD_LEN] = 0;

                                    // SENDING DISCONNECTED RESPONSE TO OTHER PLAYER
                                    // OR CHECKING IF BOTH CLIENTS ARE DISCONNECTED
                                    for (int z = 0; z < serv_env->ttt_index; z++) {
                                        for (int y = 0; y < 2; y++) {
                                            if (serv_env->ttt_game_list[z].player[y] == i) {
                                                ttt_closed = true;
                                                send(serv_env->ttt_game_list[z].player[y == 0 ? 1 : 0], serv_env->res_3_bytes, sizeof(serv_env->res_3_bytes), 0);
                                                serv_env->ttt_game_list[z].player[y] = 0;
                                                if (y == 0)
                                                    serv_env->ttt_game_list[z].player[1] = 0;
                                                else
                                                    serv_env->ttt_game_list[z].player[0] = 0;
                                                break;
                                            } else { // Both clients are closed
                                                ttt_closed = true;
                                            }
                                        }
                                    }
                                    if (ttt_closed) {
                                        close_conn = 1;
                                        serv_env->ttt_client_num--;
                                    }

                                    // SENDING DISCONNECTED RESPONSE TO OTHER PLAYER
                                    // OR CHECKING IF BOTH CLIENTS ARE DISCONNECTED
                                    for (int z = 0; z < serv_env->rps_index; z++) {
                                        for (int y = 0; y < 2; y++) {
                                            if (serv_env->rps_game_list[z].player[y] == i) { // One client disconnects
                                                rps_closed = true;
                                                send(serv_env->rps_game_list[z].player[y == 0 ? 1 : 0], serv_env->res_3_bytes, sizeof(serv_env->res_3_bytes), 0);
                                                serv_env->rps_game_list[z].player[y] = 0;
                                                if (y == 0)
                                                    serv_env->rps_game_list[z].player[1] = 0;
                                                else
                                                    serv_env->rps_game_list[z].player[0] = 0;
                                                break;
                                            } else { // Both clients disconnected
                                                rps_closed = true;
                                            }
                                        }
                                    }
                                    if (rps_closed) {
                                        close_conn = 1;
                                        serv_env->rps_client_num--;
                                    }
                                    break;
                                } else {
                                    for (int z = 0; z < serv_env->ttt_index; z++) {
                                        for (int y = 0; y < NUM_PLAYER_PER_GAME; y++) {
                                            if (serv_env->ttt_game_list[z].player[y] == i) {
                                                serv_env->ttt_game_list[z].turn++;
                                                serv_env->ttt_game_list[z].c = serv_env->req[REQ_PAYLOAD];
                                                printf("Player %d wrote: %x\n", (i - serv_env->sfd - 1),
                                                       serv_env->ttt_game_list[z].c);
                                                ttt_handle_move(&(serv_env->ttt_game_list[z]));
                                                if (serv_env->ttt_game_list[z].done) {
                                                    close_conn = 1;
                                                    serv_env->ttt_client_num--;
                                                    serv_env->ttt_game_list[z].player[y] = 0;
                                                    if (y == 0)
                                                        serv_env->ttt_game_list[z].player[1] = 0;
                                                    else
                                                        serv_env->ttt_game_list[z].player[0] = 0;
                                                }
                                                break;
                                            }
                                        }
                                    }

                                    for (int z = 0; z < serv_env->rps_index; z++) {
                                        for (int y = 0; y < NUM_PLAYER_PER_GAME; y++) {
                                            if (serv_env->rps_game_list[z].player[y] == i) {
                                                serv_env->rps_game_list[z].moves[y == 0 ? 0
                                                                                        : 1] = serv_env->req[REQ_PAYLOAD];
                                                printf("Player %d wrote: %x\n", (i - serv_env->sfd - 1),
                                                       serv_env->req[REQ_PAYLOAD]);
                                                if (serv_env->rps_game_list[z].moves[y == 0 ? 1 : 0] !=
                                                    0) // check if both players have placed their move
                                                    rps_handle_move(&(serv_env->rps_game_list[z]));
                                                if (serv_env->rps_game_list[z].done) {
                                                    close_conn = 1;
                                                    serv_env->rps_client_num--;
                                                    serv_env->rps_game_list[z].player[y] = 0;
                                                    if (y == 0)
                                                        serv_env->rps_game_list[z].player[1] = 0;
                                                    else
                                                        serv_env->rps_game_list[z].player[0] = 0;
                                                }
                                                break;
                                            }
                                        }
                                    }
                                    break;

                                }
                            }
                        } while (1);
                        if (close_conn) {
                            dc_close(i);
                            FD_CLR(i, &(serv_env->readfds));
                            if (i == serv_env->max_sd) {
                                while (FD_ISSET(serv_env->max_sd, &(serv_env->readfds)) == 0)
                                    serv_env->max_sd -= 1;
                            }
                        }
                    }
                }
            //}
            /** UDP socket is readable */
            if (FD_ISSET(serv_env->udpfd, &(serv_env->workingfds))) {
                int nbytes;
                bzero(serv_env->datagram, sizeof(serv_env->datagram));
                printf("\nUDP Server\n");
                nbytes = recvfrom(serv_env->udpfd, serv_env->datagram, sizeof(serv_env->datagram), 0,
                         (struct sockaddr *) &i, sizeof(i));
                if (nbytes < 0)
                {
                    perror ("recfrom (server)");
                    exit (EXIT_FAILURE);
                }

                if (serv_env->datagram[ORDERING] < serv_env->cur_ordering)
                    break;
                serv_env->cur_ordering = serv_env->datagram[ORDERING];
                uint8_t uid[UID_UDP];
                for (size_t x = UID_UDP; x < UID_UDP + UID_SIZE; x++)
                    uid[x - UID_SIZE] = serv_env->datagram[x];
                uint8_t val = convert_uid_to_1byte(uid);

                for (int z = 0; z < serv_env->ttt_index; z++) {
                    for (int y = 0; y < NUM_PLAYER_PER_GAME; y++) {
                        if (serv_env->ttt_game_list[z].player[y] == val) {
                            nbytes = sendto(serv_env->udpfd, serv_env->datagram, sizeof(serv_env->datagram), 0,
                                            (struct sockaddr *) &(serv_env->ttt_game_list[z].player[y == 0 ? 1 : 0]), sizeof(serv_env->ttt_game_list[z].player[y == 0 ? 1 : 0]));
                            break;
                        }
                    }
                }

                for (int z = 0; z < serv_env->rps_index; z++) {
                    for (int y = 0; y < NUM_PLAYER_PER_GAME; y++) {
                        if (serv_env->rps_game_list[z].player[y] == val) {
                            nbytes = sendto(serv_env->udpfd, serv_env->datagram, sizeof(serv_env->datagram), 0,
                                            (struct sockaddr *) &(serv_env->rps_game_list[z].player[y == 0 ? 1 : 0]), sizeof(serv_env->rps_game_list[z].player[y == 0 ? 1 : 0]));
                            break;
                        }
                    }
                }

                if (nbytes < 0)
                {
                    perror ("sendto (server)");
                    exit (EXIT_FAILURE);
                }
                break;
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

static int client_error(Environment *env) {
    ServEnvironment *serv_env;
    serv_env = (ServEnvironment *)env;

    memset(serv_env->res_3_bytes, 0, sizeof(serv_env->res_3_bytes));
    serv_env->res_3_bytes[MSG_TYPE] = INVALID_PAYLOAD;
    serv_env->res_3_bytes[CONTEXT] = INFORMATION;
    serv_env->res_3_bytes[PAYLOAD_LEN] = 0;

    return FSM_EXIT;
}

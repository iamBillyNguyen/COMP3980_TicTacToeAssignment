//
// Created by billy on 2021-04-16.
//

#ifndef COMP3980_TICTACTOEASSIGNMENT_SERVER_H
#define COMP3980_TICTACTOEASSIGNMENT_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <dc/stdlib.h>
#include <dc/stdio.h>
#include <dc/unistd.h>
#include <dc/sys/socket.h>
#include <dcfsm/fsm.h>
#include "environment/TTTGame.h"
#include "environment/RPSGame.h"
#include "shared.h"

typedef enum
{
    INIT_SERV = FSM_APP_STATE_START, // 2
    BIND,                            // 3
    LISTEN,                          // 4
    ACCEPT,                          // 5
    CLEAN_UP,                        // 6
    ERROR_SERV,                      // 7
    ERROR_CLIENT                     // 8
} States;

typedef struct {
    Environment common;
    struct sockaddr_in addr;
    int sfd, slen, max_sd, size, ttt_index, ttt_client_num, rps_index, rps_client_num, len, udpfd;
    fd_set readfds, workingfds;
    int *ttt_player_socket, *rps_player_socket, cur_ordering;
    TTTEnvironment *ttt_game_list;
    RPSEnvironment *rps_game_list;
    uint8_t msg_type, context, payload, payload_len;
    uint8_t *req, res_3_bytes[3], res_4_bytes[4], res_8_bytes[8];
    uint8_t buffer[BUF_LEN];
    uint8_t datagram[DATAGRAM_SIZE];
} ServEnvironment;

void assign_player(ServEnvironment *serv_env, uint8_t game_id);
void set_new_ttt_game(ServEnvironment *serv_env);
void set_new_rps_game(ServEnvironment *serv_env);

#endif //COMP3980_TICTACTOEASSIGNMENT_SERVER_H

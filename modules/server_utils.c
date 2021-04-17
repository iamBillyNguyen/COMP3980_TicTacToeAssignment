//
// Created by billy on 2021-04-16.
//

#include "../server.h"

void assign_player(ServEnvironment *serv_env, uint8_t game_id) {
    // Assigning 2 new players to a game
    switch (game_id) {
        case TIC_TAC_TOE:
            for (int j = (serv_env->ttt_client_num - 2), x = 0; j < serv_env->ttt_client_num, x < 2; j++, x++) {
                serv_env->ttt_game_list[serv_env->ttt_index].player[x] = serv_env->ttt_player_socket[j];
            }
            break;
        case ROCK_PAPER_SCISSOR:
            for (int j = (serv_env->rps_client_num - 2), x = 0; j < serv_env->rps_client_num, x < 2; j++, x++) {
                serv_env->rps_game_list[serv_env->rps_index].player[x] = serv_env->rps_player_socket[j];
            }
            break;
        default:
            break;
    }
}

void set_new_ttt_game(ServEnvironment *serv_env) {
    TTTEnvironment *game_env;

    game_env = dc_malloc(sizeof(TTTEnvironment));
    init_ttt_game((Environment*)game_env);
    serv_env->ttt_game_list[serv_env->ttt_index] = *game_env;

    assign_player(serv_env, TIC_TAC_TOE);
    for (int i = 0; i < NUM_PLAYER_PER_GAME; i++) {
        memset(serv_env->res_4_bytes, 0, sizeof(serv_env->res_4_bytes));
        serv_env->res_4_bytes[MSG_TYPE] = UPDATE;
        serv_env->res_4_bytes[CONTEXT] = START_GAME;
        serv_env->res_4_bytes[PAYLOAD_LEN] = 1;
        serv_env->res_4_bytes[PAYLOAD] = (i == 0) ? X : O;
        send(serv_env->ttt_game_list[serv_env->ttt_index].player[i], serv_env->res_4_bytes, sizeof(serv_env->res_4_bytes), 0);
    }
    printf("Response: %d %d %d %d\n", serv_env->res_4_bytes[MSG_TYPE], serv_env->res_4_bytes[CONTEXT], serv_env->res_4_bytes[PAYLOAD_LEN], serv_env->res_4_bytes[PAYLOAD]);
    serv_env->ttt_index++;
}

void set_new_rps_game(ServEnvironment *serv_env) {
    RPSEnvironment *game_env;

    game_env = dc_malloc(sizeof(RPSEnvironment));
    init_rps_game((Environment*)game_env);
    serv_env->rps_game_list[serv_env->rps_index] = *game_env;

    assign_player(serv_env, ROCK_PAPER_SCISSOR);
    for (int i = 0; i < NUM_PLAYER_PER_GAME; i++) {
        memset(serv_env->res_3_bytes, 0, sizeof(serv_env->res_3_bytes));
        serv_env->res_3_bytes[MSG_TYPE] = UPDATE;
        serv_env->res_3_bytes[CONTEXT] = START_GAME;
        serv_env->res_3_bytes[PAYLOAD_LEN] = 0;
        send(serv_env->rps_game_list[serv_env->rps_index].player[i], serv_env->res_3_bytes, sizeof(serv_env->res_3_bytes), 0);
    }
    printf("Response: %d %d %d\n", serv_env->res_3_bytes[MSG_TYPE], serv_env->res_3_bytes[CONTEXT], serv_env->res_3_bytes[PAYLOAD_LEN]);
    serv_env->rps_index++;
}

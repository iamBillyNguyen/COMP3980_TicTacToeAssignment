#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "RPSGame.h"
#include "shared.h"

typedef enum
{
    VALIDATE_RPS = FSM_APP_STATE_START,  // 2
    RPS_ERROR,                           // 3
    MIDGAME_QUIT_RPS,                    // 4
} RPSGameStates;

bool rps_handle_move(RPSEnvironment *env) {
    StateTransition transitions[] =
            {
                    {FSM_INIT, VALIDATE_RPS, &rps_validate},
                    {VALIDATE_RPS, RPS_ERROR, &rps_error},
                    {VALIDATE_RPS, FSM_EXIT, NULL},
                    {RPS_ERROR, FSM_EXIT, NULL},
                    {FSM_IGNORE, FSM_IGNORE, NULL}
            };

    /** INIT FSM */
    int code;
    int start_state;
    int end_state;

    start_state = FSM_INIT; // 0
    end_state = VALIDATE;       // 2
    code = fsm_run((Environment *)env, &start_state, &end_state, transitions);

    if (code != 0)
    {
        fprintf(stderr, "Cannot move from %d to %d\n", start_state, end_state);

        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void init_rps_game(Environment *env) {
    RPSEnvironment *game_env;
    game_env = (RPSEnvironment *)env;

    game_env->client_num = 2;
    game_env->done = false;
    game_env->res = (uint8_t*) dc_malloc(sizeof(uint8_t) * 4);

    bzero(game_env->moves, sizeof(game_env->moves));
}

static int rps_validate(Environment *env)
{
    RPSEnvironment *game_env;
    game_env = (RPSEnvironment *)env;
    for (int i = 0; i < NUM_PLAYER_PER_GAME; i++) {
        if (game_env->moves[i] < 0 || game_env->moves[i] > 3) {
            game_env->player_turn = i;
            return RPS_ERROR;
        }
    }
    uint8_t key = rps_check(game_env->moves);
    printf("KEY %x\n", key);
    if (key == 0) {
        printf("----- Player 1 won! -----\n");
        game_env->done = true;
        for (int i = 0; i < NUM_PLAYER_PER_GAME;i++) {
            game_env->res[MSG_TYPE] = UPDATE;
            game_env->res[CONTEXT] = END_GAME;
            game_env->res[PAYLOAD_LEN] = 1;
            game_env->res[PAYLOAD] = (i == 0) ? WIN : LOSS;
            if (game_env->res[PAYLOAD] == WIN) // sending the client's win move
                game_env->res[PAYLOAD + 1] = game_env->moves[0];
            if (game_env->res[PAYLOAD] == LOSS) // sending the opponent's win move
                game_env->res[PAYLOAD + 1] = game_env->moves[1];
            send(game_env->player[i], game_env->res, sizeof(game_env->res), 0);
        }
        return FSM_EXIT;
    } else if (key == 1) {
        printf("----- Player 2 won! -----\n");
        game_env->done = true;
        for (int i = 0; i < NUM_PLAYER_PER_GAME;i++) {
            game_env->res[MSG_TYPE] = UPDATE;
            game_env->res[CONTEXT] = END_GAME;
            game_env->res[PAYLOAD_LEN] = 1;
            game_env->res[PAYLOAD] = (i == 1) ? WIN : LOSS;
            if (game_env->res[PAYLOAD] == WIN) // sending the client's win move
                game_env->res[PAYLOAD + 1] = game_env->moves[1];
            if (game_env->res[PAYLOAD] == LOSS) // sending the opponent's win move
                game_env->res[PAYLOAD + 1] = game_env->moves[0];
            send(game_env->player[i], game_env->res, sizeof(game_env->res), 0);
        }
        return FSM_EXIT;
    } else if (key == ' ')
    {
        printf("----- TIE -----\n");
        game_env->done = true;
        for (int i = 0; i < NUM_PLAYER_PER_GAME;i++) {
            game_env->res[MSG_TYPE] = UPDATE;
            game_env->res[CONTEXT] = END_GAME;
            game_env->res[PAYLOAD_LEN] = 1;
            game_env->res[PAYLOAD] = TIE;
            send(game_env->player[i], game_env->res, sizeof(game_env->res), 0);
        }
        return FSM_EXIT;
    }

    /** SUCCESS RESPONSE BACK TO CLIENTS */
    game_env->res[MSG_TYPE] = UPDATE;
    game_env->res[CONTEXT] = MOVE_MADE;
    game_env->res[PAYLOAD_LEN] = 0;
    for (int i = 0; i < NUM_PLAYER_PER_GAME; i++)
        send(game_env->player[i], game_env->res, sizeof(game_env->res), 0);

    return FSM_EXIT;
}

/** CHECK FOR WIN, LOSE, OR TIE */
uint8_t rps_check(uint8_t moves[2])
{
    uint8_t key = ' ';
    for (int i = 0; i < NUM_PLAYER_PER_GAME; i++)
        if ((moves[i] == ROCK && moves[i == 0 ? 1 : 0] == SCISSORS) || (moves[i] == PAPER && moves[i == 0 ? 1 : 0] == ROCK) || (moves[i] == SCISSORS && moves[i == 0 ? 1 : 0] == PAPER))
            key = (uint8_t) i;

    return key;
}

static int rps_error(Environment *env)
{
    RPSEnvironment *game_env;
    game_env = (TTTEnvironment *)env;

    printf("Invalid move player %d, place again!\n", game_env->player_turn + 1);
    game_env->res[MSG_TYPE] = INVALID_ACTION;
    game_env->res[CONTEXT] = GAME_ACTION;
    game_env->res[PAYLOAD_LEN] = 0;
    game_env->res[PAYLOAD] = ' ';
    send(game_env->player[game_env->player_turn], game_env->res, sizeof(game_env->res), 0);
//    send(game_env->player[game_env->player2_turn], INVALID_MOVE, strlen(INVALID_MOVE), 0);
//    send(game_env->player[!game_env->player2_turn], WAIT, strlen(WAIT), 0);

    return FSM_EXIT;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "TTTGame.h"
#include "../shared.h"

typedef enum
{
    VALIDATE = FSM_APP_STATE_START,  // 2
    ERROR,                           // 3
    MIDGAME_QUIT,                    // 4
} GameStates;

bool ttt_handle_move(TTTEnvironment *env) {
    StateTransition transitions[] =
            {
                    {FSM_INIT, VALIDATE, &ttt_validate},
                    {VALIDATE, ERROR, &ttt_error},
                    {VALIDATE, FSM_EXIT, NULL},
                    {ERROR, FSM_EXIT, NULL},
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

void init_ttt_game(Environment *env) {
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;

    game_env->client_num = 2;
    game_env->turn = 0;
    game_env->player2_turn = false;
    game_env->done = false;
    game_env->player_c = 'X';

    for (int i = 0; i < 9; i++) {
        game_env->playBoard[i] = ' ';
        game_env->occupy[i] = false;
    }
}

static int ttt_validate(Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;
    if (game_env->c == EOF || game_env->c == '-') { // If player quits midway
        printf("Player quits midgame\n");
        return MIDGAME_QUIT;
    }
    if (game_env->c < 0 || game_env->c > 8)
    {
        game_env->turn--;
        return ERROR;
    }
    int index = game_env->c;
    if (game_env->occupy[index])
    {
        game_env->turn--;
        return ERROR;
    }

    if (game_env->player2_turn)
    {
        game_env->player_c = 'O';
    }
    else
    {
        game_env->player_c = 'X';
    }

    update_ttt_board(game_env->c, game_env->playBoard, game_env->player_c, env);

    char key = ttt_check(game_env->playBoard);
    memset(game_env->res_4_bytes, 0, sizeof(game_env->res_4_bytes));
    if (key == 'X') {
        printf("----- Player 1 won! -----\n");
        game_env->done = true;
        for (int i = 0; i < NUM_PLAYER_PER_GAME;i++) {
            game_env->res_3_bytes[MSG_TYPE] = UPDATE;
            game_env->res_4_bytes[CONTEXT] = END_GAME;
            game_env->res_4_bytes[PAYLOAD_LEN] = 1;
            game_env->res_4_bytes[PAYLOAD] = (i == 0) ? WIN : LOSS;
            send(game_env->player[i], game_env->res_4_bytes, sizeof(game_env->res_4_bytes), 0);
        }
        return FSM_EXIT;
    } else if (key == 'O') {
        printf("----- Player 2 won! -----\n");
        game_env->done = true;
        for (int i = 0; i < NUM_PLAYER_PER_GAME;i++) {
            game_env->res_4_bytes[MSG_TYPE] = UPDATE;
            game_env->res_4_bytes[CONTEXT] = END_GAME;
            game_env->res_4_bytes[PAYLOAD_LEN] = 1;
            game_env->res_4_bytes[PAYLOAD] = (i == 1) ? WIN : LOSS;
            send(game_env->player[i], game_env->res_4_bytes, sizeof(game_env->res_4_bytes), 0);
        }
        return FSM_EXIT;
    }

    if (game_env->turn == TOTAL_TURNS)
    {
        printf("----- TIE -----\n");
        game_env->done = true;
        for (int i = 0; i < NUM_PLAYER_PER_GAME;i++) {
            game_env->res_4_bytes[MSG_TYPE] = UPDATE;
            game_env->res_4_bytes[CONTEXT] = END_GAME;
            game_env->res_4_bytes[PAYLOAD_LEN] = 1;
            game_env->res_4_bytes[PAYLOAD] = TIE;
            send(game_env->player[i], game_env->res_4_bytes, sizeof(game_env->res_4_bytes), 0);
        }
        return FSM_EXIT;
    }
    /** SUCCESS RESPONSE BACK TO CLIENTS */
    memset(game_env->res_3_bytes, 0, sizeof(game_env->res_3_bytes));
    game_env->res_3_bytes[MSG_TYPE] = SUCCESS;
    game_env->res_3_bytes[CONTEXT] = GAME_ACTION;
    game_env->res_3_bytes[PAYLOAD_LEN] = 0;
    send(game_env->player[game_env->player2_turn], game_env->res_3_bytes, sizeof(game_env->res_3_bytes), 0);


    /** SEND POSITION/CHOICE TO CLIENTS */
    memset(game_env->res_4_bytes, 0, sizeof(game_env->res_4_bytes));
    game_env->res_4_bytes[MSG_TYPE] = UPDATE;
    game_env->res_4_bytes[CONTEXT] = MOVE_MADE;
    game_env->res_4_bytes[PAYLOAD_LEN] = 1;
    game_env->res_4_bytes[PAYLOAD] = game_env->c;
    game_env->player2_turn = !game_env->player2_turn;
    send(game_env->player[game_env->player2_turn ? 1 : 0], game_env->res_4_bytes, sizeof(game_env->res_4_bytes), 0);

    return FSM_EXIT;
}

static void update_ttt_board(uint8_t c, char board[9], char player, Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;

    if (board[c] == ' ') {
        board[c] = player;
        game_env->occupy[c] = true;
    }

    printf("    %c  | %c  | %c\n", board[0], board[1], board[2]);
    printf("    --------------\n");
    printf("    %c  | %c  | %c\n", board[3], board[4], board[5]);
    printf("    --------------\n");
    printf("    %c  | %c  | %c\n", board[6], board[7], board[8]);
}

/** CHECK FOR WIN, LOSS, OR TIE */
char ttt_check(const char playBoard[9])
{
    int i;
    char key = ' ';

    // Check Rows
    for (i = 0; i < 9; i+=3) {
        if (playBoard[i] == playBoard[i + 1] && playBoard[i] == playBoard[i + 2] && playBoard[i] != ' ') {
            key = playBoard[i];
            break;
        }
    }
    // check Columns
    for (i = 0; i < 3; i++) {
        if (playBoard[i] == playBoard[i + 3] && playBoard[i] == playBoard[i + 6] && playBoard[i] != ' ') {
            key = playBoard[i];
            break;
        }
    }
    // Check Diagonals
    if (playBoard[0] == playBoard[4] && playBoard[0] == playBoard[8] && playBoard[0] != ' ')
        key = playBoard[0];
    if (playBoard[2] == playBoard[4] && playBoard[2] == playBoard[6] && playBoard[0] != ' ')
        key = playBoard[2];

    return key;
}

static int ttt_error(Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;

    printf("Invalid move, place again!\n");
    memset(game_env->res_3_bytes, 0, sizeof(game_env->res_3_bytes));
    game_env->res_3_bytes[MSG_TYPE] = INVALID_ACTION;
    game_env->res_3_bytes[CONTEXT] = GAME_ACTION;
    game_env->res_3_bytes[PAYLOAD_LEN] = 0;
    send(game_env->player[game_env->player2_turn], game_env->res_3_bytes, sizeof(game_env->res_3_bytes), 0);

    return FSM_EXIT;
}

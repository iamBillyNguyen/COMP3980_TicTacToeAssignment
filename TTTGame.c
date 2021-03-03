#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "TTTGame.h"
#include "shared.h"

typedef enum
{
    VALIDATE = FSM_APP_STATE_START,  // 2
    ERROR,                           // 3
    MIDGAME_QUIT,                    // 4
} GameStates;

bool handle_move(TTTEnvironment *env) {
    StateTransition transitions[] =
            {
                    {FSM_INIT, VALIDATE, &validate},
                    {VALIDATE, ERROR, &error},
                    {VALIDATE, FSM_EXIT, NULL},
                    {ERROR, FSM_EXIT, NULL},
                    //{VALIDATE, INIT_SERV, &init_server},
                    //{VALIDATE, MIDGAME_QUIT, &awaiting_new_player},
                    //{READ, MIDGAME_QUIT, &awaiting_new_player},
                    //{MIDGAME_QUIT, READ, &read_input},
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

void init_game(Environment *env) {
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;

    game_env->client_num = 2;
    game_env->turn = 0;
    game_env->player2_turn = false;
    game_env->player_c = 'X';

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i == 0) {
                game_env->playBoard[i][j] = (char) ('A' + j);
            } else if (i == 1) {
                game_env->playBoard[i][j] = (char) ('D' + j);
            } else {
                game_env->playBoard[i][j] = (char) ('G' + j);
            }
            game_env->occupy[i][j] = false;
        }
    }
}

static int validate(Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;
    if (game_env->c == 0 || game_env->c == EOF || game_env->c == '-') { // If player quits midway
        printf("Player quits midgame\n");
        // Accept a new player
        //game_env->player[game_env->player2_turn] = dc_accept(game_env->sfd, (struct sockaddr *)&game_env->player_addr[game_env->player2_turn], &(game_env->slen));
        return MIDGAME_QUIT;
    }
    if (game_env->c < 'A' || game_env->c > 'I')
    {
        game_env->turn--;
        return ERROR;
    }
    int index = (int)(game_env->c) - 65;
    if (index < 3)
    {
        if (game_env->occupy[0][index])
        {
            game_env->turn--;
            return ERROR;
        }
    }
    else if (index > 2 && index < 6)
    {
        if (game_env->occupy[1][index-3])
        {
            game_env->turn--;
            return ERROR;
        }
    }
    else if (index > 5)
    {
        if (game_env->occupy[2][index-6])
        {
            game_env->turn--;
            return ERROR;
        }
    }
    if (game_env->player2_turn)
    {
        game_env->player_c = 'O';
    }
    else
    {
        game_env->player_c = 'X';
    }

    update_board(game_env->c, game_env->playBoard, game_env->player_c, env);
    /** SEND POSITION/CHOICE TO CLIENTS */
    char choice[1];
    choice[0] = game_env->c;
    send(game_env->player[0], choice, sizeof(choice), 0);
    send(game_env->player[1], choice, sizeof(choice), 0);

    char key = check(game_env->playBoard);
    if (key == 'X') {
        send(game_env->player[1], LOSE, strlen(LOSE), 0);
        send(game_env->player[0], WIN, strlen(WIN), 0);
        printf("----- Player 1 won! -----\n");
        check_user_choice(env);
        return FSM_EXIT;
    } else if (key == 'O') {
        send(game_env->player[1], WIN, strlen(WIN), 0);
        send(game_env->player[0], LOSE, strlen(LOSE), 0);
        printf("----- Player 2 won! -----\n");
        check_user_choice(env);
        return FSM_EXIT;
    }

    if (game_env->turn == TOTAL_TURNS)
    {
        printf("----- GAME TIES -----\n");
        send(game_env->player[1], TIE, strlen(TIE), 0);
        send(game_env->player[0], TIE, strlen(TIE), 0);
        check_user_choice(env);
        return FSM_EXIT;
    }

    // SWITCH TURN AFTER DONE VALIDATION
    if (game_env->player2_turn)
    {
        send(game_env->player[0], YES_TURN, strlen(YES_TURN), 0);
        send(game_env->player[1], NO_TURN, strlen(NO_TURN), 0);
    }
    else
    {
        send(game_env->player[1], YES_TURN, strlen(YES_TURN), 0);
        send(game_env->player[0], NO_TURN, strlen(NO_TURN), 0);

    }
    game_env->player2_turn = !game_env->player2_turn;
    return FSM_EXIT;
}
static void update_board(char c, char board[][3], char player, Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (c == board[i][j])
            {
                board[i][j] = player;
                game_env->occupy[i][j] = true;
                break;
            }
        }
    }

    printf("    %c  %c  %c\n", board[0][0], board[0][1], board[0][2]);
    printf("    %c  %c  %c\n", board[1][0], board[1][1], board[1][2]);
    printf("    %c  %c  %c\n", board[2][0], board[2][1], board[2][2]);
}

/** CHECK FOR WIN, LOSE, OR TIE */
char check(char playBoard[][3])
{
    int i;
    char key = ' ';

    // Check Rows
    for (i = 0; i < 3; i++)
        if (playBoard[i][0] == playBoard[i][1] && playBoard[i][0] == playBoard[i][2])
            key = playBoard[i][0];
    // check Columns
    for (i = 0; i < 3; i++)
        if (playBoard[0][i] == playBoard[1][i] && playBoard[0][i] == playBoard[2][i])
            key = playBoard[0][i];
    // Check Diagonals
    if (playBoard[0][0] == playBoard[1][1] && playBoard[1][1] == playBoard[2][2])
        key = playBoard[1][1];
    if (playBoard[0][2] == playBoard[1][1] && playBoard[1][1] == playBoard[2][0])
        key = playBoard[1][1];

    return key;
}

static int error(Environment *env)
{
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;

    printf("Invalid move player %d, place again!\n", game_env->player2_turn ? 2 : 1);
    send(game_env->player[game_env->player2_turn], INVALID_MOVE, strlen(INVALID_MOVE), 0);
    send(game_env->player[!game_env->player2_turn], WAIT, strlen(WAIT), 0);

    return FSM_EXIT;
}

static void check_user_choice(Environment *env) {
    TTTEnvironment *game_env;
    game_env = (TTTEnvironment *)env;
    int player_num = 2;

    if (game_env->c != 'r') {
        memset(&(game_env->player[0]), 0, sizeof(game_env->player[0]));
        player_num--;
    }
    if (game_env->c != 'r') {
        memset(&(game_env->player[1]), 0, sizeof(game_env->player[1]));
        player_num--;
    }
    if (player_num == 1) {
        game_env->client_num--;
    } else if (player_num == 0){
        game_env->client_num = 0;
    }
}




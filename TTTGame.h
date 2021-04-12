#ifndef TTTGAME_H
#define TTTGAME_H

#include <dcfsm/fsm.h>

#define TOTAL_TURNS 9
#define NUM_PLAYER_PER_GAME 2

typedef struct
{
    Environment common;
    uint8_t c, player_c;
    bool player2_turn, done;
    int player[NUM_PLAYER_PER_GAME];
    int client_num, turn;
    char playBoard[9];
    bool occupy[9];
    uint8_t* res;
} TTTEnvironment;

static int ttt_validate(Environment *game_env);
static int ttt_error(Environment *env);
static void update_ttt_board(uint8_t c, char playBoard[9], char player, Environment *env);
char ttt_check(char playBoard[9]);
static void check_user_choice(Environment *env);
bool ttt_handle_move(TTTEnvironment *env);
void init_ttt_game(Environment *env);

#endif // TTTGAME_H

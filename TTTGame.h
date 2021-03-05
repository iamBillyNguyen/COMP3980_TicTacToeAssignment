#ifndef TTTGAME_H
#define TTTGAME_H

#include <dcfsm/fsm.h>

#define TOTAL_TURNS 9
#define NUM_PLAYER_PER_GAME 2

typedef struct
{
    Environment common;
    char c, player_c;
    bool player2_turn, started;
    int player[NUM_PLAYER_PER_GAME];
    int client_num, turn;
    char playBoard[3][3];
    bool occupy[3][3];
} TTTEnvironment;

static int validate(Environment *game_env);
static int error(Environment *env);
static void update_board(char c, char playBoard[][3], char player, Environment *env);
char check(char playBoard[][3]);
static void check_user_choice(Environment *env);
bool handle_move(TTTEnvironment *env);
void init_game(Environment *env);

#endif // TTTGAME_H

#ifndef RPSGAME_H
#define RPSGAME_H

#include <dcfsm/fsm.h>

#define TOTAL_TURNS 9
#define NUM_PLAYER_PER_GAME 2

typedef struct
{
    Environment common;
    uint8_t c, player_c, moves[NUM_PLAYER_PER_GAME];
    bool done;
    int player[NUM_PLAYER_PER_GAME];
    int client_num;
    uint8_t* res;
} RPSEnvironment;

static int rps_validate(Environment *env);
static int rps_error(Environment *env);
uint8_t rps_check(uint8_t moves[2]);
static void check_user_choice(Environment *env);
bool rps_handle_move(RPSEnvironment *env);
void init_rps_game(Environment *env);

#endif // RPSGAME_H

#ifndef TTTGAME_H
#define TTTGAME_H

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
    uint8_t res_3_bytes[3], res_4_bytes[4], res_8_bytes[8];
} TTTEnvironment;

static int ttt_validate(Environment *game_env);
static int ttt_error(Environment *env);
static void update_ttt_board(uint8_t c, char board[9], char player, Environment *env);
char ttt_check(const char playBoard[9]);
static void check_user_choice(Environment *env);
bool ttt_handle_move(TTTEnvironment *env);
void init_ttt_game(Environment *env);

#endif // TTTGAME_H

//
// Created by billy on 2021-04-07.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "utils.h"
#include "../shared.h"

bool confirm_protocol_version(uint8_t version) {
    if (version == V1) return true;
    return false;
}

uint8_t confirm_game_id(uint8_t game_id) {
    switch (game_id) {
        case TIC_TAC_TOE:
            return TIC_TAC_TOE;
        case ROCK_PAPER_SCISSOR:
            return ROCK_PAPER_SCISSOR;
        default:
            return INVALID_PAYLOAD;
    }
}

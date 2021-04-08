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

uint8_t* convert_uid_to_4_bytes(uint8_t val) {
    uint8_t* bytes;
    bytes = dc_malloc(sizeof(uint8_t)*4);

    bytes[0] = (val >> 24) & 0xFF;
    bytes[1] = (val >> 16) & 0xFF;
    bytes[2] = (val >> 8) & 0xFF;
    bytes[3] = val & 0xFF;
    printf("4-byte uid: %x%x%x%x\n", bytes[0], bytes[1], bytes[2], bytes[3]);
    return bytes;
}

uint8_t convert_uid_to_1byte(uint8_t* bytes) {
    uint8_t val;
    val = (bytes[0] <<24) + (bytes[1] <<16) + (bytes[2] <<8) + bytes[3];
    printf("1-byte uid: %x\n", val);
    return val;
}

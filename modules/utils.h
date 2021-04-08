//
// Created by billy on 2021-04-07.
//

#ifndef COMP3980_ASSIGNMENT1_UTILS_H
#define COMP3980_ASSIGNMENT1_UTILS_H

#include <stdint.h>

bool confirm_protocol_version(uint8_t version);
uint8_t* convert_uid_to_4_bytes(uint8_t val);
uint8_t convert_uid_to_1byte(uint8_t* bytes);

#endif //COMP3980_ASSIGNMENT1_UTILS_H

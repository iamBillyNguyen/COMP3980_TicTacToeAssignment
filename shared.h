#ifndef SHARED_H
#define SHARED_H


#define BUF_SIZE 100

// https://www.sciencedirect.com/topics/computer-science/registered-port#:~:text=Ports%200%20through%201023%20are,be%20used%20dynamically%20by%20applications.
// /etc/services
#define PORT 8000

typedef enum
{
    INVALID_MOVE,   // 0
    VALID_MOVE,     // 1
    P1_WIN,         // 2
    P2_WIN,         // 3
    TIE,            // 4
    P1_TURN,        // 5
    P2_TURN,        // 6

} response_code;

#endif

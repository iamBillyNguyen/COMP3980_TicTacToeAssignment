#ifndef SHARED_H
#define SHARED_H


#define BUF_SIZE 100

// https://www.sciencedirect.com/topics/computer-science/registered-port#:~:text=Ports%200%20through%201023%20are,be%20used%20dynamically%20by%20applications.
// /etc/services
#define PORT 8080

#define MSG_TYPE 0
#define CONTEXT 1
#define PAYLOAD_LEN 2
#define PAYLOAD 3

/**
 * Response Message Types
 */
#define SUCCESS 10
#define UPDATE 20

/** Client error */
#define INVALID_TYPE 30
#define INVALID_CONTEXT 31
#define INVALID_PAYLOAD 32

/** Server error */
#define SERVER_ERROR 40

/** Game error */
#define INVALID_ACTION 50
#define OUT_OF_TURN 51

/**
 * Context
 */
#define START_GAME 1            // MsgType::= UPDATE, Payload::= Team
#define MOVE_MADE 2             // MsgType::= UPDATE, Payload::= Position of move
#define END_GAME 3              // MsgType::= UPDATE, Payload::= End game status
#define CONFIRMATION 1          // MsgType::= SUCCESS, Payload::= Player id

/**
 * Payload Type
 */

 /** PROTOCOL VERSION */
#define V1 1

 /** GAME ID */
#define TIC_TAC_TOE 1           // PayloadType::= PROTOCOL_VERSION
#define ROCK_PAPER_SCISSOR 2    // PayloadType::= PROTOCOL_VERSION

// 2. SUCCESS CONFIRMATION -> assign players to game
/** TEAM ID */
#define X 1
#define O 2

/** END OF GAME STATUS */
#define WIN 1
#define LOSS 2
#define TIE 3

/** MOVE FOR RPS  */
#define ROCK 1
#define PAPER 2
#define SCISSORS 3

#endif

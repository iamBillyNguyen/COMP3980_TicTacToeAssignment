#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <dc/sys/socket.h>
#include <dc/unistd.h>
#include <dc/stdlib.h>
#include <dc/netdb.h>
#include <netdb.h>
#include "shared.h"

static void update_board(char c, char playBoard[][3], char player);

int main(int argc , char *argv[])
{
    struct hostent *hostinfo;
    struct sockaddr_in addr;
    int fd, row,column,choice;;
    ssize_t num_read;
    char mess[BUF_SIZE], buf[BUF_SIZE];
    char client_reply[BUF_SIZE];
    char playBoard [3][3] =   {							// to display the actual game status
            {'A','B','C'},
            {'D','E','F'},
            {'G','H','I'}
    };
    char *win = "----- You won! -----\n";
    char *lose = "----- You lose! -----\n";
    int count = 0;
    char c;

    hostinfo = dc_gethostbyname("127.0.0.1");
    fd = dc_socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr = *(struct in_addr *) hostinfo->h_addr;
    dc_connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

    printf("WELCOME TO BIT SERVER'S TIC TAC TOE!\n");

    /*dc_read(fd, mess, strlen(mess));
    printf("%s", mess);
    dc_read(fd, mess, strlen(mess));
    printf("%s", mess);
    while(1) {
        // receive server's response
        if (recv(fd, mess, strlen(mess), 0) < 0) {
            puts("recv() failed\n");
            break;
        }
        puts(mess);
        printf("1\n");
        scanf("%s" , client_reply);
        // send answer
        if (send(fd, client_reply, strlen(client_reply), 0) < 0) {
            puts("send() failed\n");
            break;
        }

        if (strcmp(mess, win)) {
            printf("%s", mess);
            exit(EXIT_SUCCESS);
        } else if (strcmp(mess, lose)) {
            printf("%s", mess);
            exit(EXIT_SUCCESS);
        }
    }*/

    dc_read(fd, buf, sizeof(buf));
    printf("%s",buf);
    dc_read(fd, buf, sizeof(buf));
    printf("%s",buf);

    while (1) {
        while ((num_read = dc_read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
            count++;
            send(fd, buf, num_read, 0); // send answer
            //recv(fd, code, sizeof(code), 0); // to get server's response code
            //int code_num = atoi(code[0]);
            //c = buf[0];
            if (count == 2) {
                count = 0;
                break;
            }
        }
        recv(fd, mess, sizeof(mess), 0);
    }

    // system("clear");

    dc_close(fd);

    return EXIT_SUCCESS;
}



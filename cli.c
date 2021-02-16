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


int main(int argc , char *argv[])
{
    struct hostent *hostinfo;
    struct sockaddr_in addr;
    int fd, row,column,choice;;
    ssize_t num_read;
    char buf[BUF_SIZE];
    char pid[4];
    char playBoard [3][3] =   {							// to display the actual game status
            {' ',' ',' '},
            {' ',' ',' '},
            {' ',' ',' '}
    };

    hostinfo = dc_gethostbyname("127.0.0.1");
    fd = dc_socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr = *(struct in_addr *) hostinfo->h_addr;
    dc_connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

    printf("WELCOME TO BIT SERVER'S TIC TAC TOE!\n");

    dc_read(fd, buf, sizeof(buf));
    printf("%s",buf);
    dc_read(fd, buf, sizeof(buf));
    printf("%s",buf);

    while((num_read = dc_read(STDIN_FILENO, buf, BUF_SIZE)) > 0)
    {
        dc_write(fd, buf, num_read);
    }

    // system("clear");

    dc_close(fd);

    return EXIT_SUCCESS;
}
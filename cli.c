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
    char a[2][40];
    char pid[4];
    char clientWrite[1];
    char playBoard [3][3] =   {							// to display the actual game status
            {' ',' ',' '},
            {' ',' ',' '},
            {' ',' ',' '}
    };

    hostinfo = dc_gethostbyname("23.16.22.78");
    fd = dc_socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr = *(struct in_addr *) hostinfo->h_addr;
    dc_connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

    dc_read(fd, a, sizeof(a));
    printf("%s\n",a[0]);

    if(strcmp(a[1],"0")==0)
    {
        int num1 = getpid();
        sprintf(pid,"%d",num1);
        dc_write(fd, pid, sizeof(pid));
        dc_read(fd,a,sizeof(a));
        printf("%s\n",a[0]);
        printf("WELCOME TO BIT SERVER'S TIC TAC TOE!\n");
    }

    if(strcmp(a[1],"2")==0)
    {
        int num2 = getpid();
        sprintf(pid,"%d",num2);
        write(fd, pid, sizeof(pid));
    }

    if (strcmp(a[1],"1")!=0) {
        for (;;) {
            printf("\nPlayer %d,Please enter the number of the square where you want to place your '%c': \n",
                   (strcmp(a[1], "1") == 0) ? 1 : 2, (strcmp(a[1], "1") == 0) ? 'X' : 'O');
            scanf("%s", clientWrite);
            choice = atoi(clientWrite);
            row = --choice/3;
            column = choice%3;
            if(choice<0 || choice>9 || playBoard [row][column]>'9'|| playBoard [row][column]=='X' || playBoard [row][column]=='O')
            printf("Invalid Input. Please Enter again.\n\n");

            else
            {
                playBoard[row][column] = (strcmp(a[1], "1")==0)?'X':'O';
                break;
            }
        }
        write(fd, clientWrite, sizeof(clientWrite));
        system("clear");
    }

    dc_close(fd);

    return EXIT_SUCCESS;
}
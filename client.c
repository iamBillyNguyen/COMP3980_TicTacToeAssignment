#include <dc/stdio.h>
#include <dc/unistd.h>
#include <dc/sys/socket.h>
#include <dc/netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "shared.h"
#include <netdb.h>


int main(int argc, char *argv[])
{
    struct hostent *hostinfo;
    struct sockaddr_in addr;
    int fd;
    ssize_t num_read;
    char buf[BUF_SIZE];
    
    hostinfo = dc_gethostbyname("127.0.0.1");
    fd = dc_socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr = *(struct in_addr *) hostinfo->h_addr;
    dc_connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    
    while((num_read = dc_read(STDIN_FILENO, buf, BUF_SIZE)) > 0)
    {
        dc_write(fd, buf, num_read);
    }
    
    dc_close(fd);

    return EXIT_SUCCESS;
}

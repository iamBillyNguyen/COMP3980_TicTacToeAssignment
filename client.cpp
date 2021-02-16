/* This is the client side code to connect to a tic-tac-toe game server.
 * Source Code written by Indradhanush Gupta.
 * Website  :  
 * Github
 * Facebook :
 * Twitter  :
 * Quora    : 
 * E-mail   : indradhanush.gupta@gmail.com
 * 
 * To report any bugs please send me an e-mail.
 * Tips are welcome. 
 *
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "tictac.h"
#include "shared.h"
#define IPADD 3232235940

// #define SERV_HOST_ADDR "23.16.22.78"
#define SERV_HOST_ADDR "127.0.0.1"
using namespace std;

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, port_no, n, connectfd, bytes_sent, bytes_recvd;
    char cbuffer[512], sname[64], cname[64];
    char *ptr = &cbuffer[0];
    struct sockaddr_in serv_addr;
    struct hostent *he;
    char x;
    int count = 0, inp, y, ni, inp_true = 0, toss;
    char serv_choice, cli_choice, nc;
    char choice_buffer[2], co_ordinates_buffer[2], server_buffer[BUFSIZ], choice[2];
    char playBoard[3][3] = {{'A','B','C'},
                        {'D','E','F'},
                        {'G','H','I'}};

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("Sorry. Socket could not be created!");
        return 1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // serv_addr.sin_addr = *((struct in_addr *)he->h_addr);
    serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
    connectfd = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (connectfd == -1)
    {
        perror("Sorry. Could not connect to server.");

        //THIS THIING ENDS CLIENT!
        //return 1;
    }

    cout << "Creating game. Please wait..." << endl;
    // sleep(2);
    cout << endl
         << "Game created!" << endl
         << endl
         << "Doing a toss...";

     //bytes_recvd = recv(sockfd, &toss_buffer, sizeof(toss_buffer), 0);

     if (bytes_recvd == -1)
     {
     	perror("TOSS BUFFER not received");
     	return 1;
     }

     //toss = toss_buffer - '0';
     cout << "toss: " << toss << endl;

    // init();
    cout << endl
         << "Starting Game..." << endl;
    // sleep(3);
    display();

    while (count < 9)
    {
        if (strcmp(server_buffer, "----- You won! -----\n") == 0) {
            break;
        }
        if (strcmp(server_buffer, "----- You lost! -----\n") == 0) {
            break;
        }
        if (strcmp(server_buffer, "----- GAME TIES -----\n") == 0) {
            break;
        }
        
        memset(&co_ordinates_buffer, 0, sizeof(co_ordinates_buffer));
        memset(&server_buffer, 0, sizeof(server_buffer));

        recv(sockfd, &server_buffer, sizeof(server_buffer), 0);
        cout << server_buffer;

        cin >> co_ordinates_buffer;
      
        //ni = input(co_ordinates_buffer);
        //cout << "\n" << ni << "\n";
        /*if (ni == 0)
        {
            inp++;*/

            cout << "sent " << co_ordinates_buffer << " to server!" << endl;
            bytes_sent = send(sockfd, &co_ordinates_buffer, sizeof(co_ordinates_buffer), 0);

            if (bytes_sent == -1)
            {
                perror("CO-ORDINATES BUFFER could not be sent!");
                return 1;
            }

        //}

        count++;
    }

    // if (nc == 'f')
    // 	cout<<endl<<"Game ends in a draw."<<endl;

    cout << endl
         << "Thank You for playing Tic-tac-Toe" << endl;
    close(sockfd);
    return 0;
}
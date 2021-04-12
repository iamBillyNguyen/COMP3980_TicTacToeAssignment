#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace std;


void display();
int input(char, int, int);


/**
 * Function to display the formatting of the tic tac toe game.
 */
void display()
{

    int i, j;
    system("clear");
    std::cout<<"\n\n Spots"<<endl<<endl;
    char cell = '0';
    for (i=0; i<3; i++)
    {
        std::cout<<"\t";
        for (j=0; j<3; j++){
            std::cout  <<" "<< cell << "    ";
            cell++;
        }
        std::cout<<endl;

    }

}
/**
 * Input validation.
 * @param choice player choice
 * @return 0 if valid, 1 if false
 */
int input(char choice[])
{
    char letters[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I'};
    bool flag = false;
    for(int i = 0; i < 9; i++){
        if(choice[0] == letters[i]){
            flag = true;
            return 0;
        }
    }
    std::cout<<"Invalid input, use A-I!\n";
    return -1;
}

/**Q
 * Function to display board.
 * @param cell
 * @param board board format
 * @param player player
 */
void update_board(uint8_t cell, char board[9], char player) {
    if (board[cell] == ' ') {
        board[cell] = player;

        printf("    %c  | %c  | %c\n", board[0], board[1], board[2]);
        printf("    --------------\n");
        printf("    %c  | %c  | %c\n", board[3], board[4], board[5]);
        printf("    --------------\n");
        printf("    %c  | %c  | %c\n", board[6], board[7], board[8]);
    }
}
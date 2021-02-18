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
	std::cout<<"\nWelcome to the Tic-Tac-Toe Game!\n\n Spots"<<endl<<endl;
    char letter = 'A';
    for (i=0; i<3; i++)
	{	
		std::cout<<"\t";
	    for (j=0; j<3; j++){
			std::cout  <<" "<< letter << "    ";    
				letter++;
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

/**
 * Function to display board.
 * @param c letter
 * @param board board format
 * @param player player
 */
void update_board(char c, char board[][3], char player) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (c == board[i][j]) {
                board[i][j] = player;
                break;
            }
        }
    }
    printf("    %c  %c  %c\n", board[0][0], board[0][1], board[0][2]);
    printf("    %c  %c  %c\n", board[1][0], board[1][1], board[1][2]);
    printf("    %c  %c  %c\n", board[2][0], board[2][1], board[2][2]);
}
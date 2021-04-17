#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace std;


void display_ttt();
void display_rps();
int input(char, int, int);
int get_game_option();


/**
 * Function to display the formatting of the tic tac toe game.
 */
void display_ttt()
{
    int i, j;
    system("clear");
    std::cout<<"\n\n-------------- Cells --------------"<<endl<<endl;
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

void display_rps() {
    system("clear");
    cout << "\n\n-------------- Rules --------------" <<endl<<endl;
    cout << "Rock:      1"<<endl;
    cout << "Paper:     2"<<endl;
    cout << "Scissors:  3"<<endl;
    cout << endl;
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

int get_game_option() {
    system("clear");
    int answer;
    bool done = false;
    while(!done) {
        cout << "-------------- Our game menu --------------" << endl;
        cout << "Choose one:" << endl;
        cout << "   1. Tic Tac Toe" << endl;
        cout << "   2. Rock Paper Scissors" << endl;
        cout << "Your choice: ";
        cin >> answer;
        if (answer < 1 || answer > 2)
            cout << "Option not available. Choose again" << endl;
        else
            done = true;
    }
    return answer;
}

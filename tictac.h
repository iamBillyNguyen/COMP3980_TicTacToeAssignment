#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace std;

char mat[3][3], p1, p2;
void  init();
void display();
int input(char, int, int);
char check();

void init()
{
    int i, j;
    for (i=0; i<3; i++)
		for (j=0; j<3; j++)
            mat[i][j] = '-';
}    

void display()
{
	
	int i, j;
	system("clear");
	cout<<"\nWelcome to the Tic-Tac-Toe Game!\n\nGrid Co-ordinates : "<<endl<<endl;
    char letter = 'A';
    for (i=0; i<3; i++)
	{	
		cout<<"\t";
	    for (j=0; j<3; j++){
			cout  <<" "<< letter << "    ";    
				letter++;
		}
		cout<<endl;   
	
	}       

	cout<<"\nThe Tic-Tac-Toe board : "<<endl<<endl;
	for (i=0; i<3; i++)
	{
		cout<<"\t";
	    for (j=0; j<3; j++)
		    cout<<" "<<mat[i][j]<<"     ";
		cout<<endl;
	}		
}

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
	cout<<"Invalid input, use A-I!\n";
	return -1;
	


}

char check()
{
	int i, j;
	char temp;

	for (j=0; j<3; j++)
	{
		if (mat[0][j] != '-')
		{
			temp = mat[0][j];
			if (j == 0)
			{
				if (mat[0][1] == temp)
					if (mat[0][2] == temp)
						return temp;
				if (mat[1][0] == temp)
					if (mat[2][0] == temp)
						return temp;
				if (mat[1][1] == temp)
					if (mat[2][2] == temp)	
						return temp;				
			}
			else if (j == 1)	
			{
				if (mat[1][1] == temp)
					if (mat[2][1] == temp)
						return temp;
			}
			else 
			{
				if (mat[1][2] == temp)
					if (mat[2][2] == temp)
						return temp;
			}
		}
	}
	for (i=1; i<3; i++)
	{
		if (mat[i][0] != '-')
		{
			temp = mat[i][0];
			if (i == 1)
			{
				if (mat[1][1] == temp)
					if (mat[1][2] == temp)
						return temp;
			}
			else
			{
				if (mat[2][1] == temp)
					if (mat[2][2] == temp)
						return temp;
				if (mat[1][1] == temp)
					if (mat[0][2] == temp)
						return temp;
			}
		}
	}
	
	return 'f';

}
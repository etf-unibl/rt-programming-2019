#include <stdio.h>
#include "wiringPi.h"

char status(char mat[3][3]);
void blinking(char);

int main (void)
{   
    wiringPiSetup();
	pinMode(0, OUTPUT);
    int i,j;
	char mat[3][3];
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			mat[i][j] = ((i + j)% 2 == 0 ? 'O' : 'X');	
    blinking(mat[1][1]);//pocetno stanje
    blinking(status(mat));//nakon promjene
	getchar();
    return 0;
}

void blinking(char status)
{
    int i;
    if (status=='O')
            for(i=0;i<20;i++)
            {
                digitalWrite(0, HIGH);
                delay(500);
            }
    else if(status=='X')
        for(i=0;i<20;i++)
        {
            digitalWrite(0, HIGH);
            delay(500);
            digitalWrite(0, LOW);
            delay(500);
        }
}

char status(char mat[3][3]) {
	
	int counter=0;
	int i,j;
	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			if(mat[i][j]=='O')
				counter++;
	if(mat[1][1]=='O')
		counter--;
	if(counter<2)
		mat[1][1]='X'; //smrt usljed manjka populacije
	else if(counter>=2 && counter <4 && mat[1][1]=='O')
		mat[1][1]='O'; //nastavlja da zivi dalje
	else if(counter>3)
        		mat[1][1]='X'; //smrt zbog prevelike populacije
	else if(counter==3 && mat[1][1]=='X')
		mat[1][1]='O';
    
	return mat[1][1];
}

/*
	Kada se program pokrece bez argumenata komandne linije,
	default-no pocetno stanje GOL celija je (1-ziva, 0-mrtva):
	1 1 1
	0 0 0
	0 0 0
	
	Prema pravilima Conway's GOL, celija u sredini ce da ozivi, pa ce novo stanje biti:
	1 1 1
	0 1 0
	0 0 0
	
	te ce se LED upaliti.
	
	Ukoliko zelimo da simuliramo drugacije situacije (da postavimo drugacije pocetno stanje celija),
	potrebno je, pri pokretanju programa, proslijediti argumente u komandnoj liniji,
	pri cemu prvi argument predstavlja stanje celije u sredini, a ostali argumenti stanje celija
	krecuci se u krug oko centralne celije, pocevsi od pozicije (0,0) u matrici.
	Npr, pokretanjem programa na slj. nacin:
	
	./gol_cell_updated 1 0 0 1 0 0 0 0 0
	
	postavljamo pocetno stanje matrice na:
	0 0 1
	0 1 0
	0 0 0
	
	pa ce novo stanje matrice po GOL algoritmu biti:
	0 0 1
	0 0 0
	0 0 0
	
	te ce LED poceti da blinka.
*/

#include <stdio.h>
#include <stdlib.h>
#include "wiringPi.h"

void setm(int** matrix, int* states)
{
	matrix[1][1] = states[0];
	matrix[0][0] = states[1];
	matrix[0][1] = states[2];
	matrix[0][2] = states[3];
	matrix[1][2] = states[4];
	matrix[2][2] = states[5];
	matrix[2][1] = states[6];
	matrix[2][0] = states[7];
	matrix[1][0] = states[8];
}

void printm(int** matrix)
{
	int i, j;
	for (i = 0; i < 3; ++i)
	{
		for (j = 0; j < 3; ++j)
		{
			printf("%d ", matrix[i][j]);
		}
		printf("\n");
	}
}

int update_cell(int* states)
{
	int i, nc = 0;
	for (i = 1; i < 9; ++i)
	{
		if (states[i]) nc++;
	}

	if (states[0] == 1 && (nc < 2 || nc > 3)) states[0] = 0;
	else if (nc == 3) states[0] = 1;

	return states[0];
}

int main(int argc, char* argv[])
{
	wiringPiSetup();
	pinMode(0, OUTPUT);
	
	int** matrix = malloc(3*sizeof(int*));
	int i, j;
	for (i = 0; i < 3; ++i)
	{
		matrix[i] = malloc(3*sizeof(int));
	}

	int states[] = { 0, 1, 1, 1, 0, 0, 0, 0, 0 };
	if (argc == 10)
		for(i = 0; i < 9; ++i)
			states[i] = *argv[i+1] - '0';
	setm(matrix, states);

	printf("Staro stanje:\n");
	printm(matrix);

	int cell_status = update_cell(states);
	setm(matrix, states);

	printf("\nNovo stanje:\n");
	printm(matrix);
	
	if (cell_status)
		digitalWrite(0, HIGH);
	else
		for (;;)
		{
			digitalWrite(0, HIGH);
			delay(500);
			digitalWrite(0, LOW);
			delay(500);
		}

	return 0;
}
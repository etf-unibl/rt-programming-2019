#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N_ROWS 3
#define N_COLUMNS 3

char is_alive(char**);
void print_matrix(char**);

void main(void)
{
    // Allocate the matrix
    char** states = (char**)malloc(N_ROWS * sizeof(char*));
    uint8_t i;
    uint8_t j;
    for(i = 0; i < N_ROWS; i++)
	states[i] = malloc(N_COLUMNS * sizeof(char));

    
    printf("Enter neighbour's state(0 - dead or 1 - alive):\n");
    
    for(i = 0; i < N_ROWS; i++)
    {
	for(j = 0; j < N_COLUMNS; j++)
	{
	    if(i == 1 && j == 1)
		continue;
	    printf("A[%d][%d]: ", i, j);
	    scanf(" %c", &states[i][j]);
	}
    }
    printf("Enter cell initial state: ");
    scanf(" %c", &states[1][1]);
    
    printf("\nInitial state: \n");
    print_matrix(states);

    states[1][1] = is_alive(states);
    
    printf("\nNew state: \n");
    print_matrix(states);
}


/*
 * Function that checks if the cell is alive, based on the neighbours' states.
 * parameter:    states - char** - matrix of chars that represents neighbours' states
 * return:       either '0' = dead or '1' = alive
 */
char is_alive(char** states)
{
    // Count the living neighbours
    uint8_t neighbours_alive_counter = 0;
    uint8_t i;
    uint8_t j;
    for(i = 0; i < N_ROWS; i++)
    {
	for(j = 0; j < N_COLUMNS; j++)
	{
	    if(i == 1 && j == 1)
		continue;
	    if(states[i][j] != '0')
		neighbours_alive_counter++;
	}
    }

    printf("\nNeighbours alive: %d\n", neighbours_alive_counter);

    // Based on the living neighbours, decide whether or not the cell lives
    if(states[1][1] && (neighbours_alive_counter < 2 || neighbours_alive_counter > 3))
    {
	printf("\nThe cell will die.\n");
	return '0';
    }
    else if(states[1][1] == '0' && neighbours_alive_counter == 3)
    {
	printf("\nThe cell is reborn.\n");
	return '1';
    }

    // This happens when the living cell has 2 or 3 living neighbours
    printf("\nThe cell lives on.\n");
    return '1';
}


/*
 * Utility function that prints the matrix.
 * parameter:   states - char** - matrix of chars
 * return:      void
 */
void print_matrix(char** states)
{
    uint8_t i;
    uint8_t j;
    for(i = 0; i < N_ROWS; i++)
    {
	for(j = 0; j < N_COLUMNS; j++)
	{
	    if(states[i][j] == '0')
		printf("X ");
	    else
		printf("O ");
	}
	printf("\n");
    }
}

#include <stdio.h>
#include <stdlib.h>

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

int main()
{
	int** matrix = malloc(3 * sizeof(int*));
	int i, j;
	for (i = 0; i < 3; ++i)
	{
		matrix[i] = malloc(3 * sizeof(int));
	}

	int states[] = { 0, 1, 1, 1, 0, 0, 0, 0, 0 };
	setm(matrix, states);

	printf("Staro stanje:\n");
	printm(matrix);

	int cell_status = update_cell(states);
	setm(matrix, states);

	printf("\nNovo stanje:\n");
	printm(matrix);

	return 0;
}

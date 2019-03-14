#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{

	write(1, "BEFORE MALLOC\n", 14);
	malloc(150000);
	write(1, "AFTER MALLOC\n", 13);
	
	getchar();
	return 0;
}

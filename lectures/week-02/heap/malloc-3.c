#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	int i;

	write(1, "START\n", 6);
	malloc(1);
	getchar();

	write(1, "LOOP START\n", 11);
	
	for (i = 0; i < 0x25000 / 1024; i++)
		malloc(1024);

	write(1, "LOOP END\n", 9);

	getchar();
	return 0;
}

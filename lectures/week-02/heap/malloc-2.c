#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	void *p;

	write(1, "BEFORE MALLOC #1\n", 17);
	p = malloc(1024);
	write(1, "AFTER MALLOC #1\n", 16);
	printf("%p\n", p);

	write(1, "BEFORE MALLOC #2\n", 17);
	p = malloc(1024);
	write(1, "AFTER MALLOC\n #2", 16);
	printf("%p\n", p);
	
	write(1, "BEFORE MALLOC #3\n", 17);
	p = malloc(1024);
	write(1, "AFTER MALLOC #3\n", 16);
	printf("%p\n", p);

	write(1, "BEFORE MALLOC #4\n", 17);
	p = malloc(1024);
	write(1, "AFTER MALLOC #4\n", 16);
	printf("%p\n", p);
	
	getchar();
	return 0;
}

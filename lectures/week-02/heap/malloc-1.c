#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	void *p;

	write(1, "BEFORE MALLOC\n", 14);
	p = malloc(1);
	write(1, "AFTER MALLOC\n", 13);
	printf("%p\n", p);
	getchar();
	return 0;
}

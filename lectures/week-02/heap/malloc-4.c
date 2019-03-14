#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>

int main()
{
	
	// disable mmap allocation
	//mallopt(M_MMAP_MAX, 0);
	// redefine mmap threshold
	//mallopt(M_MMAP_THRESHOLD, 150*1024);

	write(1, "BEFORE MALLOC\n", 14);
	malloc(150000);
	write(1, "AFTER MALLOC\n", 13);
	
	getchar();
	return 0;
}

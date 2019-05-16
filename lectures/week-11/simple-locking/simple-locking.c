#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/resource.h>

#define SOMESIZE (100*1024)

int main()
{
	// Allocate some memory
	int i, page_size;
	char *buffer;
	struct rusage usage;

	printf("Before mlockall.\n");

	// Lock all current and future pages and prevent paging
	if (mlockall(MCL_CURRENT | MCL_FUTURE))
		perror("mlockall failed:");
	
	page_size = sysconf(_SC_PAGESIZE);
	buffer = malloc(SOMESIZE);

	// Touch each page to get it mapped in RAM
	for (i=0; i < SOMESIZE; i+=page_size)
	{
		buffer[i] = 0;
		getrusage(RUSAGE_SELF, &usage);
		printf("Major Pagefaults: %d, Minor Pagefaults: %d\n", usage.ru_majflt, usage.ru_minflt);
	}

	printf("After mlockall.\n");

	// From now on, the buffer is never released or swapped (i.e., no page faults)
	for (i=0; i < SOMESIZE; i+=page_size)
	{
		buffer[i] = 100;
		getrusage(RUSAGE_SELF, &usage);
		printf("Major Pagefaults: %d, Minor Pagefaults: %d\n", usage.ru_majflt, usage.ru_minflt);
	}
	
	free(buffer);

	// Unlockall the pages
	if (munlockall())
		perror("munlockall failed:");

	return 0;
}

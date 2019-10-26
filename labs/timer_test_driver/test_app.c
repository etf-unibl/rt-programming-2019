#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define TIMER_START _IOW('q',1,char*)
#define TIMER_PAUSE _IOW('q',2,char*)
#define TIMER_READ _IOR('q',3,char*)
#define TIMER_STOP _IOW('q',4,char*)
#define TIMER_STOP_ALL _IOW('q',5,char*)
#define TIMER_DELETE _IOW('q',6,char*)
#define TIMER_DELETE_ALL _IOW('q',7,char*)

int main(int argc, char **argv)
{
	
    char* command;
    char* name;
	char dummy[10]="dummy";
	int file_desc;
	
	file_desc = open("/dev/timer_driver", O_RDWR);

    if(file_desc < 0)
    {
        printf("Error, file not opened.\n");
        return -1;
    }
	
	if(argc == 2)
	{
		char* command=argv[1];
		if(strcmp(command,"stop_all") == 0)
			ioctl(file_desc, TIMER_STOP_ALL,dummy);
		else if(strcmp(command,"delete_all")==0)
			ioctl(file_desc, TIMER_DELETE_ALL,dummy);
		else
			printf("Unknown command, you have commands:\n'start' 'pause' 'stop' 'stop_all' 'read' 'delete' 'delete_all\n"); 
		
	}
	else if (argc == 3)
	{
		command=argv[1];
		name=argv[2];
		
		if(strcmp(command,"start") == 0)
			ioctl(file_desc, TIMER_START,name);
		else if(strcmp(command,"pause") == 0)
			ioctl(file_desc, TIMER_PAUSE,name);
		else if(strcmp(command,"read") == 0)
			ioctl(file_desc, TIMER_READ,name);
		else if(strcmp(command,"stop") == 0)
			ioctl(file_desc, TIMER_STOP,name);
		else if(strcmp(command,"delete") == 0)
			ioctl(file_desc, TIMER_DELETE,name);	
		else
			printf("Unknown command, you have commands:\n'start' 'pause' 'stop' 'stop_all' 'read' 'delete' 'delete_all\n");
	}
	else 
		printf("Wrong number of command line arguments!\nArguments should be 'command' 'timer_name' if "
	"you have specific command for timer.\nIf you want to issue commands for all timers argument should be just 'command'.\n");

    return 0;
}

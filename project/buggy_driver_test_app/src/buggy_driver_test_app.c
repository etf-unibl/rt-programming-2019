#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define FORWARD        _IOW('p',1,unsigned)
#define BACKWARD       _IOW('p',2,unsigned)
#define LEFT           _IOW('p',3,unsigned)
#define RIGHT          _IOW('p',4,unsigned)
#define LEFT_BLINKERS  _IOW('p',5,unsigned)
#define RIGHT_BLINKERS _IOW('p',6,unsigned)
#define BRAKE          _IOW('p',7,unsigned)
#define HEADLIGHTS     _IOW('p',8,unsigned)
#define NO_ACTION      _IOW('p',9,unsigned)

void changemode(int dir);

int main()
{
	changemode(1);
	
	char c;
	int file_desc;
	
	file_desc = open("/dev/buggy_driver", O_RDWR);

    if(file_desc < 0)
    {
        printf("'/dev/buggy_driver' device isn't open\n");
        printf("Try:\t1) Check does '/dev/gpio_driver' node exist\n\t2)'chmod 666 /dev/gpio_Driver'\n\t3) ""insmod"" gpio_driver module\n");
        changemode(0);
		return -1;
    }
	
	while(1)
	{
		c = getchar();
		
		if (c == 'w' || c == 'W'){
			if (ioctl(file_desc, FORWARD, c) == -1)
			{
				perror("ioctl - forward error\n");
				return -1;
			}
		}
		else if (c == 's' || c == 'S'){
			if (ioctl(file_desc, BACKWARD, c) == -1)
			{
				perror("ioctl - backward error\n");
				return -1;
			}
		}
		else if (c == 'a' || c == 'A'){
			if (ioctl(file_desc, LEFT, c) == -1)
			{
				perror("ioctl - left error\n");
				return -1;
			}
		}
		else if (c == 'd' || c == 'D'){
			if (ioctl(file_desc, RIGHT, c) == -1)
			{
				perror("ioctl - right error\n");
				return -1;
			}
		}
		else if (c == 'q' || c == 'Q'){
			if (ioctl(file_desc, LEFT_BLINKERS, c) == -1)
			{
				perror("ioctl - left_blinkers error\n");
				return -1;
			}
		}
		else if (c == 'e' || c == 'E'){
			if (ioctl(file_desc, RIGHT_BLINKERS, c) == -1)
			{
				perror("ioctl - right_blinkers error\n");
				return -1;
			}
		}
		else if (c == 'b' || c == 'B'){
			if (ioctl(file_desc, BRAKE, c) == -1)
			{
				perror("ioctl - brake error\n");
				return -1;
			}
		}
		else if (c == 'h' || c == 'H'){
			if (ioctl(file_desc, HEADLIGHTS, c) == -1)
			{
				perror("ioctl - brake error\n");
				return -1;
			}
		}
		else if (c == 'x' || c == 'X'){
			break;
		}
		sleep(0);
	}
	
	changemode(0);
	return 0;
}

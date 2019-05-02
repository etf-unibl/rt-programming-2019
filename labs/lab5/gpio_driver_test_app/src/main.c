#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>

#define BUF_LEN 80

#define GPIO_PIN_GET _IOR('p',1,gpio_pin_val*)
#define GPIO_PIN_SET _IOW('p',2,gpio_pin_val*)

typedef struct {
	char pin;
	unsigned int direction;
} gpio_pin_val;

int main(int argc, char* argv[])
{
    int file_desc;
    gpio_pin_val p;
	
	file_desc = open("/dev/gpio_driver", O_RDWR);

    if(file_desc < 0)
    {
        printf("'/dev/gpio_driver' device isn't open\n");
        printf("Try:\t1) Check does '/dev/gpio_driver' node exist\n\t2)'chmod 666 /dev/gpio_Driver'\n\t3) ""insmod"" gpio_driver module\n");
        return -1;
    }
	
	int i,j;
	if(argc == 2)
	{
		sscanf(argv[1], "%d", &i);

		if(i >= 2 && i <= 27)
		{
			p.pin = i;
			if (ioctl(file_desc, GPIO_PIN_GET, &p) == -1)
			{
				perror("ioctl - pin get error: ");
				return -1;
			}
			printf("GPIO Pin %d has direction %s.\n", p.pin, (p.direction)? "OUT" : "IN");
		}
	}
	else if (argc == 3)
	{
		sscanf(argv[1], "%d", &i);
		sscanf(argv[2], "%d", &j);
		
		if(i >= 2 && i <= 27)
		{
			p.pin = (char)i;
			if(j == 0) p.direction = 0;
			else
				p.direction = 1;
			if (ioctl(file_desc, GPIO_PIN_SET, &p) == -1)
			{
				perror("ioctl - pin set error: ");
				return -1;
			}
			printf("GPIO Pin %d successfully set to direction %s.\n", p.pin, (p.direction)? "OUT" : "IN");
		}
	}
	
    close(file_desc);
    
    return 0;
}

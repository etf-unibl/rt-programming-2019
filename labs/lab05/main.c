/*
 * This program is a test application for gpio driver that changes the pin direction.
 *
 * Author: Slaven Smiljanic
 * 
 * 13.04.2019 21:44
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define ESC 27

// IOCTL commands:
#define OUTPUT _IOW('a', 'a', int*)
#define INPUT _IOW('a', 'b', int*)
#define PULLUP _IOW('a', 'c', int*)
#define PULLDOWN _IOW('a', 'd', int*)
#define PULLNONE _IOW('a', 'e', int*)

// Function declarations:
void* check_switch(void*);
void* check_quit(void*);

// Global variables:
volatile char done = 0x0;


int main()
{
    pthread_t switch_thread;
    pthread_t quit_thread;
    
    int pin0 = 0;
    int pin1 = 1;
    int file_desc;

    file_desc = open("/dev/gpio_driver", O_RDWR);

    if(file_desc < 0)
    {
        printf("Error, file not opened\n");
        return -1;
    }
    
    ioctl(file_desc, OUTPUT, &pin0);
    ioctl(file_desc, PULLUP, &pin1);
    ioctl(file_desc, INPUT, &pin1);

    wiringPiSetup();
    
    pthread_create(&switch_thread, NULL, check_switch, (void*)&pin1);
    pthread_create(&quit_thread, NULL, check_quit, NULL);

    pthread_join(quit_thread, NULL);

    ioctl(file_desc, PULLNONE, &pin1);
    ioctl(file_desc, INPUT, &pin0);
    close(file_desc);
    return 0;
}


/*
 * This function checks the given pin and turns the LED on or off.
 *
 */
void* check_switch(void* p)
{
    uint8_t pin = *(uint8_t*)p;
    while(done != ESC)
    {	
	if(digitalRead(pin) && !digitalRead(0))
	    digitalWrite(0, HIGH);
	else
	    digitalWrite(0, LOW);	   
    }
}


/*
 * This function checks if the user pressed ECS and sets 'done' variable accordingly.
 *
 */
void* check_quit(void* p)
{
    while(done != ESC)
	done = getch();
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include<semaphore.h>

#define DISTANCE_READ _IOR('q',1,uint64_t*)
#define STOP_IN_BACKGROUND _IOW('q',2,int8_t*)

int file_desc;
char getch(void);
sem_t semFinishSignal;


void* writing(void* arg)
{
	uint64_t value;
    while(1)
	{
        if(sem_trywait(&semFinishSignal)==0) //	If application is exiting, break from the loop
            break;
        ioctl(file_desc, DISTANCE_READ, (uint64_t*) &value);
		printf("Udaljenost je %04f cm\n",(double)value*0.0343/2);
		fflush(stdout);
		usleep(100000);
    }
	return NULL;
}

void* reading(void* arg)
{
    int8_t c;
	printf("Pritisnite ESC za izlazak iz programa\n");
    while(1)
	{
        if(sem_trywait(&semFinishSignal)==0) //	If application is exiting, break from the loop
            break;
        c=getch();
            
        if(c==27)	// Decimal 27 is ASCII for ESC	
		{
			ioctl(file_desc, STOP_IN_BACKGROUND, (int8_t*) &c); // Stop our timer in background
            sem_post(&semFinishSignal);
            sem_post(&semFinishSignal);
        }
    }
	return NULL;
}

int main(int argc, char **argv)
{
	file_desc = open("/dev/ultrasonic_driver", O_RDWR);

    if(file_desc < 0) {
        printf("Error, file not opened.\n");
        return -1;
    }
	
    pthread_t threadReader;
    pthread_t threadWriter;
	
	sem_init(&semFinishSignal,0,0);
	
	pthread_create(&threadReader,NULL,reading,0);
    pthread_create(&threadWriter,NULL,writing,0);
    
    pthread_join(threadReader,NULL);
    pthread_join(threadWriter,NULL);
    
    sem_destroy(&semFinishSignal);
	
    return 0;
}

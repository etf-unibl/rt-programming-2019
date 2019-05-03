#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "IOCTLdevBlinker.h"

static int f;

enum State{INPUT=0,OUTPUT=1};

void pinMode(char pin,enum State state){
	if(state==INPUT)
	ioctl(f,IOCTL_INPUT_PIN,pin);
	else
	ioctl(f,IOCTL_OUT_PIN,pin);
}

enum Out{LOW=0,HIGH=1};

void digitalWrite(char pin, enum Out out){
	if(out==LOW)
	ioctl(f,IOCTL_CLEAR_PIN,pin);
	else
	ioctl(f,IOCTL_SET_PIN,pin);
}

int main(void){
	if((f=open("/dev/gpio",O_RDWR))==-1){
		printf("Error while opening a device\n");
		return -1;
	}
	
	char command;
	int pin_no;
	while(command!='q'){
		printf("What do you want to do?\n");
		printf("To set a pin n to input type i\n");
		printf("To set a pin n to output type o\n");
		printf("To set a pin n to high type h\n");
		printf("To set a pin n to low type l\n\n\n\n\n\n\n");
		
		scanf("%c",&command);
		switch(command){
			case 'g':
			ioctl(f,IOCTL_HELLO);
			break;
			case 'i':
			printf("Enter pin number:\n");
			scanf("%d",&pin_no);
			ioctl(f,IOCTL_INPUT_PIN,pin_no);
			break;
			case 'o':
			printf("Enter pin number:\n");
			scanf("%d",&pin_no);
			ioctl(f,IOCTL_OUT_PIN,pin_no);
			break;
			case 'h':
			printf("Enter pin number:\n");
			scanf("%d",&pin_no);
			ioctl(f,IOCTL_SET_PIN,pin_no);
			break;
			case 'l':
			printf("Enter pin number:\n");
			scanf("%d",&pin_no);
			ioctl(f,IOCTL_CLEAR_PIN,pin_no);
			break;
			case 's':
			printf("Enter pin number:\n");
			scanf("%d",&pin_no);
			ioctl(f,IOCTL_PIN_STATE,&pin_no);
			if(pin_no==0) printf("Pin is low\n");
			else printf("Pin is high\n");
			
		}
	}
	close(f);
	return 0;
}

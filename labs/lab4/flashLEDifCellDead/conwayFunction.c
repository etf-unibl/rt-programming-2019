
#include<stdio.h>
#include<stdlib.h>

#include"wiringPi/wiringPi/wiringPi.h"
#include<pthread.h>

#define OUT_LED 0

char stop=0;

void* workWithLED(void* ptr){
    
    char mod=*(char*)ptr;
    
    while(!stop){

	digitalWrite(OUT_LED, HIGH);
	delay(200);
	if(mod==0){	
		digitalWrite(OUT_LED, LOW);
		delay(200);
	}
    }
}

unsigned int deadOrAlive(char environment[3][3]){
 
    unsigned int neighNum=0;
    
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            if(i==1&&j==1) continue;
            if(environment[i][j]) neighNum++;
        }
    }
    printf("The cell has %d neighbours\n",neighNum);
    //live cell with less than two neighbours -> dies
    //live cell with two or three neighours -> lives
    //live cell with more than three neighbours -> dies
    //Dead cell with exactly three neighbours -> lives
    
    if(environment[1][1]==0){
        if(neighNum==3) return 1;
    }
    else{
        if(neighNum<2) return 0;
        if(neighNum>3) return 0;
    }
    return 1;
}

int main(int argv,char* argc[]){
 
    printf("%d",argv);

    for(int i=0;i<argv;i++) printf("%d\n",argc[i][0]-0x30);
	
    if(argv!=10) return -1;
   
    char state[3][3]={
        {argc[1][0]-0x30,argc[2][0]-0x30,argc[3][0]-0x30},
        {argc[4][0]-0x30,argc[5][0]-0x30,argc[6][0]-0x30},
        {argc[7][0]-0x30,argc[8][0]-0x30,argc[9][0]-0x30}
    };
    
    char newState[3][3];
    
    for(int i=0;i<3;i++){
     for(int j=0;j<3;j++)
         newState[i][j]=state[i][j];
    }
    
    newState[1][1]=deadOrAlive(state);
    
    printf("In the next iteration the cell will ");
    if(newState[1][1]==0) printf("die\n");
    else printf("live\n");

    wiringPiSetup();
    pinMode(OUT_LED, OUTPUT);

    pthread_t t1;
    pthread_create(&t1,NULL,workWithLED,(void*)&newState[1][1]);

    printf("Previous state:\n");
    for(int i=0;i<3;i++){
     for(int j=0;j<3;j++)
         printf("%d",state[i][j]);
     printf("\n");
    }
    
    printf("Next state:\n");
    for(int i=0;i<3;i++){
     for(int j=0;j<3;j++)
         printf("%d",newState[i][j]);
     printf("\n");
    }
    
    delay(5000);
    stop=1;    
    pthread_join(t1,NULL);  

    return 1;
}

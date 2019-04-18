#include <stdio.h>
#include "wiringPi.h"
#include <pthread.h>
#include <semaphore.h>

void status(char mat[3][3]);
void* blinking(void*);

sem_t semFinishSignal;
pthread_mutex_t controlLocker;

/* Posto mi je c pravio problem prilikom dinamicke alokacije matrice, ovo je rjesenje da napravim
 * dijeljeni objekat koji je ustvari matrica sa stanjima susjeda, problem kod ovoga je relativno
 * dugacka kriticna sekcija, ali posto se pristupa 9 elemenata to i nije nekakav strasan gubitak vremena*/
char mat[3][3];

void* blinking(void* arg)
{
    char cont;
    while(1)
	{
        /*Ako pokupimo signal za kraj, odnosno 'Q' ili 'q' izadji iz petlje i zavrsi program*/
        if(sem_trywait(&semFinishSignal)==0)
            break;
        
        /*Koristimo mutex jer ovdje citamo iz zajednicke promjenljive a u funkciji cancle pisemo u nju*/
        pthread_mutex_lock(&controlLocker);        
        cont=mat[1][1];
        pthread_mutex_unlock(&controlLocker);
        /*Ako je celija ziva, dioda svijetli*/
        if (cont=='O')
        {
            digitalWrite(0, HIGH);
            delay(500); //delay kaze ne radi nista na 500 milisekundi, ali ne prepustaj moje procesorsko vrijeme
        }
        
        /*Ako je celija mrtva, dioda treperi*/
        else if(cont=='X')
            {
                digitalWrite(0, HIGH);
                delay(500);
                digitalWrite(0, LOW);
                delay(500);
            }
        
        /*da nam odradi kontekst svic*/
        usleep(2000);

    }
}

void* cancle(void* arg)
{
    char c;
    while(1)
    {
        /*Moramo ugasiti i ovaj thread, pa cekamo semafor*/
        if(sem_trywait(&semFinishSignal)==0)
        break;
        
        c=getchar();//ne blokirajuci poziv, cita i upisuje bez entera
        
        /*Ukoliko je vrijednost ucitanog karaktera sa tastature q ili Q, signaliziraj kraj thread-ovima*/
        if(c=='Q' || c=='q')
        {
            sem_post(&semFinishSignal);
            sem_post(&semFinishSignal);
            sem_post(&semFinishSignal);
        }
        
        /*spavaj na 2000 mikro sekundi jer zelimo da prepustimo proc. vrijeme blinkeru*/
        usleep(2000);
        
    }
}
/*Koristim thread change jer ukoliko bi pozivao funkciju u main-u morao bi koristiti delay, koji uspavljuje 
 * nit ali ne pravi promjenu konteksta pa je ovako program responzivniji, odnosno da sam koristio delay morao
 * bih cekati 5 sekundi da uhvati izlaz iz programa na slovo q*/
void* change(void* arg)
{
    int i=0;
    printf("Status will change in 5 seconds\n");
    while(1)
    {
        if(sem_trywait(&semFinishSignal)==0)
            break;
        /*svaki put uvecaj za 1 i uspavaj na sekundu, tako da je ovo mini brojac*/
        i++;
        usleep(1000000);
        if(i==5)
            status(mat);
    }
}


/*Funkcija koja odredjuje status celije koju posmatramo*/
void status(char mat[3][3]) {
	
	int counter=0;
	int i,j;
    
    /*posto upisujemo u djeljenu promjenljivu moramo je zakljucati*/
    pthread_mutex_lock(&controlLocker);        
    /*prebroj zive u matrici*/
    for(i=0;i<3;i++)
		for(j=0;j<3;j++)
            if(mat[i][j]=='O')
				counter++;
	if(mat[1][1]=='O')
		counter--;
	if(counter<2)
		mat[1][1]='X'; //smrt usljed manjka populacije
	else if(counter>=2 && counter <4 && mat[1][1]=='O')
		mat[1][1]='O'; //nastavlja da zivi dalje
	else if(counter>3)
        		mat[1][1]='X'; //smrt zbog prevelike populacije
	else if(counter==3 && mat[1][1]=='X')
		mat[1][1]='O';
    /*ispis*/
    for(i=0;i<3;i++){
		for(j=0;j<3;j++)
            printf("%c",mat[i][j]);
        printf("\n");
    }
    pthread_mutex_unlock(&controlLocker); 
    
}


int main (void)
{   
    pthread_t blinker;
    pthread_t canceler;
    pthread_t worker;
    
    int i,j;
	
	wiringPiSetup();
	pinMode(0, OUTPUT);
    
    
    sem_init(&semFinishSignal,0,0);
 
    pthread_mutex_init(&controlLocker,NULL);
    
    /*postavljamo vrijednosti u matricu susjeda i zakljucavamo je*/
    
    pthread_mutex_lock(&controlLocker);        
    
    for (i = 0; i < 3; i++){ 
		for (j = 0; j < 3; j++)
        {
            mat[i][j] = ((i + j)% 2 == 0 ? 'O' : 'X');
            printf("%c",mat[i][j]);
        }
        printf("\n");
    }
    
    pthread_mutex_unlock(&controlLocker);   
    
    pthread_create(&blinker,NULL,blinking,0);
    pthread_create(&canceler,NULL,cancle,0);
    pthread_create(&worker,NULL,change,0);
    
	pthread_join(blinker,NULL);
	pthread_join(canceler,NULL);
    pthread_join(worker,NULL);
    
    sem_destroy(&semFinishSignal);
    
    pthread_mutex_destroy(&controlLocker);
    
    /*ukoliko je dioda ostala da svjetlucka, moramo je ugasiti*/
    digitalWrite(0, LOW);
 
    return 0;
}


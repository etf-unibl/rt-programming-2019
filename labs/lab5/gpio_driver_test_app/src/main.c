#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <stdint.h>



/*Definisemo koje su moguce komande zadate prilikom ioctl, a to su read i write, argument koji se prosljedjuje je broj pina
kao i smijer up ili down te direction in ili out (zavisno da li setujemo nekakav prekidac ili diodicu*/

#define PIN_READ _IOR('q',1,pin_arg*)
#define PIN_SET _IOW('q',2,pin_arg*)

/*Struktura u kojoj koristim int-ove, jer je c nezgodan za interaktivni meni zbog ostavljanja entera u baferu"*/
typedef struct {
	uint32_t pin;
	uint32_t direction;
	uint32_t value;
} pin_arg;

/*Niz koji nam sluzi da po izlazu iz programa postavimo sve pinove u neutralan polozaj*/
uint8_t pins[28]={0};

/*Argument kojem pristupaju sve funkcije*/
pin_arg arg;

/*
	Funkcija za ocitavanje vrijednosti pina, kao argument prima file deskriptor, provjerava da li je pin koji se provjerava u odgovarajucem
	opsegu, ako jeste procita se vrijednost, ako nije preskoci se citanje.
*/
void read_pin(int file_desc) {
	
	printf("Unesite vrijednosti pina kojeg zelite da ocitate (izmedju 2 i 27).\n");
	scanf("%d",&arg.pin);
	getchar();
	if(arg.pin>=2 && arg.pin<=27)
	{
		if (ioctl(file_desc, PIN_READ, &arg) == -1)
		{
			perror("Could not read pins from gpio_driver\n");
			goto error_loading;
		}
		printf("Pin %d ima smjer %s i vrijednost na izlazu %s.\n",arg.pin,(arg.direction==0)?"IN":"OUT",(arg.value==0)?"LOW":"HIGH");
	}
	else{
		error_loading:
		printf("Unijeli ste pogresnu vrijednost, ponovite izbor.\n");
		getchar();
	}

}

/*
	Funkcija za upisivanje vrijednosti pinova, kao parametar se prosljedjuje file deskriptor, provjerava se vrijednost unesenih pinova, te parametara za njih
	ukoliko je pin postavljen na in nema potrebe da se za njega postavlja izlazna vrijednost low ili high (mislim da je po defaultu high).
*/
void write_pin(int file_desc) {
	
	printf("Unesite vrijednost pina kojeg zelite da podesite (izmedju 2 i 27).\n");
	scanf("%d",&arg.pin);
	if(arg.pin>=2 && arg.pin<=27)
	{
		printf("Unesite da li zelite da vam pin bude ulazni (0) ili izlazni (1)\n");
		scanf("%d",&arg.direction);
		if(arg.direction>=0 && arg.direction<=1)
		{
			if(arg.direction==1)
			{
				printf("Unesite da li zelite da vrijednost pina bude LOW (0) ili HIGH (1)\n");
				scanf("%d",&arg.value);
				getchar();
				if(arg.value<0 && arg.value>1)
					goto error_loading;
			}
			/* Postavlja se kao fleg da se poslije mogu ugasiti pinovi*/
			pins[arg.pin]=1;
			if (ioctl(file_desc, PIN_SET, &arg) == -1)
			{
				perror("Could not read pins from gpio_driver\n");
			}
		}
	}
	else{
		error_loading:
		printf("Unijeli ste pogresnu vrijednost, ponovite izbor.\n");
		getchar();
	}
}

int main()
{
    int file_desc;
	char t=1,c;
	int i;
	
	file_desc = open("/dev/gpio_driver", O_RDWR);

    if(file_desc < 0)
    {
        printf("Error, file not opened.\n");
        return -1;
    }
	/*Imamo nesto kao izborni meni*/
	while(t) {
		printf("Ukoliko zelite da procitate vrijednosti pinova unesite slovo r.\nUkoliko zelite da postavite vrijednosti pinova unesite slovo s.\nUkoliko zelite da zavrsite aplikaciju unesite slovo q.\n");
		c=getchar();
		switch (c){
			case 'r':
				read_pin(file_desc);
				break;
			case 's':
				write_pin(file_desc);
				break;
			case 'q':
				t=0;
				break;
			default:
				printf("Unijeli ste pogresno slovo.\n");
				getchar();
				break;
		}
	}
    /*Postavljamo pinove na IN direction da ne trose struju po izlasku iz programa*/
	arg.direction=0;
	arg.value=0;
	for(i=0;i<28;i++)
		if(pins[i]==1)
		{
			arg.pin=(unsigned char)i;
			if (ioctl(file_desc, PIN_SET, &arg) == -1)
			{
				perror("Could not read pins from gpio_driver\n");
			}
		}
		
    close(file_desc);
    
    return 0;
}

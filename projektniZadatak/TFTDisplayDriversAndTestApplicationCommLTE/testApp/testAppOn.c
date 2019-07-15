/*
To test that the Linux framebuffer is set up correctly, and that the device permissions
are correct, use the program below which opens the frame buffer and draws a gradient-
filled red square:
retrieved from:
Testing the Linux Framebuffer for Qtopia Core (qt4-x11-4.2.2)
http://cep.xor.aps.anl.gov/software/qt4-x11-4.2.2/qtopiacore-testingframebuffer.html
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <unistd.h>

#include <stdio.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

char getch(){
    /*#include <unistd.h>   //_getch*/
    /*#include <termios.h>  //_getch*/
    char buf=0;
    struct termios old={0};
    fflush(stdout);
    if(tcgetattr(0, &old)<0)
        perror("tcsetattr()");
    old.c_lflag&=~ICANON;
    old.c_lflag&=~ECHO;
    old.c_cc[VMIN]=1;
    old.c_cc[VTIME]=0;
    if(tcsetattr(0, TCSANOW, &old)<0)
        perror("tcsetattr ICANON");
    if(read(0,&buf,1)<0)
        perror("read()");
    old.c_lflag|=ICANON;
    old.c_lflag|=ECHO;
    if(tcsetattr(0, TCSADRAIN, &old)<0)
        perror ("tcsetattr ~ICANON");
    printf("%c\n",buf);
    return buf;
 }

 int cx = 400; 
 int cy = 240;       // Where we are going to put the pixel
 
 volatile int xcoord=400;
 volatile int ycoord=240;
 volatile char chg=0;
 volatile char c=0;
 void* getCoords(void* a)
 {
     printf("Ulaz");
	FILE* fd = open("/dev/input/event0", O_RDWR);
	struct input_event event;
	
	while(c!='q')
	{
		int count = read(fd, &event, sizeof(struct input_event));
		if(count > 1)
		{
			for(int i = 0; i < (int)count / sizeof(struct input_event); i++)
			{
				if(EV_ABS == event.type)
				{
					if(event.code == 0 && event.value < 1024){
                        xcoord=(int)(event.value * 0.78125);
                    }else if(event.value < 600){ 
                            ycoord=(int)(event.value * 0.8);
                    }
                    printf("X:%d Y:%d\n",xcoord,ycoord);
                    chg=1;
                }
			}
		}
	}
	close(fd);
}
 
 unsigned char r=0;
 unsigned char g=0;
 unsigned char b=0;
 int f1=1;
 int f2=0;
 int f3=0;
 
 void* inputData(void* a){
     
    while((c=getch())!='q'){
        if(c=='d') cx=(cx+5)%800;
        if(c=='a') cx-=5;
        if(cx<0) cx=800;
        
        if(c=='s') cy=(cy+5)%480;
        if(c=='w') cy-=5;
        if(cy<0) cx=480;
        
        c=0;
        
    }
     
 }
 
int main()
{
    pthread_t coords; 
    pthread_create(&coords, NULL, getCoords, NULL);
    pthread_t input; 
    pthread_create(&input, NULL, inputData, NULL);

    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;
    long int location = 0;

    // Open the file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    
    printf("%d",screensize);
    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }
    printf("The framebuffer device was mapped to memory successfully.\n");

    while(c!='q'){
    // Figure out where in memory to put the pixel
        for (int y = 0; y < 480; y++)
            for (int x = 0; x < 800; x++) {
                if((x-cx)*(x-cx)+(y-cy)*(y-cy)<1000){
                    *((unsigned char*)(fbp + x+y*800)) = r;
                    *((unsigned char*)(fbp + x+y*800+480*800)) = g;
                    *((unsigned char*)(fbp + x+y*800+2*480*800)) = b;
                }
                else
                if((x-xcoord)*(x-xcoord)+(y-ycoord)*(y-ycoord)<1000){
                    *((unsigned char*)(fbp + x+y*800)) = 0x00;
                    *((unsigned char*)(fbp + x+y*800+480*800)) = 0x00;
                    *((unsigned char*)(fbp + x+y*800+2*480*800)) = 0xff;
                }
                else{
                    *((unsigned char*)(fbp + x+y*800)) = 0x00;
                    *((unsigned char*)(fbp + x+y*800+480*800)) = 0x00;
                    *((unsigned char*)(fbp + x+y*800+2*480*800)) = 0x00;
                }
                    
            }
        if(f1){
         r++;
         if(r==255){
          r=0;
          f1=0;
          f2=1;
         }
        }
        
        if(f2){
         g++;
         if(g==255){
          g=0;
          f2=0;
          f3=1;
         }
        }
        
        if(f3){
         b++;
         if(b==255){
          b=0;
          f3=0;
          f1=1;
         }
        }
    }
    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}

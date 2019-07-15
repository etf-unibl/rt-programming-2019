
/*
 * 
 * Test application
 * 
 * DVGAdriver
 * mtl2 touch screen drivers
 * 
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

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <png.h>
#include <wiringPi.h>

png_uint_32 width;
png_uint_32 height;

png_bytepp rows;
png_bytep row;
int rowbytes;

int cx = 400; 
int cy = 240;

unsigned char r=0;
unsigned char g=0;
unsigned char b=0;
int f1=1;
int f2=0;
int f3=0;

volatile int xcoord=400;
volatile int ycoord=240;
volatile char chg=0;
volatile char c=0;

int fbfd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize = 0;
char *fbp = 0;
long int location = 0;
char* scr =0;

static void fatal_error (const char * message, ...){
    va_list args;
    va_start (args, message);
    vfprintf (stderr, message, args);
    va_end (args);
    exit (EXIT_FAILURE);
}

void readImg(char *name){
    const char * png_file = name;
    png_structp	png_ptr;
    png_infop info_ptr;
    FILE * fp;

    int bit_depth;
    int color_type;
    int interlace_method;
    int compression_method;
    int filter_method;
    int j;
    
    fp = fopen (png_file, "rb");
    if (! fp) {
	fatal_error ("Cannot open '%s': %s\n", png_file, strerror (errno));
    }
    png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (! png_ptr) {
	fatal_error ("Cannot create PNG read structure");
    }
    info_ptr = png_create_info_struct (png_ptr);
    if (! png_ptr) {
	fatal_error ("Cannot create PNG info structure");
    }
    png_init_io (png_ptr, fp);
    png_read_png (png_ptr, info_ptr, 0, 0);
    png_get_IHDR (png_ptr, info_ptr, & width, & height, & bit_depth,
		  & color_type, & interlace_method, & compression_method,
		  & filter_method);
    rows = png_get_rows (png_ptr, info_ptr);
    printf ("Width is %d, height is %d\n", width, height);
    
    rowbytes = png_get_rowbytes (png_ptr, info_ptr);
    printf ("Row bytes = %d\n", rowbytes);
    return 0;
}

char getch(){
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

void* getCoords(void* a){
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
 
void toScreen(void){
    for (int i=0;i<vinfo.yres;i++){
        for (int j=0;j<vinfo.xres;j++) {
            *((unsigned char*)(fbp + j+i*800)) = *((unsigned char*)(scr + j+i*800));
            *((unsigned char*)(fbp + j+i*800+480*800)) = *((unsigned char*)(scr + j+i*800+480*800));
            *((unsigned char*)(fbp + j+i*800+2*480*800)) = *((unsigned char*)(scr + j+i*800+2*480*800));
        }
    }
}
 
void clearScreen(void){
    for (int i=0;i<vinfo.yres;i++){
        for (int j=0;j<vinfo.xres;j++) {
            *((unsigned char*)(scr + j+i*800)) = 0xff;
            *((unsigned char*)(scr + j+i*800+480*800)) = 0xff;
            *((unsigned char*)(scr + j+i*800+2*480*800)) = 0xff;
        }
    }
}
 
void putPicture(int x, int y,int scale){
    int a=0;
    int b=0;
    for(int i=0;i<height;i+=scale){
        row = rows[i];
        for(int j=0;j<1.5*width;j+=scale){
            if((x+j)/scale<vinfo.xres&&(y+i)/scale<vinfo.yres){
                *((unsigned char*)(scr + (a+y+(b+x)*800))) = row[scale*(j)];
                *((unsigned char*)(scr + (a+y+(b+x)*800)+480*800)) = row[scale*(j+1)];
                *((unsigned char*)(scr + (a+y+(b+x)*800)+2*480*800)) = row[scale*(j+2)];
            }        
            a++;
        }
        b++;
        a=0;
    }
}
 
void putCircle(int x,int y,int radius,char r,char g,char b){
    for (int i = 0; i < 480; i++){
            for (int j = 0; j < 800; j++) {
                if((i-y)*(i-y)+(j-x)*(j-x)<radius*radius){
                    *((unsigned char*)(scr + j+i*800)) = r;
                    *((unsigned char*)(scr + j+i*800+480*800)) = g;
                    *((unsigned char*)(scr + j+i*800+2*480*800)) = b;
                }
            }
    }
}
 
char *names[15]={"badApple.png","penguin.png","etf.png","panda.png","planet.png","tank.png"};
 
void loadDifferentPicture(int x,int y){
 static yes=0;
 if(x<40&&y<40){
     static int i=0;
     if(yes==1){
         yes=0;
         i=(i+1)%6;
         readImg(names[i]);
     }
 }
 else yes=1;
}
 
void changeColors(void){
    if(f1){
        r+=10;
        if(r==255){
            r=0;
            f1=0;
            f2=1;
        }
    }

    if(f2){
        g+=10;
        if(g==255){
            g=0;
            f2=0;
            f3=1;
        }
    }

    if(f3){
        b+=10;
        if(b==255){
            b=0;
            f3=0;
            f1=1;
        }
    }
}
 
int main()
{
    pthread_t coords; 
    pthread_create(&coords, NULL, getCoords, NULL);
    pthread_t input; 
    pthread_create(&input, NULL, inputData, NULL);

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
    scr=(char*)malloc(sizeof(char)*screensize);
    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }
    printf("The framebuffer device was mapped to memory successfully.\n");
    
    readImg("penguina.png");
    
    while(c!='q'){
        toScreen();
        
        clearScreen();
        putPicture(50,50,2);
        
        putCircle(xcoord,ycoord,20,r,g,b);
        putCircle(cx,cy,20,g,b,r);
        
        loadDifferentPicture(xcoord,ycoord);
        changeColors();
    }
    munmap(fbp, screensize);
    free(scr);
    close(fbfd);
    return 0;
}

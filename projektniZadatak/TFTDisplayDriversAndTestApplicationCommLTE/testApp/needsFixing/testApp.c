

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>
#define BUF_LEN (640*480)

#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <unistd.h>

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

int main()
{
    int file_desc;
    int ret_val;
    char tmp[3*BUF_LEN];
    
    /* Open DVGAcontrol file. */
    file_desc = open("/dev/fb0", O_RDWR);

    if(file_desc < 0)
    {
        printf("Error, fb0 not opened\n");
        return -1;
    }
    
    int cx=320;
    int cy=240;
    char c=0;
    while((c=getch())!='q'){
        for(int i=0;i<480;i++){
            for(int j=0;j<640;j++){
                
                tmp[i*640+j]=0x00;
                tmp[i*640+j+BUF_LEN]=0x00;
                tmp[i*640+j+2*BUF_LEN]=0x00;
          
            }
        }
        
        for(int i=0;i<480;i++){
            for(int j=0;j<640;j++){
                if((i-cy)*(i-cy)+(j-cx)*(j-cx)>300&&(i-cy)*(i-cy)+(j-cx)*(j-cx)<500){
                    tmp[i*640+j]=0xff;
                    tmp[i*640+j+BUF_LEN]=0xff;
                    tmp[i*640+j+2*BUF_LEN]=0xff;
                }
            }
        }
        if(c=='d') cx=(cx+5)%640;
        if(c=='a') cx-=5;
        if(cx<0) cx=639;
        
        if(c=='s') cy=(cy+5)%480;
        if(c=='w') cy-=5;
        if(cy<0) cx=479;
        
        c=0;
        /* Write to dummy file. */
        ret_val = write(file_desc, tmp, BUF_LEN);
        //delay(200);
    }
    /* Close dummy file. */
    close(file_desc);
    
    return 0;
}

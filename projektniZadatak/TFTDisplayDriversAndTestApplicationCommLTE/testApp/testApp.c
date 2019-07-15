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

int main()
{
    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;
    int x = 0, y = 0;
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

    x = 100; y = 100;       // Where we are going to put the pixel
    
    // Figure out where in memory to put the pixel
    for (y = 0; y < 160; y++)
        for (x = 0; x < 800; x++) {
            *((unsigned char*)(fbp + x+y*800)) = 0x00;
            *((unsigned char*)(fbp + x+y*800+480*800)) = 0xff;
            *((unsigned char*)(fbp + x+y*800+2*480*800)) = 0x00;
        }
    for (y = 160; y < 320; y++)
        for (x = 0; x < 800; x++) {
            *((unsigned char*)(fbp + x+y*800)) = 0x00;
            *((unsigned char*)(fbp + x+y*800+480*800)) = 0x00;
            *((unsigned char*)(fbp + x+y*800+2*480*800)) = 0xff;
        }
    for (y = 320; y < 480; y++)
        for (x = 0; x < 800; x++) {
            *((unsigned char*)(fbp + x+y*800)) = 0xff;
            *((unsigned char*)(fbp + x+y*800+480*800)) = 0xff;
            *((unsigned char*)(fbp + x+y*800+2*480*800)) = 0xff;
        }
    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}

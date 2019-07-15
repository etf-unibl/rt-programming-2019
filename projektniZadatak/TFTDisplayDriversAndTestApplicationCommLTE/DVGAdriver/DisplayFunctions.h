/*
 * Author: Damjan Prerad
 * Date: 6.7.2019.
 * 
 * This driver was made for a college class "Programing in real time" held during the summer semester at the University of Banja Luka, Republic of Srpska, faculty of electrical engineering
 * Asistents: Srdjan Popic
 * Professor: Mladen Knezic
 * 
 */


#ifndef DISPLAYFUNCTIONS_H
#define DISPLAYFUNCTIONS_H

#include"DisplayPinout.h"
#include"GPIOcontrol.h"

/*
 * Display specific constants
 *
 */
#define THPW 20
#define THB26 26
#define THFP 210

#define TVPW 10
#define TVB13 13
#define TVFP 22

#define UPPER_ALIGNMENT 22
#define LOWER_ALIGNMENT 10

#define DISPLAY_WIDTH 800
#define DISPLAY_HEIGHT 480

static int i=0;/*To satisfy the compiler, remove some warnings*/


/*
 *
 *	void initBus(void);
 *
 *	Initializes GPIO pins used to interface the display
 * 
 *	definitions of pins are located in <DisplayPinout.h>
 *
 */

void initBus(void){
    //Some pins are removed due to a lack of GPIO pins on Raspberry
    //Leave the MSBs
    
    //SetGpioPinDirection(R0,GPIO_DIRECTION_OUT);
    //SetGpioPinDirection(R1,GPIO_DIRECTION_OUT);
    //SetGpioPinDirection(R2,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(R3,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(R4,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(R5,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(R6,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(R7,GPIO_DIRECTION_OUT);
    
    //SetGpioPinDirection(G0,GPIO_DIRECTION_OUT);
    //SetGpioPinDirection(G1,GPIO_DIRECTION_OUT);
    //SetGpioPinDirection(G2,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(G3,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(G4,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(G5,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(G6,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(G7,GPIO_DIRECTION_OUT);
    
    //SetGpioPinDirection(B0,GPIO_DIRECTION_OUT);
    //SetGpioPinDirection(B1,GPIO_DIRECTION_OUT);
    //SetGpioPinDirection(B2,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(B3,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(B4,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(B5,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(B6,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(B7,GPIO_DIRECTION_OUT);
    
    SetGpioPinDirection(DCLK,GPIO_DIRECTION_OUT);
    
    SetGpioPinDirection(VSYNC,GPIO_DIRECTION_OUT);
    SetGpioPinDirection(HSYNC,GPIO_DIRECTION_OUT);
    
    //ClearGpioPin(R0);
    //ClearGpioPin(R1);
    //ClearGpioPin(R2);
    ClearGpioPin(R3);
    ClearGpioPin(R4);
    ClearGpioPin(R5);
    ClearGpioPin(R6);
    ClearGpioPin(R7);
    
    //ClearGpioPin(G0);
    //ClearGpioPin(G1);
    //ClearGpioPin(G2);
    ClearGpioPin(G3);
    ClearGpioPin(G4);
    ClearGpioPin(G5);
    ClearGpioPin(G6);
    ClearGpioPin(G7);
    
    //ClearGpioPin(B0);
    //ClearGpioPin(B1);
    //ClearGpioPin(B2);
    ClearGpioPin(B3);
    ClearGpioPin(B4);
    ClearGpioPin(B5);
    ClearGpioPin(B6);
    ClearGpioPin(B7);
    
    ClearGpioPin(DCLK);
    
    ClearGpioPin(HSYNC);
    ClearGpioPin(VSYNC);
}

/*
 * void releaseBus(void);
 * 
 * Releases GPIO pins used to interface the display
 * 
 * definitions of pins are located in <DisplayPinout.h>
 * 
 */

void releaseBus(void){
    //ClearGpioPin(R0);
    //ClearGpioPin(R1);
    //ClearGpioPin(R2);
    ClearGpioPin(R3);
    ClearGpioPin(R4);
    ClearGpioPin(R5);
    ClearGpioPin(R6);
    ClearGpioPin(R7);
    
    //ClearGpioPin(G0);
    //ClearGpioPin(G1);
    //ClearGpioPin(G2);
    ClearGpioPin(G3);
    ClearGpioPin(G4);
    ClearGpioPin(G5);
    ClearGpioPin(G6);
    ClearGpioPin(G7);
    
    //ClearGpioPin(B0);
    //ClearGpioPin(B1);
    //ClearGpioPin(B2);
    ClearGpioPin(B3);
    ClearGpioPin(B4);
    ClearGpioPin(B5);
    ClearGpioPin(B6);
    ClearGpioPin(B7);
    
    ClearGpioPin(DCLK);
    
    ClearGpioPin(HSYNC);
    ClearGpioPin(VSYNC);
    
    //SetGpioPinDirection(R0,GPIO_DIRECTION_IN);
    //SetGpioPinDirection(R1,GPIO_DIRECTION_IN);
    //SetGpioPinDirection(R2,GPIO_DIRECTION_IN);
    SetGpioPinDirection(R3,GPIO_DIRECTION_IN);
    SetGpioPinDirection(R4,GPIO_DIRECTION_IN);
    SetGpioPinDirection(R5,GPIO_DIRECTION_IN);
    SetGpioPinDirection(R6,GPIO_DIRECTION_IN);
    SetGpioPinDirection(R7,GPIO_DIRECTION_IN);
    
    //SetGpioPinDirection(G0,GPIO_DIRECTION_IN);
    //SetGpioPinDirection(G1,GPIO_DIRECTION_IN);
    //SetGpioPinDirection(G2,GPIO_DIRECTION_IN);
    SetGpioPinDirection(G3,GPIO_DIRECTION_IN);
    SetGpioPinDirection(G4,GPIO_DIRECTION_IN);
    SetGpioPinDirection(G5,GPIO_DIRECTION_IN);
    SetGpioPinDirection(G6,GPIO_DIRECTION_IN);
    SetGpioPinDirection(G7,GPIO_DIRECTION_IN);
    
    //SetGpioPinDirection(B0,GPIO_DIRECTION_IN);
    //SetGpioPinDirection(B1,GPIO_DIRECTION_IN);
    //SetGpioPinDirection(B2,GPIO_DIRECTION_IN);
    SetGpioPinDirection(B3,GPIO_DIRECTION_IN);
    SetGpioPinDirection(B4,GPIO_DIRECTION_IN);
    SetGpioPinDirection(B5,GPIO_DIRECTION_IN);
    SetGpioPinDirection(B6,GPIO_DIRECTION_IN);
    SetGpioPinDirection(B7,GPIO_DIRECTION_IN);
    
    SetGpioPinDirection(DCLK,GPIO_DIRECTION_IN);
    
    SetGpioPinDirection(VSYNC,GPIO_DIRECTION_IN);
    SetGpioPinDirection(HSYNC,GPIO_DIRECTION_IN);
}

/*
 * void thb26(void);
 * 
 * Display specific function, as specified in the datasheet
 * 
 */

void thb26(void){//20 na thpw, ukupno 46
 for(i=0;i<THB26;i++){
    ClearGpioPin(DCLK);
    SetGpioPin(DCLK);
 }
}

/*
 * void thpw(void);
 * 
 * Display specific function, as specified in the datasheet
 * 
 */

void thpw(void){
 ClearGpioPin(HSYNC);
 for(i=0;i<THPW;i++){
    ClearGpioPin(DCLK);
    SetGpioPin(DCLK);
 }
 SetGpioPin(HSYNC);
}

/*
 * void thfp(void);
 * 
 * Display specific function, as specified in the datasheet
 * 
 */

void thfp(void){
 for(i=0;i<THFP;i++){
    ClearGpioPin(DCLK);
    SetGpioPin(DCLK);
 }
}

/*
 * void tvpw(void);
 * 
 * Display specific function, as specified in the datasheet
 * 
 */

void tvpw(void){
 ClearGpioPin(VSYNC);
 for(i=0;i<TVPW;i++){
    ClearGpioPin(DCLK);
    SetGpioPin(DCLK);
 }
 SetGpioPin(VSYNC);
}

/*
 * void tvb13(void);
 * 
 * Display specific function, as specified in the datasheet
 * 
 */

void tvb13(void){//10 na tvpw tvb ukupno 23
 for(i=0;i<TVB13;i++){
    ClearGpioPin(DCLK);
    SetGpioPin(DCLK);
 }   
}

/*
 * void tvfp(void);
 * 
 * Display specific function, as specified in the datasheet
 * 
 */

void tvfp(void){
 for(i=0;i<TVFP;i++){
    ClearGpioPin(DCLK);
    SetGpioPin(DCLK);
 }   
}

/*
 * void setBusBit(char busBit,char data);
 * 
 * Helper function - set a bus bit acording to either 0 or 1
 * 
 * busBit - what line to set up
 * 
 * data - to what logic level 0 or 1
 * 
 */

void setBusBit(char busBit,char data){
    if(data==0)
        ClearGpioPin(busBit);
    else SetGpioPin(busBit);
}


/*
 * Leftover function... will be deleted later
 * NOT USED
 */

void setLineV2(void){
    thpw();
    thb26();
    for(i=0;i<DISPLAY_WIDTH;i++){
        ClearGpioPin(DCLK);
        SetGpioPin(DCLK);
    }
    thfp();
}

/*
 * void verticalSyncStart(void);
 * 
 * Begining of the vertical synchronization
 * 
 */

void verticalSyncStart(void){
    tvpw();
    tvb13();
}

/*
 * void verticalSyncStop(void);
 * 
 * End of the vertical synchronization
 * 
 */

void verticalSyncStop(void){
    tvfp();
}

/*
 * void toScreen(void * videomemory);
 * 
 * videomemory - pointer to video memory
 * 
 * Transfers videomemory data to display
 * 
 */

void toScreen(void * videomemory){
    /*
     * R component in the first 800x480
     * G component in the seconds 800x480
     * B component in the third 800x480 
     */
    verticalSyncStart();
    
    int j;
    
    //Alignment lines
    for(j=0;j<UPPER_ALIGNMENT;j++){
        thpw();
        thb26();
        for(i=0;i<DISPLAY_WIDTH;i++){
            ClearGpioPin(DCLK);
            SetGpioPin(DCLK);
        }
        
        thfp();
    }
    //---------------
    
    for(j=0;j<DISPLAY_HEIGHT;j++){
        
        thpw();
        thb26();
        
        for(i=0;i<DISPLAY_WIDTH;i++){
            setBusBit(R3,((char*)videomemory)[i+j*800]&0x08);
            setBusBit(R4,((char*)videomemory)[i+j*800]&0x10);
            setBusBit(R5,((char*)videomemory)[i+j*800]&0x20);
            setBusBit(R6,((char*)videomemory)[i+j*800]&0x40);
            setBusBit(R7,((char*)videomemory)[i+j*800]&0x80);
            
            setBusBit(G3,((char*)videomemory)[i+j*800+480*800]&0x08);
            setBusBit(G4,((char*)videomemory)[i+j*800+480*800]&0x10);
            setBusBit(G5,((char*)videomemory)[i+j*800+480*800]&0x20);
            setBusBit(G6,((char*)videomemory)[i+j*800+480*800]&0x40);
            setBusBit(G7,((char*)videomemory)[i+j*800+480*800]&0x80);
            
            setBusBit(B3,((char*)videomemory)[i+j*800+2*480*800]&0x08);
            setBusBit(B4,((char*)videomemory)[i+j*800+2*480*800]&0x10);
            setBusBit(B5,((char*)videomemory)[i+j*800+2*480*800]&0x20);
            setBusBit(B6,((char*)videomemory)[i+j*800+2*480*800]&0x40);
            setBusBit(B7,((char*)videomemory)[i+j*800+2*480*800]&0x80);
            
            ClearGpioPin(DCLK);
            SetGpioPin(DCLK);
        }
        
        thfp();
    }
    
    //Alignment lines
    for(j=0;j<LOWER_ALIGNMENT;j++){
        thpw();
        thb26();
        for(i=0;i<DISPLAY_WIDTH;i++){
            ClearGpioPin(DCLK);
            SetGpioPin(DCLK);
        }
        
        thfp();
    }
    //---------------
    
    verticalSyncStop();
}

#endif

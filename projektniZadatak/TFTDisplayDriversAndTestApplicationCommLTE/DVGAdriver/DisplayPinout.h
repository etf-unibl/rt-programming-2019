/*
 * Author: Damjan Prerad
 * Date: 6.7.2019.
 * 
 * This driver was made for a college class "Programing in real time" held during the summer semester at the University of Banja Luka, Republic of Srpska, faculty of electrical engineering
 * Asistents: Srdjan Popic
 * Professor: Mladen Knezic
 * 
 */


#ifndef DISPLAYPINOUT_H
#define DISPLAYPINOUT_H

#include "GPIOcontrol.h"

/*
 * Due to a lack of GPIO pins on a raspberry PI it was decided not all 8 bits of data bus will be used.
 * 
 */

/*
 * 
 * Define R bus, 8 bits for red component
 * 
 */

//#define R0 GPIO_02
//#define R1 GPIO_17
//#define R2 GPIO_27
#define R3 GPIO_22
#define R4 GPIO_10
#define R5 GPIO_09
#define R6 GPIO_11
#define R7 GPIO_05//MSB of the red component

/*
 * 
 * Define G bus, 8 bits for green component
 * 
 */

//#define G0 GPIO_10
//#define G1 GPIO_06
#define G2 GPIO_13
#define G3 GPIO_19
#define G4 GPIO_26
#define G5 GPIO_21
#define G6 GPIO_20
#define G7 GPIO_16//MSB of the green component

/*
 * 
 * Define B bus, 8 bits for blue component
 * 
 */

//#define B0 GPIO_18
//#define B1 GPIO_12
//#define B2 GPIO_25
#define B3 GPIO_12
#define B4 GPIO_25
#define B5 GPIO_24
#define B6 GPIO_23
#define B7 GPIO_18//MSB of the blue component

/*
 * Clock pin
 */

#define DCLK GPIO_17

/*
 * Vertical synchronization pin
 */

#define VSYNC GPIO_27

/*
 * Horizontal synchronization pin
 */

#define HSYNC GPIO_06

#endif

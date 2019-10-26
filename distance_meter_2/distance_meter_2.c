/*
 * Distance Meter 2 Device Driver
 *
 * Copyright (c) 2019 Milan Medic
 * 
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/time.h>

#define DEVICE_NAME		"distance_meter_2"
#define DEVICE_ADDRESS	0x08		// Address of ltc2309 chip on cooking-hacks arduPi board

// CHANs and their addresses according to cooking-hacks arduPi board documentation, we will be using CHAN_2
#define CHAN_0 0xDC
#define CHAN_1 0x9C
#define CHAN_2 0xCC
#define CHAN_3 0x8C
#define CHAN_4 0xAC
#define CHAN_1 0xEC
#define CHAN_2 0xBC
#define CHAN_3 0xFC

/*
 * GPIO pins that we will use to make software PWM for Distance Meter 2 sensor. We need to drive sensor with complementary PWM 
 * (when PWMA is high, PWMB has to be low), with frequency 38kHz-42kHz. GPIO pin 18 maps into digital pin 2 on cooking-hacks arduPi board,
 * and GPIO pin 23 maps into digital pin 3.
 */
#define PWMA_PIN 18
#define PWMB_PIN 23
#define HIGH	1
#define LOW		0

// Const values for timers, and treshhold 
#define MIN_SLEEP_TIME		11
#define MAX_SLEEP_TIME		12

#define PWM_TIMER_SEC		0
#define PWM_TIMER_NANO_SEC	100*1000*1000	//100ms

#define TRESHHOLD 1000				// ltc2309 ADC chip uses 5V as Vcc, thus 1000/4095 *5 gives us value of 1.2V what is value above treshhold voltage (1.1 V)



struct distance_meter_2_data {
    struct i2c_client *client;
};

typedef struct {
    ktime_t start_time;
    ktime_t stop_time;
	spinlock_t lock;
} timer_struct;

/* Timer vars. */
static struct hrtimer pwm_timer;
static ktime_t pwm_period;
timer_struct measure_time;

uint64_t last_value=0;




// Timer that will preform software pwm every pwm_period seconds 
static enum hrtimer_restart pwm_timer_callback(struct hrtimer *param)
{

    uint16_t i;
	
	spin_lock(&measure_time.lock);
    measure_time.start_time= ktime_get();
	spin_unlock(&measure_time.lock);
	
	//TODO: Invoke measuring thread or timer 
	
    for(i=0;i<400;i++) {	// This is PWM burst that lasts around 10ms with frequency between 38kHz-42kHz
        gpio_set_value(PWMA_PIN,HIGH);
		gpio_set_value(PWMB_PIN,LOW);
		udelay(MIN_SLEEP_TIME);
		gpio_set_value(PWMA_PIN,LOW);
		gpio_set_value(PWMB_PIN,HIGH);
		udelay(MAX_SLEEP_TIME);
	}
	
	// After PWM we need to set everything to LOW 
	gpio_set_value(PWMA_PIN,LOW);
	gpio_set_value(PWMB_PIN,LOW);
    
    
    hrtimer_forward(&pwm_timer, ktime_get(), pwm_period);
    
    return HRTIMER_RESTART;
}




/*
 * TODO: napraviti strukturu u kojoj cu da imam timeval kao i spinlock-ove, funkciju za kthread koja ce stalno da provjerava stanje analognog ulaza preko i2c-a 
 * zatim imati u16 vrijednost za taj read, taj nekakav value, takodje imati i definisan nekakav prag (najbolje eksperimentalno, ali mislim da je 1/3 od 4095 koliko mogu dobiti jako dobra)
 * i ukoliko se desi da je ocitana veca od toga, radi se timestamp, thread se uspavljuje na burst milisekundi a vrijednost se prebacuje u userspace preko ioctl. Napisati i kthread funkciju
 * koja radi pwm, softverski pwm svako pola sekunde otprilike, trajanja 10-ak ms. Registrovati i GPIO-o pinove, koji su mapirani na plocici u digitalne pinove, dakle mogu 23 i 8 koji su digitalni 2 i 3 sa plocice
 */

/*
 * Function used to get voltage value and determine if we got value above treshhold
 */
static uint8_t get_value(uint16_t data)
{	
	uint16_t value =0;
	value += (data & 0x00FF)<<4;
	value += (data & 0xFF00)>>12;
	printk(KERN_INFO "Vrijednost iznosi %d \n", value);
	if(value>TRESHHOLD) {
		spin_lock(&measure_time.lock);
		measure_time.stop_time= ktime_get();
		spin_unlock(&measure_time.lock);
		printk(KERN_INFO "Napon koji se dobije iznosi: %04f \n",(value/4095.0)*5);
		return 1;
	}
	
	return 0;
}
/*
static int thread_function(void* args) {
	
	ktime_t start=ktime_get();
	struct timespec ts_start=ktime_to_timespec(start);
	uint16_t data1;
	
	while((ktime_to_timespec(ktime_get())).tv_sec > ts_start.tv_sec+20) {
		data1=i2c_smbus_read_word_data(data.client, CHAN_2);
		if(get_value(data)) {
			spin_lock(&measure_time.lock);
			last_value= ((measure_time.stop_time.tv_sec - measure_time.start_time.tv_sec) / 1000000 + (measure_time.stop_time.tv_nsec - measure_time.start_time.tv_nsec) * 1000) / 2; //We have to devide value cause it travels both ways 
			spin_unlock(&measure_time.lock);
			printk(KERN_INFO "Udaljenost iznosi: %04f",last_value*0.034);
		}
		else {
			usleep_range(25,30);
		}
	}
}*/

static const struct i2c_device_id distance_meter_2_id[] = {
    {DEVICE_NAME, DEVICE_ADDRESS},
    {},
};
MODULE_DEVICE_TABLE(i2c, distance_meter_2_id);

static const struct i2c_board_info distance_meter_2_board_info = {
	I2C_BOARD_INFO(DEVICE_NAME, DEVICE_ADDRESS)
};

struct distance_meter_2_data data;



static int distance_meter_2_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    
	
	if(gpio_request(PWMA_PIN, NULL)) {
		dev_err(&client->dev, "GPIO request failure.");
		return -1;
	}
	
	if(gpio_request(PWMB_PIN, NULL)) {
		dev_err(&client->dev, "GPIO request failure.");
		return -1;
	}
	
	hrtimer_init(&pwm_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    pwm_period = ktime_set(PWM_TIMER_SEC, PWM_TIMER_NANO_SEC);
    pwm_timer.function = &pwm_timer_callback;
    hrtimer_start(&pwm_timer, pwm_period, HRTIMER_MODE_REL);
	
	//TODO: Ovdje ubaciti i thread init
	/*
	if(!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c_check_functionality error\n");
		return -ENXIO;
    }	
	*/
}

static struct i2c_driver distance_meter_2_driver = {
    .driver = {
		.name	= DEVICE_NAME,
		.owner	= THIS_MODULE,
    },
    .probe    = distance_meter_2_probe,
    .id_table = distance_meter_2_id
};

static int __init distance_meter_2_init(void)
{
	printk(KERN_CONT "Inserting module.");
	
	struct i2c_adapter *i2c_adapt;
	i2c_adapt = i2c_get_adapter(1); // i2c-1
	data.client = i2c_new_device(i2c_adapt, &distance_meter_2_board_info);
	
	i2c_add_driver(&distance_meter_2_driver);
	
	return 0;
}

static void __exit distance_meter_2_exit(void)
{
	printk(KERN_CONT "Removing module.");
	
	hrtimer_cancel(&pwm_timer);
	
	//TODO: Ovdje ubaciti thread release
	
    gpio_free(PWMA_PIN);
    gpio_free(PWMB_PIN);
	i2c_unregister_device(data.client);
	i2c_del_driver(&distance_meter_2_driver);
}

module_init(distance_meter_2_init);
module_exit(distance_meter_2_exit);
MODULE_AUTHOR("Milan Medic");
MODULE_LICENSE("Dual BSD/GPL");


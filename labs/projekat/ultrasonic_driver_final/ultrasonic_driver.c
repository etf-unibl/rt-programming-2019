/*
 *	Ultrasonic Ranging Module HC - SR04 Linux Device Driver
 *	
 *	Copyright (c) 2019 Milan Medic
 *  
 *	This program is free software; you can redistribute it and/or modify
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
#include <linux/spinlock.h>
#include <linux/time.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>


/* Timer interval defined as (TIMER_SEC + TIMER_NANO_SEC). */
#define TIMER_SEC    	0
#define TIMER_NANO_SEC  100*1000*1000 /* 100ms */

/*	These are GPIO pins that will be used for this driver */ 
#define TRIGGER_PIN		4
#define ECHO_PIN		3

/*	Struct for storing start/stop times and spinlock to avoid race conditions */
typedef struct {
	ktime_t start_time;
	ktime_t stop_time;
	uint64_t passed_time;
	spinlock_t lock;
} time_struct;

/*	Declaration of ioctl macros */
#define DISTANCE_READ _IOR('q',1,uint64_t*)
#define STOP_IN_BACKGROUND _IOW('q',2,int8_t*)

/*	Declaration of ultrasonic_driver.c functions */
int ultrasonic_driver_init(void);
void ultrasonic_driver_exit(void);
static int ultrasonic_driver_open(struct inode *, struct file *);
static int ultrasonic_driver_release(struct inode *, struct file *);
static ssize_t ultrasonic_driver_read(struct file *, char *buf, size_t , loff_t *);
static ssize_t ultrasonic_driver_write(struct file *, const char *buf, size_t , loff_t *);
static long ultrasonic_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
void calculate_distance(void);

/* Structure that declares the usual file access functions. */
struct file_operations ultrasonic_driver_fops =
{
    open    :   ultrasonic_driver_open,
    release :   ultrasonic_driver_release,
    read    :   ultrasonic_driver_read,
    write   :   ultrasonic_driver_write,
	unlocked_ioctl : ultrasonic_ioctl 
};

/* Declaration of the init and exit functions. */
module_init(ultrasonic_driver_init);
module_exit(ultrasonic_driver_exit);

/* Global variables of the driver */

/* Major number. */
int ultrasonic_driver_major;

/* Measure timer vars. */
static struct hrtimer measure_timer;
static ktime_t period;

/* Struct used for time measurement */
static time_struct ts;

/* IRQ number. */
static int irq_gpio3 = -1;

/* Thread interrupt routine which contains busy wait loop to measure time echo pin spends in high state */
static irqreturn_t h_irq_gpio3_th(int irq, void *data) {
	
	unsigned long time_in_us; // For debugging purpouses only 
	struct timespec start,stop;
	
	printk(KERN_INFO "Inside interrupt thread routine!\n"); // Just for debugging
	
	while(gpio_get_value(ECHO_PIN)==1);
	
	spin_lock(&ts.lock);
    ts.stop_time = ktime_get();
	start = ktime_to_timespec(ts.start_time);
	stop = ktime_to_timespec(ts.stop_time);
	ts.passed_time = (stop.tv_sec - start.tv_sec)*1000000 + (stop.tv_nsec - start.tv_nsec)/1000;
	spin_unlock(&ts.lock);
	
	time_in_us=ts.passed_time;
	printk(KERN_INFO "Microseconds that have passed: %lu\n", time_in_us);
	
	return IRQ_HANDLED;
		
}

/*	Interrupt routine that makes timestamp on start of pulse and calls thread interrupt routine */
static irqreturn_t h_irq_gpio3(int irq, void *data) {
	
	printk(KERN_INFO "Inside interrupt routine!\n"); // Just for debugging
	
	ts.start_time = ktime_get(); // There is no need for spinlock in interrupt 
	
	return IRQ_WAKE_THREAD;
}

/* 
 *	Timer callback function is called every time timer expires, it generates
 *	pulse of 10us duration, and prints it into kernel log (for debugging purpouses)
 */  
static enum hrtimer_restart measure_timer_callback(struct hrtimer *param) {
    
	gpio_set_value(TRIGGER_PIN,0);
	udelay(5);
    
	gpio_set_value(TRIGGER_PIN,1);
	udelay(10);
	gpio_set_value(TRIGGER_PIN,0);
    
    printk(KERN_INFO "Pulse has been generated\n");

    hrtimer_forward(&measure_timer, ktime_get(), period);

    return HRTIMER_RESTART;
}

int32_t ultrasonic_driver_init(void) {
	
    int32_t result = -1;

    printk(KERN_INFO "Inserting ultrasonic_driver module\n");

    /* Registering device. */
    result = register_chrdev(0, "ultrasonic_driver", &ultrasonic_driver_fops);
    if (result < 0)
    {
        printk(KERN_INFO "ultrasonic_driver: cannot obtain major number %d\n", ultrasonic_driver_major);
        return result;
    }

    ultrasonic_driver_major = result;
    printk(KERN_INFO "ultrasonic_driver major number is %d\n", ultrasonic_driver_major);

	/*	Setting up trigger pin */
	result = gpio_request(TRIGGER_PIN,NULL);
	
    if(result != 0)
    {
        printk("Error: GPIO request failed!\n");
        goto fail_no_mem;
    }

    gpio_direction_output(TRIGGER_PIN, 0);
	
	
    /*	Setting up echo pin	*/
	result = gpio_request_one(ECHO_PIN, GPIOF_IN, "irq_gpio3");
	if(result != 0)
    {
        printk("Error: GPIO request failed!\n");
        goto fail_gpio_request;
    }
	
	irq_gpio3 = gpio_to_irq(ECHO_PIN);
	
	result = request_threaded_irq(irq_gpio3, h_irq_gpio3, h_irq_gpio3_th,
      IRQF_TRIGGER_RISING, "irq_gpio3", (void *)(h_irq_gpio3));
	
	if(result != 0)
    {
        printk("Error: ISR not registered!\n");
        goto fail_gpio_request;
    }
	
		
	/*	Setting up timer */
    hrtimer_init(&measure_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    period = ktime_set(TIMER_SEC, TIMER_NANO_SEC);
    measure_timer.function = &measure_timer_callback;
	
    return 0;
fail_gpio_request:
	/* Freeing trigger pin */
	gpio_set_value(TRIGGER_PIN,0);
    gpio_direction_input(TRIGGER_PIN);
	gpio_free(TRIGGER_PIN);
	
fail_no_mem:
    /* Freeing the major number. */
    unregister_chrdev(ultrasonic_driver_major, "ultrasonic_driver");

    return result;
}

void ultrasonic_driver_exit(void) {
	
    printk(KERN_INFO "Removing ultrasonic_driver module\n");
	
    /* Release high resolution timer. */
    hrtimer_cancel(&measure_timer);
	
	/* Disable IRQ and free GPIO pin */
	disable_irq(irq_gpio3);
    free_irq(irq_gpio3, h_irq_gpio3);
    gpio_free(ECHO_PIN);

    /* Clear Trigger GPIO pin. */
    gpio_set_value(TRIGGER_PIN,0);
    gpio_direction_input(TRIGGER_PIN);
	gpio_free(TRIGGER_PIN);
    
    /* Freeing the major number. */
    unregister_chrdev(ultrasonic_driver_major, "ultrasonic_driver");

}

/*	In open function timer will start */
static int32_t ultrasonic_driver_open(struct inode *inode, struct file *filp) {
    if(hrtimer_active(&measure_timer) == 0) {
		hrtimer_start(&measure_timer, period, HRTIMER_MODE_REL);
		printk(KERN_INFO "Timer has been activated again.\n");
	}
	return 0;
}

static int32_t ultrasonic_driver_release(struct inode *inode, struct file *filp) {
	
    return 0;
}

static ssize_t ultrasonic_driver_read(struct file *filp, char *buf, size_t len, loff_t *f_pos) {
	
	return 0;
}

/*	ioctl function is used to send current distance to userspace */
static long ultrasonic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd) {
		case DISTANCE_READ:
			if(hrtimer_active(&measure_timer) == 0) { // If timer was canceled, it starts again
				hrtimer_start(&measure_timer, period, HRTIMER_MODE_REL);
				printk(KERN_INFO "Timer has been activated again.\n");
			}
			spin_lock(&ts.lock);
			if(copy_to_user((uint64_t*) arg, &ts.passed_time, sizeof(ts.passed_time)) != 0) { // To eliminate compile-time warning 
				printk(KERN_INFO " Problem with copying data to user!\n");
				return EAGAIN;
			}
			spin_unlock(&ts.lock);
			break;
		/*	This ioctl call will be used when exiting userspace app to stop timer in background and save resources */
		case STOP_IN_BACKGROUND:
			if(hrtimer_active(&measure_timer) > 0) {
				hrtimer_cancel(&measure_timer);
				printk(KERN_INFO "Timer has been stopped.\n");
			}
			break;
	}
	return 0;
}

static ssize_t ultrasonic_driver_write(struct file *filp, const char *buf, size_t len, loff_t *f_pos)
{
	return 0;
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Milan Medic");

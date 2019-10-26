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
/* dodajemo header za ioctl */
#include <linux/ioctl.h> 
/*Dodajemo dva nova header-a*/
#include <linux/spinlock.h>
#include <linux/time.h>

MODULE_LICENSE("Dual BSD/GPL");

/* if defined, timer callback will implement LED0 flashing and
   SW0 reading after each interval */
#define TEST

// timer interval defined as (TIMER_SEC + TIMER_NANO_SEC)
#define TIMER_SEC    0
#define TIMER_NANO_SEC  10*1000*1000 /* 10ms */


/*Enum koji koristim u strukturi */
typedef enum {STOPPED = 0, PAUSED = 1, COUNTING = 2, DELETED=3} STATUS;


typedef struct {
    ktime_t value;
    spinlock_t lock;
    STATUS state; /*U zavisnosti od enuma tajmer je ili pauziran ili stopiran*/
    uint8_t name[20]; /*char* koji predstavlja ime*/
} timer_struct;

/*Definisemo koje su moguce komande zadate prilikom ioctl, a to su read i write, argument koji se prosljedjuje je ime tajmera koji se prati*/

#define TIMER_START _IOW('q',1,char*)
#define TIMER_PAUSE _IOW('q',2,char*)
#define TIMER_READ _IOR('q',3,char*)
#define TIMER_STOP _IOW('q',4,char*)
#define TIMER_STOP_ALL _IOW('q',5,char*)
#define TIMER_DELETE _IOW('q',6,char*)
#define TIMER_DELETE_ALL _IOW('q',7,char*)

/* Declaration of timer_driver.c functions */
int timer_driver_init(void);
void timer_driver_exit(void);
static int timer_driver_open(struct inode *, struct file *);
static int timer_driver_release(struct inode *, struct file *);
static ssize_t timer_driver_read(struct file *, char *buf, size_t , loff_t *);
static ssize_t timer_driver_write(struct file *, const char *buf, size_t , loff_t *);

static long timer_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/* Structure that declares the usual file access functions. */
struct file_operations timer_driver_fops =
{
    open    :   timer_driver_open,
    release :   timer_driver_release,
    read    :   timer_driver_read,
    write   :   timer_driver_write,
	unlocked_ioctl : timer_ioctl //dodao ovo
};

/* Declaration of the init and exit functions. */
module_init(timer_driver_init);
module_exit(timer_driver_exit);

/* Global variables of the driver */

/* Major number. */
int timer_driver_major;

/* Buffer to store data. */
//#define BUF_LEN 80
//char* timer_driver_buffer;

/* Timer vars. */
static struct hrtimer main_timer;
static ktime_t resolution;
/* Napraviti define za ovu 5 */
timer_struct timers[5];

/*Jednostavniji mehanizam za ocuvanje broja tajmera*/
static spinlock_t in_use_lock;
uint8_t in_use=0;


/* timer callback function called each time the timer expires
   flashes the LED0, reads the SW0 and prints its value to kernel log */
  
static enum hrtimer_restart main_timer_callback(struct hrtimer *param)
{
#ifdef TEST
    uint8_t i;
    for(i=0;i<5;i++) {
        if(timers[i].state==COUNTING){
            spin_lock(&timers[i].lock);
            timers[i].value=ktime_add_safe(timers[i].value,resolution);
            spin_unlock(&timers[i].lock);
		}
    }
    
#endif
    
    hrtimer_forward(&main_timer, ktime_get(), resolution);
    
    return HRTIMER_RESTART;
}

/*
 * U ovoj funkciji provjeravam da li postoji dati tajmer, odnosno koji je njegov status.
 * Mogao sam je odraditi i tako da ne postavljam spinlock sto bi omogucilo citanje tajmera 
 * koji je mozda zaspao u trenutku brisanja prije nego sto je promjenjeno stanje na njemu.
 * Ja sam se odlucio za ovu verziju gdje postavim spinlock na svaki tajmer prije nego mu provjerim
 * stanje jer mi je logicno ako izdam 10 komandi da se one izvrsavaju u redosljedu u kojem su izdane.
 */
int8_t exists(uint8_t* name) {
    
    uint8_t i;
    for(i=0;i<5;i++) {
        spin_lock(&timers[i].lock);
        if(timers[i].state!=DELETED) {
            if(strcmp(name,timers[i].name)==0) {
                spin_unlock(&timers[i].lock);
                return i;
            }
        }
        spin_unlock(&timers[i].lock);
    }
    return -1;
}

/*
 * Initialization:
 *  1. Register device driver
 *  2. Allocate buffer
 *  3. Initialize buffer
 *  4. Map GPIO Physical address space to virtual address
 *  5. Initialize GPIO pins
 *  6. Init and start the high resoultion timer
 */
int timer_driver_init(void)
{
    int32_t result = -1;

    printk(KERN_INFO "Inserting timer_driver module\n");

    /* Registering device. */
    result = register_chrdev(0, "timer_driver", &timer_driver_fops);
    if (result < 0)
    {
        printk(KERN_INFO "timer_driver: cannot obtain major number %d\n", timer_driver_major);
        return result;
    }

    timer_driver_major = result;
    printk(KERN_INFO "timer_driver major number is %d\n", timer_driver_major);

   
    /* Ovdje vrsim inicijalizaciju tajmera, postavljam vrijednosti ktime-a na 0, 
     * u pocetku su svi tajmeri obrisani i inicijalizujem spinlock za svaki tajmer.
     */
    
    uint8_t i;
    for(i=0;i<5;i++){
        timers[i].value=ktime_set(0,0);
        timers[i].state=DELETED;
        spin_lock_init(&timers[i].lock);
    }

    /* Inicijalizujem high resolution tajmer. */
    
    hrtimer_init(&main_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    resolution = ktime_set(TIMER_SEC, TIMER_NANO_SEC);
    main_timer.function = &main_timer_callback;
    hrtimer_start(&main_timer, resolution, HRTIMER_MODE_REL);

    return 0;
}

/*
 * Cleanup:
 *  1. stop the timer
 *  2. release GPIO pins (clear all outputs, set all as inputs and pull-none to minimize the power consumption)
 *  3. Unmap GPIO Physical address space from virtual address
 *  4. Free buffer
 *  5. Unregister device driver
 */
void timer_driver_exit(void)
{
    printk(KERN_INFO "Removing timer_driver module\n");

    /* Release high resolution timer. */
    hrtimer_cancel(&main_timer);


    /* Freeing the major number. */
    unregister_chrdev(timer_driver_major, "timer_driver");
}

/* File open function. */
static int timer_driver_open(struct inode *inode, struct file *filp)
{
    /* Initialize driver variables here. */

    /* Reset the device here. */

    /* Success. */
    return 0;
}

/* File close function. */
static int timer_driver_release(struct inode *inode, struct file *filp)
{
    /* Success. */
    return 0;
}

/*
 * File read function
 *  Parameters:
 *   filp  - a type file structure;
 *   buf   - a buffer, from which the user space function (fread) will read;
 *   len - a counter with the number of bytes to transfer, which has the same
 *           value as the usual counter in the user space function (fread);
 *   f_pos - a position of where to start reading the file;
 *  Operation:
 *   The gpio_driver_read function transfers data from the driver buffer (gpio_driver_buffer)
 *   to user space with the function copy_to_user.
 */
static ssize_t timer_driver_read(struct file *filp, char *buf, size_t len, loff_t *f_pos)
{
    return 0;
}


static long timer_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	uint8_t name[20]; /* Staticki sam alocirao niz za ime da bude 20 karaktera */
    strcpy(name,(uint8_t*)arg); 
    int8_t id;
    uint8_t b;
    struct timespec ts;
    switch(cmd) {
        case TIMER_START:
            
			/* Trazimo da li vec postoji dati tajmer, ako postoji provjeravamo da li je pokrenut
             * ako nije ponovo ga startamo i ispisujemo odgovarajucu poruku u kernel log a ako jeste
             * ispisujemo odgovarajucu poruku u kernel log i ne diramo tajmer.*/
            
            if( (id=exists(name)) >= 0) {
                spin_lock(&timers[id].lock);
                if( timers[id].state == COUNTING) { 
                    printk(KERN_INFO "Timer %s(%d) is allready running\n",timers[id].name,id);                                                              
                    spin_unlock(&timers[id].lock);
                    break;                                                                      
                }
                else if( timers[id].state == PAUSED) {
                    timers[id].state=COUNTING;
                    ts=ktime_to_timespec(timers[id].value);
                    printk(KERN_INFO "Timer %s (%d) has started counting again,from value %ld:%02ld\n", 
                    timers[id].name,id,ts.tv_sec,ts.tv_nsec/10000000);
                    spin_unlock(&timers[id].lock);
                    break;
                }
                else {
                    timers[id].state=COUNTING;
                    printk(KERN_INFO "Timer %s (%d) has started counting from zero\n",timers[id].name,id);
                    spin_unlock(&timers[id].lock);
                    break;
                }
            }
            /*Ako ne postoji, onda prvo provjerimo da li smo dostigli maksimum dozvoljenih tajmera, u nasem slucaju 5 
             *ako jesmo javljamo error u kernel info i napustamo ako nismo startamo novi tajmer.*/
            
            else {
                spin_lock(&in_use_lock);
                if( in_use > 4) {
                    printk(KERN_INFO "We have reached maximum number of wanted timers!\n");
                    spin_unlock(&in_use_lock);
                    break;
                }
                in_use++;
                spin_unlock(&in_use_lock);
                /*Sigurno ima bar 1 tajmer koji je deleted i moze da se koristi*/
                b=0;
                id=0;
                while( b == 0) {
                    spin_lock(&timers[id].lock);
                    if(timers[id].state==DELETED) {
                        strcpy(timers[id].name,name);
                        timers[id].state=COUNTING;
                        printk(KERN_INFO "Timer %s (%d) has started counting from zero\n",timers[id].name,id);
                        b=1;
                    }
                    spin_unlock(&timers[id].lock);
                    id++;
                }
                break;
            }
            
        case TIMER_PAUSE:
			/*Provjeravamo da li postoji tajmer, zatim ako postoji postavljamo mu status na PAUSED*/
            if( (id=exists(name)) >= 0)
            {
                spin_lock(&timers[id].lock);
                if( timers[id].state == PAUSED) {
                    printk(KERN_INFO "Timer %s (%d) has already been paused.\n",timers[id].name,id);
                    spin_unlock(&timers[id].lock);
                    break;
                }
                else if ( timers[id].state == STOPPED) {
                    printk(KERN_INFO "Timer %s (%d) is stopped, you cannot pause stopped timer.\n",timers[id].name,id);
                    spin_unlock(&timers[id].lock);
                    break;
                }
                else {
                    timers[id].state=PAUSED;
                    printk(KERN_INFO "Timer %s (%d) is paused.\n",timers[id].name,id);
                    spin_unlock(&timers[id].lock);
                    break;
                }
            }
            printk(KERN_INFO "There is no timer %s \n",name);
            break;
            
        case TIMER_READ:
            /*Prvo provjeravamo da li je dati tajmer postoji, ako postoji ocitavamo vrijeme, ako ne postoji saljemo error*/
            if( (id=exists(name)) >= 0)
            {
                /* Lock kod read-a je mozda i suvisan, ali posto prilikom dodavanja vremena koristimo lock zelio sam da i vrijeme citanja
                 * bude ono koje je bilo u datom trenutku poziva instrukcije. */
                spin_lock(&timers[id].lock);
                ts=ktime_to_timespec(timers[id].value);
                printk(KERN_INFO "Current time on timer %s (%d) is %ld:%02ld\n",timers[id].name,id,ts.tv_sec,ts.tv_nsec/10000000); /* ovdje sam dijelio sa 10 000 000 */
                spin_unlock(&timers[id].lock);
                break;
            }
            printk(KERN_INFO "There is no timer %s \n",name);
            break;
            
        case TIMER_STOP:
            /*Prvo provjeravamo da li je dati tajmer postoji, ako postoji zaustavljamo ga i postavljamo vrijeme na 0, ako ne postoji saljemo error*/                        
            if( (id=exists(name)) >= 0)
            {
                spin_lock(&timers[id].lock);
                timers[id].value=ktime_set(0,0);
                if( timers[id].state == PAUSED) {
                    timers[id].state=STOPPED;
                    printk(KERN_INFO "Timer %s (%d) was paused and now is stopped.\n",timers[id].name,id);
                    spin_unlock(&timers[id].lock);
                    break;
                }
                else if ( timers[id].state == STOPPED) {
                    printk(KERN_INFO "Timer %s (%d) has allready been stopped, you cannot stop it twice.\n",timers[id].name,id);
                    spin_unlock(&timers[id].lock);
                    break;
                }
                else {
                    timers[id].state=STOPPED;
                    printk(KERN_INFO "Timer %s (%d) in now stopped.\n",timers[id].name,id);
                    spin_unlock(&timers[id].lock);
                    break;
                }
            }
            printk(KERN_INFO "There is no timer %s \n",name);
            break;
            
        case TIMER_STOP_ALL:
            /*Proci cemo kroz petlju i u petlji postaviti sve tajmere na stop satus*/
            for( id=0; id<5; id++) {
                if( timers[id].state != DELETED) {
                    spin_lock(&timers[id].lock);
                    timers[id].value=ktime_set(0,0);
                    timers[id].state=STOPPED;
                    spin_unlock(&timers[id].lock);
                }
            }
            printk(KERN_INFO "All timers have been stopped.\n");
            break;
            
        case TIMER_DELETE:
            /*Trazimo dati tajmer, ako postoji smanjujemo in_use parametar, ispisujemo koji smo tajmer obrisali i postavljamo vrijednost na DELETED u 
             *strukturi timer_struct i u imenu setujemo sve 0*/
            if( (id=exists(name)) >= 0)
            {
                spin_lock(&timers[id].lock);
                spin_lock(&in_use_lock);
                in_use--;
                spin_unlock(&in_use_lock);
                printk(KERN_INFO "Timer %s has been deleted.\n",timers[id].name);
                for( b=0; b<20; b++)
                    timers[id].name[b]=0;
                timers[id].state=DELETED;
                spin_unlock(&timers[id].lock);
                break;
            }
            printk(KERN_INFO "There is no timer %s \n",name);
            break;
            
        case TIMER_DELETE_ALL:
            /*Prolazimo kroz petlju i svi tajmeri koji nisu vec obrisani se brisu analogno prethodnoj funkciji*/
            spin_lock(&in_use_lock);
            in_use=0;
            spin_unlock(&in_use_lock);
            for( id=0; id<5; id++) {
                if( timers[id].state != DELETED) {
                    spin_lock(&timers[id].lock);
                    /* Resetujem i imena tj. postavljam na prazan string,jer mi to koristi funkcija exists */
                    for( b=0; b<20; b++)
                        timers[id].name[b]=0;
                    timers[id].state=DELETED;
                    spin_unlock(&timers[id].lock);
                }
            }
            printk(KERN_INFO "All timers have been deleted.\n");
            break;
        default:
            return EINVAL;
        }
}

/*
 * File write function
 *  Parameters:
 *   filp  - a type file structure;
 *   buf   - a buffer in which the user space function (fwrite) will write;
 *   len - a counter with the number of bytes to transfer, which has the same
 *           values as the usual counter in the user space function (fwrite);
 *   f_pos - a position of where to start writing in the file;
 *  Operation:
 *   The function copy_from_user transfers the data from user space to kernel space.
 */
static ssize_t timer_driver_write(struct file *filp, const char *buf, size_t len, loff_t *f_pos)
{
    
        return -EFAULT;
   
}

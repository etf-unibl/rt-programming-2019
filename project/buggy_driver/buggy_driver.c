#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/ioctl.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/spinlock.h>

MODULE_LICENSE("Dual BSD/GPL");

// NOTE: Check Broadcom BCM8325 datasheet, page 91+
// NOTE: GPIO Base address is set to 0x7E200000,
//       but it is VC CPU BUS address, while the
//       ARM physical address is 0x3F200000, what
//       can be seen in pages 5-7 of Broadcom
//       BCM8325 datasheet, having in mind that
//       total system ram is 0x3F000000 (1GB - 16MB)
//       instead of 0x20000000 (512 MB)

//////////////////////////////////////////////////////////////////////////////////////////

/* GPIO registers base address. */
#define BCM2708_PERI_BASE   (0x3F000000)
#define GPIO_BASE           (BCM2708_PERI_BASE + 0x200000)

/* GPIO registers */
volatile struct S_GPIO_REGS
{
    uint32_t GPFSEL[6]; // GPIO Function Select registers
    uint32_t Reserved0; // Reserved Byte
    uint32_t GPSET[2]; // GPIO Output Pin Set registers
    uint32_t Reserved1; // Reserved Byte
    uint32_t GPCLR[2]; // GPIO Output Pin Clear registers
    uint32_t Reserved2; // Reserved Byte
    uint32_t GPLEV[2]; // GPIO Pin Level registers
    uint32_t Reserved3; // Reserved Byte
    uint32_t GPEDS[2]; // GPIO Pin Event Detect Status registers
    uint32_t Reserved4; // Reserved Byte
    uint32_t GPREN[2]; // GPIO Pin Rising Edge Detect Enable registers
    uint32_t Reserved5; // Reserved Byte
    uint32_t GPFEN[2]; // GPIO Pin Falling Edge Detect Enable registers
    uint32_t Reserved6; // Reserved Byte
    uint32_t GPHEN[2]; // GPIO Pin High Detect Enable registers
    uint32_t Reserved7; // Reserved Byte
    uint32_t GPLEN[2]; // GPIO Pin Low Detect Enable registers
    uint32_t Reserved8; // Reserved Byte
    uint32_t GPAREN[2]; // GPIO Pin Asynchronous Rising Edge Detect registers
    uint32_t Reserved9; // Reserved Byte
    uint32_t GPAFEN[2]; // GPIO Pin Asynchronous Falling Edge Detect registers
    uint32_t Reserved10; // Reserved Byte
    uint32_t GPPUD; // GPIO Pin Pull-up/down Enable Register
    uint32_t GPPUDCLK[2]; // GPIO Pin Pull-up/down Enable Clock Register
    uint32_t Reserved11[4]; // Reserved Bytes
} *gpio_regs;

/* GPIO pins available on connector p1 */
typedef enum {GPIO_02 = 2,
              GPIO_03 = 3,
              GPIO_04 = 4,
              GPIO_05 = 5,
              GPIO_06 = 6,
              GPIO_07 = 7,
              GPIO_08 = 8,
              GPIO_09 = 9,
              GPIO_10 = 10,
              GPIO_11 = 11,
              GPIO_12 = 12,
              GPIO_13 = 13,
              GPIO_14 = 14,
              GPIO_15 = 15,
              GPIO_16 = 16,
              GPIO_17 = 17,
              GPIO_18 = 18,
              GPIO_19 = 19,
              GPIO_20 = 20,
              GPIO_21 = 21,
              GPIO_22 = 22,
              GPIO_23 = 23,
              GPIO_24 = 24,
              GPIO_25 = 25,
              GPIO_26 = 26,
              GPIO_27 = 27} GPIO;
		
/* GPIO Pin Pull-up/down */
typedef enum {PULL_NONE = 0,
              PULL_DOWN = 1,
              PULL_UP = 2} PUD;

/* GPIO Pin Alternative Function selection */
// By default GPIO pin is being used as an INPUT
typedef enum {GPIO_INPUT     = 0b000,
              GPIO_OUTPUT    = 0b001,
              GPIO_ALT_FUNC0 = 0b100,
              GPIO_ALT_FUNC1 = 0b101,
              GPIO_ALT_FUNC2 = 0b110,
              GPIO_ALT_FUNC3 = 0b111,
              GPIO_ALT_FUNC4 = 0b011,
              GPIO_ALT_FUNC5 = 0b010,
			  UNDEFINED      = 8} FSEL;

/*
 * SetGPIOFunction function
 *  Parameters:
 *   pin   - number of GPIO pin;
 *
 *   code  - alternate function code to which the GPIO pin is to be set
 *  Operation:
 *   Based on the specified GPIO pin number and function code, sets the GPIO pin to
 *   operate in the desired function. Each of the GPIO pins has at least two alternative functions.
 */
void SetGPIOFunction(GPIO pin, FSEL code)
{
    uint8_t regIndex = pin / 10;
    uint8_t bit = (pin % 10) * 3;

    uint32_t oldValue = gpio_regs->GPFSEL[regIndex];
    uint32_t mask = 0b111 << bit;
	
    gpio_regs->GPFSEL[regIndex] = (oldValue & ~mask) | ((code << bit) & mask);
}

/*
 * GetGPIOFunction function
 *  Parameters:
 *   pin   - number of GPIO pin;
 *  Operation:
 *   Based on the specified GPIO pin, reads the function that is currently performed by the pin.
 */
FSEL GetGPIOFunction(GPIO pin)
{
    uint8_t regIndex = pin / 10;
    uint8_t bit = (pin % 10) * 3;
	
	uint32_t mask = 0b111 << bit;
	
	uint8_t val = ((gpio_regs->GPFSEL[regIndex] & mask)>>bit);
	
	if (val == 0)
		return GPIO_INPUT;
	else if (val == 1)
		return GPIO_OUTPUT;
	else if (val == 4)
		return GPIO_ALT_FUNC0;
	else if (val == 5)
		return GPIO_ALT_FUNC1;
	else if (val == 6)
		return GPIO_ALT_FUNC2;
	else if (val == 7)
		return GPIO_ALT_FUNC3;
	else if (val == 3)
		return GPIO_ALT_FUNC4;
	else if (val == 2)
		return GPIO_ALT_FUNC5;
	else
		return UNDEFINED;
}

/*
 * SetGpioPin function
 *  Parameters:
 *   pin       - number of GPIO pin;
 *  Operation:
 *   Sets the desired GPIO pin to HIGH level. The pin should previously be defined as output.
 */
void SetGpioPin(GPIO pin)
{
	uint8_t regIndex = pin / 32;
	uint8_t bit = pin % 32;
	
    uint32_t mask = 1 << bit;
	
	gpio_regs->GPSET[regIndex] = mask;
}

/*
 * ClearGpioPin function
 *  Parameters:
 *   pin       - number of GPIO pin;
 *  Operation:
 *   Sets the desired GPIO pin to LOW level. The pin should previously be defined as output.
 */
void ClearGpioPin(GPIO pin)
{
    uint8_t regIndex = pin / 32;
	uint8_t bit = pin % 32;
	
    uint32_t mask = 1 << bit;
	
	gpio_regs->GPCLR[regIndex] = mask;
}

/*
 * GetGpioPinLevel function
 *  Parameters:
 *   pin       - number of GPIO pin;
 *  Operation:
 *   Reads the current level of GPIO pin, 0 for LOW and 1 for HIGH.
 */
uint32_t GetGpioPinLevel(GPIO pin)
{
	uint8_t regIndex = pin / 32;
	uint8_t bit = pin % 32;
	
    uint32_t mask = 1 << bit;
	
	uint32_t lev = (gpio_regs->GPLEV[regIndex] &= mask) >> bit;
	return lev;
}

//////////////////////////////////////////////////////////////////////////////////////////

/* PWM registers base address */
#define PWM_BASE (BCM2708_PERI_BASE + 0x20C000)
#define PWM_CLK_BASE (BCM2708_PERI_BASE + 0x101000)
#define PWMCLK_CTL	40
#define PWMCLK_DIV	41

/* PWM registers */
volatile struct S_PWM_REGS
{
	uint32_t CTL; // PWM Control Register
	uint32_t STA; // PWM Status Register
	uint32_t DMAC; // PMW DMA Configuration Register
	uint32_t reserved0; // Reserved Byte
	uint32_t RNG1; // PWM Channel 1 Range Register
	uint32_t DAT1; // PMW Channel 1 Data Register
	uint32_t FIF1; // PMW FIFO Input Register
	uint32_t reserved1; // Reserved Byte
	uint32_t RNG2; // PMW Channel 2 Range Register
	uint32_t DAT2; // PMW Channel 2 Data Register
} *pwm_regs;

/* PWM Control Register (CTL) bit set starting with LSB:
    PWEN1 : 1 bit - Used to enable/disable PWM channel 0.
    MODE1 : 1 bit - Used to determine the mode of operation of channel 0. Setting to 0/1 enables PWM/Serial mode.
                    In PWM mode, data stored in either PMW DAT1 register or FIFO is transmitted by pulse width modulation
                    within the range defined by PWM RNG1 register.
                    In Serial mode, data stored in either PMW DAT1 register or FIFO is transmitted serially within the range
                    defined by PWM RNG1 register. Data is transmitted MSB first and truncated or zero padded depending on
                    PMW RNG1 and available data.
    RPTL1 : 1 bit - Used to enable/disable the repeating of the last data available in the FIFO just before it empties.
                    When this bit is 1 and FIFO is used in transmission, the last available data in the FIFO is repeatedly sent.
                    This may be useful in PWM mode to avoid duty cycle gaps.
    SBIT1 : 1 bit - Defines the state of the output pin on channel 0 when there is no transmission.
    POLA1 : 1 bit - Defines the polarity of the output pin on channel 0. When set to 1, the output is inverted.
    USEF1 : 1 bit - Used to enable/disable FIFO transfer. When set to 0/1 - data stored in PWM DAT1 register/FIFO is used for transmission.
    CLRF1 : 1 bit - Used to clear the FIFO. Writing 1 to this bit clears the FIFO. Both channels use the same FIFO.
    MSEN1 : 1 bit - Used to determine whether to use PWM algorithm (when set to 1) or simple M/S (Mark/Space) mode transmission (when set to 0).
                    When MODE1 bit is set to 1, this bit has no effect, since channel 0 is then is serial mode.
    PWEN2 : 1 bit - Same as PWEN1, but for channel 1.
    MODE2 : 1 bit - Same as MODE1, but for channel 1.
    RPTL2 : 1 bit - Same as RPTL1, but for channel 1.
    SBIT2 : 1 bit - Same as SBIT1, but for channel 1.
    POLA2 : 1 bit - Same as POLA1, but for channel 1.
    USEF2 : 1 bit - Same as USEF1, but for channel 1.
    Reserved1 : 1 bit
    MSEN2 : 1 bit - Same as MSEN1, but for channel 1.
    Reserved2 : 16 bits
*/

/* PWM Status Register (STA) bit set starting with LSB:
    FULL1 : 1 bit - Indicates the full status of the FIFO. If this bit is 1, FIFO is full.
    EMPT1 : 1 bit - Indicates the empty status of the FIFO. If this bit is 1, FIFO is empty.
    WERR1 : 1 bit - Sets to 1 when a write occurs on full FIFO. Write 1 to this bit to clear it.
    RERR1 : 1 bit - Sets to 1 when a read occurs on empty FIFO. Write 1 to this bit to clear it.
    GAPO1 : 1 bit - These bits indicate that there has been a gap between transmission of two consecutive data form FIFO on channel 0.
                    It may happen when FIFO empties and state machine waits for next word to send from FIFO.
                    If control bit RPTL1 is set to 1, this event will not occur and GAPO1 will always be 0.
    GAPO2 : 1 bit - Same as GAPO1, but for channel 1.
    GAPO3 : 1 bit - Same as GAPO1, but for channel 2. - not accessible on RPI 3 model B+
    GAPO4 : 1 bit - Same as GAPO1, but for channel 3. - not accessible on RPI 3 model B+
    BERR  : 1 bit - Sets to 1 when an error has occured while writing to registers via APB. Write 1 to this bit to clear it. 
    STA1  : 1 bit - Indicates the current state of the channel 0 which is useful for debugging purposes.
                    0 means the channel is not currently transmitting, and 1 means that it is.
    STA2  : 1 bit - Same as STA 1, but for channel 1.
    STA3  : 1 bit - Same as STA 1, but for channel 2.  - not accessible on RPI 3 model B+
    STA4  : 1 bit - Same as STA 1, but for channel 3.  - not accessible on RPI 3 model B+
    Reserved : 19 bits
*/


volatile uint32_t *pwm_clk_regs; // Holds the address of PWM CLK registers

/* PWM Clock Control Register (CM_PWMCTL) bit set starting from LSB:
    SRC    : 4 bits - Clock Source - 0 is GND, 1 is PWM Oscillator at 19.2MHz frequency.
    ENAB   : 1 bit  - Clock Enable - Write 1 to enable, or 0 to disable -> clock won't stop immediately because
                      the cycle must be allowed to complete to avoid glitches. The BUSY bit will go LOW when the
                      final cycle is completed.
    KILL : 1 bit    - Kill the clock generator - Write 1 to stop and reset clock generator.
Unused1 : 1 bit
    BUSY   : 1 bit  - Flag that indicates if clock generator is running (1 when running).
    FLIP   : 1 bit  - Invert the clock generator output, when set to 1, 0 means HIGH and 1 LOW.
    MASH   : 2 bits - MASH control - Write 0 for only integer division.
Unused2 : 13 bits
    PASSWD : 8 bits - Clock manager password - In order to modify this register, this field has to be "5A".
*/

/* PWM Clock Divisor Register (CM_PWMDIV) bit set starting from LSB:
    DIVF : 12 bits  - Fractional part of divisor.
    DIVI : 12 bits  - Integer part of divisor.
    PASSWD : 8 bits - Clock manager password - In order to modify this register, this field has to be "5A".
*/


/*
 * Establish PWM Frequency function
 *  Parameters:
 *   divi   - integer part of oscillator frequency divisor.
 *  Operation:
 *   Based on the provided integer divisor, establishes a new PWM frequency for both channels.
 */
void pwm_frequency(uint32_t divi) {

	// Kill the clock
	*(pwm_clk_regs+PWMCLK_CTL) = 0x5A000020;
	
	// Disable PWM
	pwm_regs->CTL &= 0xFEFE;
	//pwm_ctl->PWEN1 = 0;
	//pwm_ctl->PWEN2 = 0;
	udelay(10);
	
	// Set the divisor
	*(pwm_clk_regs+PWMCLK_DIV) = 0x5A000000 | (divi << 12);

	// Set source to oscillator and enable clock
	*(pwm_clk_regs+PWMCLK_CTL) = 0x5A000011;
	udelay(10);
}

/*
 * Set up PWM Channels function
 *  Operation:
 *   Writes to CTL register as described in the comment inside function.
 */
void set_up_pwm_channels(void){
	
	pwm_regs->CTL = 0x000080C0;
	
	/*
	// Channel 1 set-up
	pwm-ctl->PWEN1 = 0; // Disable channel 0
	pwm_ctl->MODE1 = 0; // Set PWM mode
	pwm_ctl->RPTL1 = 0; // Transmission interrupts when FIFO is empty
	pwm_ctl->SBIT1 = 0; // State of the output when PWM is stopped
	pwm_ctl->POLA1 = 0; // Channel polarity - if POLA is set to 0 then 0 represents LOW and 1 represents HIGH
	pwm_ctl->USEF1 = 0; // Data register is transmitted, FIFO is not used for transmission when set to 0
	pwm_ctl->CLRF1 = 1; // Clears the FIFO
	pwm_ctl->MSEN1 = 1; // Set PWM Mark-Space mode

	// Channel 2 set-up
	pwm-ctl->PWEN2 = 0; // Disable channel 1
	pwm_ctl->MODE2 = 0; // Set PWM mode
	pwm_ctl->RPTL2 = 0; // Transmission interrupts when FIFO is empty
	pwm_ctl->SBIT2 = 0; // State of the output when PWM is stopped
	pwm_ctl->POLA2 = 0; // Channel polarity - if POLA is set to 0 then 0 represents LOW and 1 represents HIGH
	pwm_ctl->USEF2 = 0; // Data register is transmitted, FIFO is not used for transmission when set to 0
	pwm_ctl->Res1  = 0;
	pwm_ctl->MSEN2 = 1; // Set PWM Mark-Space mode
	pwm_ctl->Res2  = 0;
	*/
}

/*
 * PWM Ratio for Channel 0 function
 *  Operation:
 *   Writes to DAT1 AND RNG1 registers, resulting ing n/m duty cycle on channel 0.
 */
void pwm_ratio_c0(uint32_t n, uint32_t m) {

	// Disable PWM Channel 0
	pwm_regs->CTL &= 0xFFFFFFFE;
	//pwm_ctl->PWEN1 = 0;

	// Set the PWM Channel 0 Data Register
	pwm_regs->DAT1 = n;
	// Set the PWM Channel 0 Range Register
	pwm_regs->RNG1 = m;
	udelay(10);
	
	// Enable PWM Channel 0
	pwm_regs->CTL |= 0x00000001;
	//pwm_ctl->PWEN1 = 1;
}

/*
 * PWM Ratio for Channel 1 function
 *  Operation:
 *   Writes to DAT2 AND RNG2 registers, resulting ing n/m duty cycle on channel 1.
 */
void pwm_ratio_c1(uint32_t n, uint32_t m) {

	// Disable PWM Channel 1
	pwm_regs->CTL &= 0xFFFFFEFF;
	//pwm_ctl->PWEN2 = 0;
	
	// Set the PWM Channel 1 Data Register
	pwm_regs->DAT2 = n;
	// Set the PWM Channel 1 Range Register
	pwm_regs->RNG2 = m;
	udelay(10);
	
	// Enable PWM Channel 1
	pwm_regs->CTL |= 0x00000100;
	//pwm_ctl->PWEN2 = 1;
}

//////////////////////////////////////////////////////////////////////////////////////////

/* Declaration of buggy_driver.c functions */
int buggy_driver_init(void);
void buggy_driver_exit(void);
static int buggy_driver_open(struct inode *, struct file *);
static int buggy_driver_release(struct inode *, struct file *);
static ssize_t buggy_driver_read(struct file *, char *buf, size_t , loff_t *);
static ssize_t buggy_driver_write(struct file *, const char *buf, size_t , loff_t *);
static long buggy_driver_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/* Definitions of IOCTL commands */
#define FORWARD        _IOW('p',1,unsigned)
#define BACKWARD       _IOW('p',2,unsigned)
#define LEFT           _IOW('p',3,unsigned)
#define RIGHT          _IOW('p',4,unsigned)
#define LEFT_BLINKERS  _IOW('p',5,unsigned)
#define RIGHT_BLINKERS _IOW('p',6,unsigned)
#define BRAKE          _IOW('p',7,unsigned)
#define HEADLIGHTS     _IOW('p',8,unsigned)
#define NO_ACTION      _IOW('p',9,unsigned)

/* Definitions of PWM related settings */
#define DAT_MIN    200 // Defines the minimum duty cycle to be 20%, buggy can't move if duty cycle is lower due to weight or uneven terrain
#define DAT_START  500 // Defines the starting duty cycle to be 50%, which gives buggy the required force to start moving
#define DAT_MAX    800 // Defines the maximum duty cycle to be 80%
#define DDC         50 // Defines delay between starting duty cycle change
#define TURN_MIN   500 // Minimum turn duty cycle
#define RANGE     1000

// Blinkers interval
#define TIMER_SEC	0
#define TIMER_NANO_SEC	500000000

/* Structure that declares buggy_driver file operations */
struct file_operations buggy_driver_fops =
{
    open            :   buggy_driver_open,
    release         :   buggy_driver_release,
    read            :   buggy_driver_read,
    write           :   buggy_driver_write,
    unlocked_ioctl  :   buggy_driver_ioctl
};

/* Declaration of the init and exit functions. */
module_init(buggy_driver_init);
module_exit(buggy_driver_exit);

/* Major number. */
int buggy_driver_major;

/* Left and right blinkers timer */
static struct hrtimer btimer;
static ktime_t kt;

/* Blinkers states, LSB is for left blinkers and a bit above LSB is for right blinkers */
uint8_t bstates = 0;

/* Timer value */
uint32_t tval = 0;

/* Spinlock for ioctl access */
static spinlock_t ilock;

/* Blinkers timer callback function */
static enum hrtimer_restart btimer_callback(struct hrtimer *param)
{
	tval++;
	if(tval == 100000)
		tval = 0;
	
	// Left blinkers
	if((bstates & 1) == 0)
		ClearGpioPin(GPIO_24);
	else{
		if(tval %2 == 0)
			SetGpioPin(GPIO_24);
		else
			ClearGpioPin(GPIO_24);
	}
	
	// Right blinkers
	if((bstates & 2) == 0)
		ClearGpioPin(GPIO_25);
	else{
		if(tval %2 == 0)
			SetGpioPin(GPIO_25);
		else
			ClearGpioPin(GPIO_25);
	}
	
    hrtimer_forward(&btimer, ktime_get(), kt);

    return HRTIMER_RESTART;
}

/*
 * Initialization:
 *  1. Register device driver
 *  2. Map the GPIO register space from PHYSICAL address space to VIRTUAL address space
 *  3. Map the PWM register space from PHYSICAL address space to VIRTUAL address space
 *  4. Map the PWM Clock register space from PHYSICAL address space to VIRTUAL address space
 *  5. Initialize GPIO pins
 *  6. Set up PWM channels and establish PWM frequency
 *  7. Initialize blinkers timer
 *  8. Initialize ioctl spinlock
 */
int buggy_driver_init(void)
{
    int result = -1;

    printk(KERN_INFO "Inserting buggy_driver module..\n");

    /* Registering device. */
    result = register_chrdev(0, "buggy_driver", &buggy_driver_fops);
    if (result < 0)
    {
        printk(KERN_INFO "buggy_driver: cannot obtain major number %d\n", buggy_driver_major);
        return result;
    }

    buggy_driver_major = result;
    printk(KERN_INFO "buggy_driver major number is %d\n", buggy_driver_major);

    // Map the GPIO register space from PHYSICAL address space to VIRTUAL address space
    gpio_regs = (struct S_GPIO_REGS *)ioremap(GPIO_BASE, sizeof(struct S_GPIO_REGS));
    if(!gpio_regs)
    {
        result = -ENOMEM;
        goto fail_no_virt_mem;
    }
	//printk(KERN_INFO "GPFSEL Register 1 value: 0x%x\n", gpio_regs->GPFSEL[1]);
	
	// Map the PWM register space from PHYSICAL address space to VIRTUAL address space
	pwm_regs = (struct S_PWM_REGS *)ioremap(PWM_BASE, sizeof(struct S_PWM_REGS));
	if(!pwm_regs)
    {
        result = -ENOMEM;
        goto fail_no_virt_mem;
    }
	
	// Map the PWM Clock register space from PHYSICAL address space to VIRTUAL address space
	pwm_clk_regs = ioremap(PWM_CLK_BASE, 4096);
	if(!pwm_clk_regs)
    {
        result = -ENOMEM;
        goto fail_no_virt_mem;
    }
	
	
	// Setting GPIO pins to initial state
	// Left FORWARD
	SetGPIOFunction(GPIO_12, GPIO_OUTPUT);
	ClearGpioPin(GPIO_12);
	
	// Left BACKWARD
	SetGPIOFunction(GPIO_18, GPIO_OUTPUT);
	ClearGpioPin(GPIO_18);
	
	// Right FORWARD
	SetGPIOFunction(GPIO_13, GPIO_OUTPUT);
	ClearGpioPin(GPIO_13);
	
	// Right BACKWARD
	SetGPIOFunction(GPIO_19, GPIO_OUTPUT);
	ClearGpioPin(GPIO_19);
	
	// Headlamps
	SetGPIOFunction(GPIO_04, GPIO_OUTPUT);
	SetGpioPin(GPIO_04);
	
	// Head lights
	SetGPIOFunction(GPIO_16, GPIO_OUTPUT);
	ClearGpioPin(GPIO_16);
	
	// Brake lights
	SetGPIOFunction(GPIO_27, GPIO_OUTPUT);
	ClearGpioPin(GPIO_27);
	
	// Left blinkers
	SetGPIOFunction(GPIO_24, GPIO_OUTPUT);
	ClearGpioPin(GPIO_24);
	
	// Right blinkers
	SetGPIOFunction(GPIO_25, GPIO_OUTPUT);
	ClearGpioPin(GPIO_25);
	
	// Set up PWM channels
	set_up_pwm_channels();
	
	// Setting PWM with approx 1kHz frequency and fine tuning with 0.1% duty cycle step
	// PWM frequency can be calculated with formula -> pwmFrequency in Hz = 19200000Hz / divi / pwm_range.
	pwm_frequency(19);
	
	// Initialize blinkers timer
	kt = ktime_set(TIMER_SEC, TIMER_NANO_SEC);
	hrtimer_init(&btimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	btimer.function = &btimer_callback;
	
	// Initialize ioctl spinlock
	spin_lock_init(&ilock);

	return 0;

fail_no_virt_mem:

	// Unmap the GPIO registers PHYSICAL address space from VIRTUAL memory
	if (gpio_regs)
		iounmap(gpio_regs);
	// Unmap the PWM registers PHYSICAL address space from VIRTUAL memory
	if (pwm_regs)
		iounmap(pwm_regs);
	// Unmap the PWM Clock registers PHYSICAL address space from VIRTUAL memory
	if (pwm_clk_regs)
		iounmap(pwm_clk_regs);
	
    /* Freeing the major number. */
    unregister_chrdev(buggy_driver_major, "buggy_driver");

    return result;
}

/*
 * Cleanup:
 *  1. stop the timer
 *  2. release GPIO pins (clear all outputs, set all as inputs)
 *  3. Unmap PHYSICAL address space from VIRTUAL address
 *  4. Cancel blinkers timer
 *  4. Unregister device driver
 */
void buggy_driver_exit(void)
{
	// Kill the clock
	*(pwm_clk_regs+PWMCLK_CTL) = 0x5A000020;
	
	// Disable PWM
	pwm_regs->CTL &= 0xFEFE;
	//pwm_ctl->PWEN1 = 0;
	//pwm_ctl->PWEN2 = 0;
	
	// Resetting GPIO pins to initial state
	SetGPIOFunction(GPIO_12, GPIO_OUTPUT);
	ClearGpioPin(GPIO_12);
	SetGPIOFunction(GPIO_18, GPIO_OUTPUT);
	ClearGpioPin(GPIO_18);
	SetGPIOFunction(GPIO_13, GPIO_OUTPUT);
	ClearGpioPin(GPIO_13);
	SetGPIOFunction(GPIO_19, GPIO_OUTPUT);
	ClearGpioPin(GPIO_19);

	ClearGpioPin(GPIO_04);
	ClearGpioPin(GPIO_16);
	ClearGpioPin(GPIO_27);
	ClearGpioPin(GPIO_24);
	ClearGpioPin(GPIO_25);
	
	// Setting the GPIO pins to default function - as INPUT
	SetGPIOFunction(GPIO_12, GPIO_INPUT);
	SetGPIOFunction(GPIO_13, GPIO_INPUT);
	SetGPIOFunction(GPIO_18, GPIO_INPUT);
	SetGPIOFunction(GPIO_19, GPIO_INPUT);
	SetGPIOFunction(GPIO_04, GPIO_INPUT); // Headlamps
	SetGPIOFunction(GPIO_16, GPIO_INPUT); // Head lights
	SetGPIOFunction(GPIO_27, GPIO_INPUT); // Brake lights
	SetGPIOFunction(GPIO_24, GPIO_INPUT); // Left blinkers
	SetGPIOFunction(GPIO_25, GPIO_INPUT); // Right blinkers
	
	// Unmap the GPIO registers PHYSICAL address space from VIRTUAL memory
	if (gpio_regs)
		iounmap(gpio_regs);
	// Unmap the PWM registers PHYSICAL address space from VIRTUAL memory
	if (pwm_regs)
		iounmap(pwm_regs);
	// Unmap the PWM Clock registers PHYSICAL address space from VIRTUAL memory
	if (pwm_clk_regs)
		iounmap(pwm_clk_regs);
	
	hrtimer_cancel(&btimer);
	
    printk(KERN_INFO "Removing buggy_driver module\n");

    /* Freeing the major number. */
    unregister_chrdev(buggy_driver_major, "buggy_driver");
}

/*
 * File ioctl function
 *  Parameters:
 *   f  - a type file structure;
 *   cmd   - a buffer in which the user space function (fwrite) will write;
 *   arg - a counter with the number of bytes to transfer, which has the same
 *           values as the usual counter in the user space function (fwrite);
 *  Operation:
 *   The function buggy_driver_ioctl performs specific operations that are commanded from user space.
 */
static long buggy_driver_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	spin_lock(&ilock);
	
	switch (cmd)
    {
        case FORWARD:
			ClearGpioPin(GPIO_27); // Turn off break lights
			if(GetGPIOFunction(GPIO_12) == GPIO_ALT_FUNC0 && GetGPIOFunction(GPIO_19) == GPIO_ALT_FUNC5){
				SetGPIOFunction(GPIO_19, GPIO_OUTPUT); // Right BACKWARD OFF
				ClearGpioPin(GPIO_19);
				SetGPIOFunction(GPIO_13, GPIO_ALT_FUNC0); // Right FORWARD ON
				
				pwm_ratio_c0(DAT_MIN, RANGE);
				pwm_ratio_c1(DAT_MIN, RANGE);
			}
			else if(GetGPIOFunction(GPIO_13) == GPIO_ALT_FUNC0 && GetGPIOFunction(GPIO_18) == GPIO_ALT_FUNC5){
				SetGPIOFunction(GPIO_18, GPIO_OUTPUT); // Left BACKWARD OFF
				ClearGpioPin(GPIO_18);
				SetGPIOFunction(GPIO_12, GPIO_ALT_FUNC0); // Left FORWARD ON
				
				pwm_ratio_c0(DAT_MIN, RANGE);
				pwm_ratio_c1(DAT_MIN, RANGE);
			}
			else if(GetGPIOFunction(GPIO_18) == GPIO_ALT_FUNC5 || GetGPIOFunction(GPIO_19) == GPIO_ALT_FUNC5){
				SetGpioPin(GPIO_27); // Turn on break lights
				
				SetGPIOFunction(GPIO_18, GPIO_OUTPUT); // Left BACKWARD OFF
				ClearGpioPin(GPIO_18);
				SetGPIOFunction(GPIO_19, GPIO_OUTPUT); // Right BACKWARD OFF
				ClearGpioPin(GPIO_19);
				
				pwm_ratio_c0(0, RANGE);
				pwm_ratio_c1(0, RANGE);
			}
			else {
				SetGPIOFunction(GPIO_12, GPIO_ALT_FUNC0); // Left FORWARD ON
				SetGPIOFunction(GPIO_13, GPIO_ALT_FUNC0); // Right FORWARD ON
			
				if (pwm_regs->DAT1 < DAT_MIN && pwm_regs->DAT2 < DAT_MIN){
					pwm_ratio_c0(DAT_START, RANGE);
					pwm_ratio_c1(DAT_START, RANGE);
					mdelay(DDC);
					pwm_ratio_c0(DAT_MIN, RANGE);
					pwm_ratio_c1(DAT_MIN, RANGE);
				}
				else if (pwm_regs->DAT1 < DAT_MIN){
					pwm_ratio_c0(DAT_START, RANGE);
					mdelay(DDC);
					pwm_ratio_c0(DAT_MIN, RANGE);
				}
				else if (pwm_regs->DAT2 < DAT_MIN){
					pwm_ratio_c1(DAT_START, RANGE);
					mdelay(DDC);
					pwm_ratio_c1(DAT_MIN, RANGE);
				}
				else if (pwm_regs->DAT1 < pwm_regs->DAT2){
					pwm_ratio_c0(pwm_regs->DAT1 + 1, RANGE);
				}
				else if (pwm_regs->DAT2 < pwm_regs->DAT1){
					pwm_ratio_c1(pwm_regs->DAT2 + 1, RANGE);
				}
				else if (pwm_regs->DAT1 < DAT_MAX && pwm_regs->DAT2 < DAT_MAX){
					pwm_ratio_c0(pwm_regs->DAT1 + 1, RANGE);
					pwm_ratio_c1(pwm_regs->DAT2 + 1, RANGE);
				}
			}
            break;
        case BACKWARD:
			ClearGpioPin(GPIO_27); // Turn off break lights
			if(GetGPIOFunction(GPIO_12) == GPIO_ALT_FUNC0 && GetGPIOFunction(GPIO_19) == GPIO_ALT_FUNC5){
				SetGPIOFunction(GPIO_12, GPIO_OUTPUT);
				ClearGpioPin(GPIO_12);
				SetGPIOFunction(GPIO_18, GPIO_ALT_FUNC5);

				pwm_ratio_c0(DAT_MIN, RANGE);
				pwm_ratio_c1(DAT_MIN, RANGE);
			}
			else if(GetGPIOFunction(GPIO_13) == GPIO_ALT_FUNC0 && GetGPIOFunction(GPIO_18) == GPIO_ALT_FUNC5){
				SetGPIOFunction(GPIO_13, GPIO_OUTPUT);
				ClearGpioPin(GPIO_13);
				SetGPIOFunction(GPIO_19, GPIO_ALT_FUNC5);
				
				pwm_ratio_c0(DAT_MIN, RANGE);
				pwm_ratio_c1(DAT_MIN, RANGE);
			}
			else if(GetGPIOFunction(GPIO_12) == GPIO_ALT_FUNC0 || GetGPIOFunction(GPIO_13) == GPIO_ALT_FUNC0){
				SetGpioPin(GPIO_27); // Turn on break lights
				
				SetGPIOFunction(GPIO_12, GPIO_OUTPUT); // Left FORWARD OFF
				ClearGpioPin(GPIO_12);
				SetGPIOFunction(GPIO_13, GPIO_OUTPUT); // Right FORWARD OFF
				ClearGpioPin(GPIO_13);
				
				pwm_ratio_c0(0, RANGE);
				pwm_ratio_c1(0, RANGE);
			}
			else {
				SetGPIOFunction(GPIO_18, GPIO_ALT_FUNC5); // Left BACKWARD ON
				SetGPIOFunction(GPIO_19, GPIO_ALT_FUNC5); // Right BACKWARD ON
			
				if (pwm_regs->DAT1 < DAT_MIN && pwm_regs->DAT2 < DAT_MIN){
					pwm_ratio_c0(DAT_START, RANGE);
					pwm_ratio_c1(DAT_START, RANGE);
					mdelay(DDC);
					pwm_ratio_c0(DAT_MIN, RANGE);
					pwm_ratio_c1(DAT_MIN, RANGE);
				}
				else if (pwm_regs->DAT1 < DAT_MIN){
					pwm_ratio_c0(DAT_START, RANGE);
					mdelay(DDC);
					pwm_ratio_c0(DAT_MIN, RANGE);
				}
				else if (pwm_regs->DAT2 < DAT_MIN){
					pwm_ratio_c1(DAT_START, RANGE);
					mdelay(DDC);
					pwm_ratio_c1(DAT_MIN, RANGE);
				}
				else if (pwm_regs->DAT1 < pwm_regs->DAT2){
					pwm_ratio_c0(pwm_regs->DAT1 + 1, RANGE);
				}
				else if (pwm_regs->DAT2 < pwm_regs->DAT1){
					pwm_ratio_c1(pwm_regs->DAT2 + 1, RANGE);
				}
				else if (pwm_regs->DAT1 < DAT_MAX && pwm_regs->DAT2 < DAT_MAX){
					pwm_ratio_c0(pwm_regs->DAT1 + 1, RANGE);
					pwm_ratio_c1(pwm_regs->DAT2 + 1, RANGE);
				}
			}
            break;
		case LEFT:
			ClearGpioPin(GPIO_27); // Turn off break lights
			if(GetGPIOFunction(GPIO_12) == GPIO_ALT_FUNC0 && GetGPIOFunction(GPIO_19) == GPIO_ALT_FUNC5)
			{
				SetGPIOFunction(GPIO_12, GPIO_OUTPUT); // Left FORWARD OFF
				ClearGpioPin(GPIO_12);
				SetGPIOFunction(GPIO_19, GPIO_OUTPUT); // RIGHT BACKWARD OFF
				ClearGpioPin(GPIO_19);
				
				pwm_ratio_c0(0, RANGE);
				pwm_ratio_c1(0, RANGE);
			}
			else {
				if(GetGPIOFunction(GPIO_12) == GPIO_ALT_FUNC0){
					SetGPIOFunction(GPIO_12, GPIO_OUTPUT); // Left FORWARD OFF
					ClearGpioPin(GPIO_12);
				}
				if(GetGPIOFunction(GPIO_19) == GPIO_ALT_FUNC5){
					SetGPIOFunction(GPIO_19, GPIO_OUTPUT); // RIGHT BACKWARD OFF
					ClearGpioPin(GPIO_19);
				}
				
				if(pwm_regs->DAT1 < TURN_MIN && pwm_regs->DAT2 < TURN_MIN){
					pwm_ratio_c0(TURN_MIN, RANGE);
					pwm_ratio_c1(TURN_MIN, RANGE);
				}
				else if(pwm_regs->DAT1 < TURN_MIN){
					pwm_ratio_c0(TURN_MIN, RANGE);
				}
				else if(pwm_regs->DAT2 < TURN_MIN){
					pwm_ratio_c1(TURN_MIN, RANGE);
				}
				else if (pwm_regs->DAT1 < pwm_regs->DAT2){
					pwm_ratio_c0(pwm_regs->DAT1 + 1, RANGE);
				}
				else if (pwm_regs->DAT2 < pwm_regs->DAT1){
					pwm_ratio_c1(pwm_regs->DAT2 + 1, RANGE);
				}
				else if (pwm_regs->DAT1 < DAT_MAX && pwm_regs->DAT2 < DAT_MAX){
					pwm_ratio_c0(pwm_regs->DAT1 + 1, RANGE);
					pwm_ratio_c1(pwm_regs->DAT2 + 1, RANGE);
				}
				
				SetGPIOFunction(GPIO_13, GPIO_ALT_FUNC0); // RIGHT FORWARD ON
				SetGPIOFunction(GPIO_18, GPIO_ALT_FUNC5); // Left BACKWARD ON
			}
            break;
		case RIGHT:
			ClearGpioPin(GPIO_27); // Turn off break lights
			if(GetGPIOFunction(GPIO_13) == GPIO_ALT_FUNC0 && GetGPIOFunction(GPIO_18) == GPIO_ALT_FUNC5)
			{
				SetGPIOFunction(GPIO_13, GPIO_OUTPUT); // Left FORWARD OFF
				ClearGpioPin(GPIO_13);
				SetGPIOFunction(GPIO_18, GPIO_OUTPUT); // RIGHT BACKWARD OFF
				ClearGpioPin(GPIO_18);
				
				pwm_ratio_c0(0, RANGE);
				pwm_ratio_c1(0, RANGE);
			}
			else{
				if(GetGPIOFunction(GPIO_13) == GPIO_ALT_FUNC0){
					SetGPIOFunction(GPIO_13, GPIO_OUTPUT); // RIGHT FORWARD OFF
					ClearGpioPin(GPIO_13);
				}
				if(GetGPIOFunction(GPIO_18) == GPIO_ALT_FUNC5){
					SetGPIOFunction(GPIO_18, GPIO_OUTPUT); // Left BACKWARD OFF
					ClearGpioPin(GPIO_18);
				}
				
				if(pwm_regs->DAT1 < TURN_MIN && pwm_regs->DAT2 < TURN_MIN){
					pwm_ratio_c0(TURN_MIN, RANGE);
					pwm_ratio_c1(TURN_MIN, RANGE);
				}
				else if (pwm_regs->DAT1 < TURN_MIN){
					pwm_ratio_c0(TURN_MIN, RANGE);
				}
				else if (pwm_regs->DAT2 < TURN_MIN){
					pwm_ratio_c1(TURN_MIN, RANGE);
				}
				else if (pwm_regs->DAT1 < pwm_regs->DAT2){
					pwm_ratio_c0(pwm_regs->DAT1 + 1, RANGE);
				}
				else if (pwm_regs->DAT2 < pwm_regs->DAT1){
					pwm_ratio_c1(pwm_regs->DAT2 + 1, RANGE);
				}
				else if (pwm_regs->DAT1 < DAT_MAX && pwm_regs->DAT2 < DAT_MAX){
					pwm_ratio_c0(pwm_regs->DAT1 + 1, RANGE);
					pwm_ratio_c1(pwm_regs->DAT2 + 1, RANGE);
				}
				
				SetGPIOFunction(GPIO_12, GPIO_ALT_FUNC0); // Left FORWARD ON
				SetGPIOFunction(GPIO_19, GPIO_ALT_FUNC5); // Right BACKWARD ON
			}
            break;
		case LEFT_BLINKERS:
			if((bstates & 1) == 0)
			{
				bstates |= 1;
				if(hrtimer_active(&btimer) == 0)
					hrtimer_start(&btimer, kt, HRTIMER_MODE_REL);
			}
			else
				bstates &= 2;
            break;
		case RIGHT_BLINKERS:
			if((bstates & 2) == 0)
			{
				bstates |= 2;
				if(hrtimer_active(&btimer) == 0)
					hrtimer_start(&btimer, kt, HRTIMER_MODE_REL);
			}
			else
				bstates &= 1;
            break;
		case BRAKE:
			if(GetGpioPinLevel(GPIO_27) == 0)
				SetGpioPin(GPIO_27); // Turn on break lights
			else
				ClearGpioPin(GPIO_27); // Turn off break lights
			
			pwm_ratio_c0(0, RANGE);
			pwm_ratio_c1(0, RANGE);
			
			SetGPIOFunction(GPIO_12, GPIO_OUTPUT);
			ClearGpioPin(GPIO_12);
			SetGPIOFunction(GPIO_18, GPIO_OUTPUT);
			ClearGpioPin(GPIO_18);
			SetGPIOFunction(GPIO_13, GPIO_OUTPUT);
			ClearGpioPin(GPIO_13);
			SetGPIOFunction(GPIO_19, GPIO_OUTPUT);
			ClearGpioPin(GPIO_19);
            break;
		case HEADLIGHTS:
			if(GetGpioPinLevel(GPIO_16) == 0)
				SetGpioPin(GPIO_16);
			else
				ClearGpioPin(GPIO_16);
			break;
		case NO_ACTION:
			ClearGpioPin(GPIO_27); // Turn off break lights
			if(pwm_regs->DAT1 > DAT_MIN){
				pwm_ratio_c0(pwm_regs->DAT1 - 1, RANGE);
			}
			else{
				pwm_ratio_c0(0, RANGE);
			}
			if(pwm_regs->DAT2 > DAT_MIN){
				pwm_ratio_c1(pwm_regs->DAT2 - 1, RANGE);
			}
			else{
				pwm_ratio_c1(0, RANGE);
			}
			break;
        default:
            return -EINVAL;
    }
	
	spin_unlock(&ilock);
	
    return 0;
}

/* File open function. */
static int buggy_driver_open(struct inode *inode, struct file *filp)
{
    return 0;
}

/* File close function. */
static int buggy_driver_release(struct inode *inode, struct file *filp)
{
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
 *   Not supported.
 */
static ssize_t buggy_driver_read(struct file *filp, char *buf, size_t len, loff_t *f_pos)
{
    printk(KERN_ALERT "Read operation is not permitted.\n");
	
	return -EPERM; // Operation not permitted error
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
 *   Not supported.
 */
static ssize_t buggy_driver_write(struct file *filp, const char *buf, size_t len, loff_t *f_pos)
{
    printk(KERN_ALERT "Write operation is not permitted.\n");

	return -EPERM; // Operation not permitted error
}

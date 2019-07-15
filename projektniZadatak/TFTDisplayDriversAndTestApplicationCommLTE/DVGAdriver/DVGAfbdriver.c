/*
 * Author: Damjan Prerad
 * Date: 6.7.2019.
 * 
 * This driver was made for a college class "Programing in real time" held during the summer semester at the University of Banja Luka, Republic of Srpska, faculty of electrical engineering
 * Asistants: Srdjan Popic
 * Professor: Mladen Knezic
 * 
 */

/*
 * 
 * Big thanks to asistants and profesors who made this happen
 * also, to the great invention of the interntet, where most of the documentation can be found
 * and countless other people whose code I skimmed through
 * Most of the code here was taken from vfb.c (virtual framebuffer driver) which can be found in Linux source code /drivers/video/fbdev/vfb.c
 * 
 */

/* 
 * Usefull website: https://www.tldp.org/HOWTO/html_single/Framebuffer-HOWTO/
 * 
 * You'll need to create the framebuffer device in /dev. You need one per framebuffer device, so all you need to do is to type in
 * mknod /dev/fb0 c 29 0 for the first one. Subsequent ones would be in multiples of 32,
 * so for example to create /dev/fb1, you would need to type in
 * mknod /dev/fb1 c 29 32
 * , and so on up to the eighth framebuffer device (mknod /dev/fb7 c 29 224)
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

#include <linux/fb.h>
#include <linux/init.h>

#include <linux/timer.h>

#include "GPIOcontrol.h"
#include "DisplayPinout.h"
#include "DisplayFunctions.h"

/*
*  RAM we reserve for the frame buffer. This defines the maximum screen
*  size
*
*/
#define VIDEOMEMSIZE	(3*1024*1024)	/* 3 MB > 3*800*480=1152000 */

static void *videomemory;
static u_long videomemorysize = VIDEOMEMSIZE;

// timer interval defined as (TIMER_SEC + TIMER_NANO_SEC)
#define TIMER_SEC    0
#define TIMER_NANO_SEC 33*1000*1000 

/* 30FPS - this criteria can not be met due to limitations of raspberry PI, limitations of software VGA controler implementation*/

static struct hrtimer screen_timer;
static ktime_t kt;

/*
 * 
 * Periodicaly refresh displayScreen with data in videomemory
 * Idealy it would be 30FPS
 * 
 */

static enum hrtimer_restart screen_timer_callback(struct hrtimer *param)
{
    
    //Here, do screen
    toScreen(videomemory);
    //---------------
    
    hrtimer_forward(&screen_timer, ktime_get(), kt);

    return HRTIMER_RESTART;
}

static char mode_option[] ="800x480-24@30";

static const struct fb_videomode vfb_default = {
	.xres =		800,
	.yres =		480,
	//.pixclock =	20000,
	//.left_margin =	64,
	//.right_margin =	64,
	//.upper_margin =	32,
	//.lower_margin =	32,
	//.hsync_len =	64,
	//.vsync_len =	2,
	.vmode =	FB_VMODE_NONINTERLACED
};

static struct fb_fix_screeninfo vfb_fix = {
	.id =		"DVGAfbdriver",
	.type =		FB_TYPE_PACKED_PIXELS,
	.visual =	FB_VISUAL_PSEUDOCOLOR,
	.xpanstep =	1,
	.ypanstep =	1,
	.ywrapstep = 1,
	.accel =	FB_ACCEL_NONE
};

static bool vfb_enable __initdata = 1;	/* enabled */

static int vfb_set_par(struct fb_info *info);

static int vfb_mmap(struct fb_info *info,
		    struct vm_area_struct *vma);

static struct fb_ops vfb_ops = {
	.fb_read        = fb_sys_read,
	.fb_write       = fb_sys_write,
	.fb_set_par    	= vfb_set_par,
	.fb_fillrect	= sys_fillrect,
	.fb_copyarea	= sys_copyarea,
	.fb_imageblit	= sys_imageblit,
	.fb_mmap	    = vfb_mmap,
};

    /*
     *  Internal routines
     */

static u_long get_line_length(int xres_virtual, int bpp)
{
	u_long length;

	length = xres_virtual * bpp;
	length = (length + 31) & ~31;
	length >>= 3;
	return (length);
}


/* This routine actually sets the video mode. It's in here where we
 * the hardware state info->par and fix which can be affected by the 
 * change in par. For this driver it doesn't do much. 
 */
static int vfb_set_par(struct fb_info *info)
{
	switch (info->var.bits_per_pixel) {
	case 1:
		info->fix.visual = FB_VISUAL_MONO01;
		break;
	case 8:
		info->fix.visual = FB_VISUAL_PSEUDOCOLOR;
		break;
	case 16:
	case 24:
	case 32:
		info->fix.visual = FB_VISUAL_TRUECOLOR;
		break;
	}

	info->fix.line_length = get_line_length(info->var.xres_virtual,
						info->var.bits_per_pixel);

	return 0;
}

/*
 *  Most drivers don't need their own mmap function 
 */

static int vfb_mmap(struct fb_info *info,
		    struct vm_area_struct *vma)
{
	return remap_vmalloc_range(vma, (void *)info->fix.smem_start, vma->vm_pgoff);
}


/*
 *  Initialisation
 */

static int vfb_probe(struct platform_device *dev)
{
	struct fb_info *info;
	unsigned int size = PAGE_ALIGN(videomemorysize);
	int retval = -ENOMEM;

	if (!(videomemory = vmalloc_32_user(size)))
		return retval;

	info = framebuffer_alloc(sizeof(u32) * 256, &dev->dev);
	if (!info)
		goto err;

	info->screen_base = (char __iomem *)videomemory;
	info->fbops = &vfb_ops;

	if (!fb_find_mode(&info->var, info, mode_option,
			  NULL, 0, &vfb_default, 8)){
		fb_err(info, "Unable to find usable video mode.\n");
		retval = -EINVAL;
		goto err1;
	}

	vfb_fix.smem_start = (unsigned long) videomemory;
	vfb_fix.smem_len = videomemorysize;
	info->fix = vfb_fix;
	info->pseudo_palette = info->par;
	info->par = NULL;
	info->flags = FBINFO_FLAG_DEFAULT;

	retval = fb_alloc_cmap(&info->cmap, 256, 0);
	if (retval < 0)
		goto err1;

	retval = register_framebuffer(info);
	if (retval < 0)
		goto err2;
	platform_set_drvdata(dev, info);

	vfb_set_par(info);

	fb_info(info, "Virtual frame buffer device, using %ldK of video memory\n",
		videomemorysize >> 10);
	return 0;
err2:
	fb_dealloc_cmap(&info->cmap);
err1:
	framebuffer_release(info);
err:
	vfree(videomemory);
	return retval;
}

static int vfb_remove(struct platform_device *dev)
{
	struct fb_info *info = platform_get_drvdata(dev);

	if (info) {
		unregister_framebuffer(info);
		vfree(videomemory);
		fb_dealloc_cmap(&info->cmap);
		framebuffer_release(info);
	}
	return 0;
}

static struct platform_driver vfb_driver = {
	.probe	= vfb_probe,
	.remove = vfb_remove,
	.driver = {
		.name	= "DVGAfbdriver",
	},
};

static struct platform_device *vfb_device;

static int ret;
static int __init vfb_init(void)
{
    printk(KERN_INFO "Damjanov driver");
	ret = 0;

	if (!vfb_enable)
		return -ENXIO;

	ret = platform_driver_register(&vfb_driver);

	if (!ret) {
		vfb_device = platform_device_alloc("DVGAfbdriver", 0);

		if (vfb_device)
			ret = platform_device_add(vfb_device);
		else
			ret = -ENOMEM;

		if (ret) {
			platform_device_put(vfb_device);
			platform_driver_unregister(&vfb_driver);
		}
	}
    
    virt_gpio_base = ioremap(GPIO_BASE, GPIO_ADDR_SPACE_LEN);
    if(!virt_gpio_base)
    {
        ret = -ENOMEM;
        platform_driver_unregister(&vfb_driver);
        return ret;
    }
    
    initBus();
    
    hrtimer_init(&screen_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    kt = ktime_set(TIMER_SEC, TIMER_NANO_SEC);
    screen_timer.function = &screen_timer_callback;
    hrtimer_start(&screen_timer, kt, HRTIMER_MODE_REL);
    
	return ret;
}

module_init(vfb_init);

static void __exit vfb_exit(void)
{
    printk(KERN_INFO "Izlazim iz damjanovog drivera");
    
    hrtimer_cancel(&screen_timer);
    
    releaseBus();
    
    if (virt_gpio_base){
        iounmap(virt_gpio_base);
    }
    
	platform_device_unregister(vfb_device);
	platform_driver_unregister(&vfb_driver);
}

module_exit(vfb_exit);

MODULE_AUTHOR("Damjan Prerad");
MODULE_LICENSE("GPL");

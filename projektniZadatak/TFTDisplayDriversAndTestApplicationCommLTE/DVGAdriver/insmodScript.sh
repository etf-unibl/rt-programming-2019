#hello
sudo insmod /lib/modules/4.14.91-rt49-v7+/kernel/drivers/video/fbdev/core/fb_sys_fops.ko
sudo insmod /lib/modules/4.14.91-rt49-v7+/kernel/drivers/video/fbdev/core/syscopyarea.ko
sudo insmod /lib/modules/4.14.91-rt49-v7+/kernel/drivers/video/fbdev/core/sysfillrect.ko
sudo insmod /lib/modules/4.14.91-rt49-v7+/kernel/drivers/video/fbdev/core/sysimgblt.ko

sudo insmod DVGAfbdriver.ko

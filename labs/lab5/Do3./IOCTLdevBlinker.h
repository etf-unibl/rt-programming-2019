
#include<linux/ioctl.h>

#define IOCTL_MAGIC 243//Major number, why not?

#define IOCTL_HELLO _IO(IOCTL_MAGIC,0)
#define IOCTL_SET_PIN _IOW(IOCTL_MAGIC,1,int)
#define IOCTL_CLEAR_PIN _IOW(IOCTL_MAGIC,2,int)
#define IOCTL_INPUT_PIN _IOW(IOCTL_MAGIC,3,int)
#define IOCTL_OUT_PIN _IOW(IOCTL_MAGIC,4,int)
#define IOCTL_PIN_STATE _IOWR(IOCTL_MAGIC,5,char*)


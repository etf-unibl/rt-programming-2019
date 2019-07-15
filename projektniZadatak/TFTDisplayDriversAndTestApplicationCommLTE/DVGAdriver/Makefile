TARGET := DVGAfbdriver.ko

MDIR := arch/arm/DVGAfbdriver

CURRENT := $(shell uname -r)
KDIR :=/home/damjan/rpi-kernel/rt-kernel/lib/modules/4.14.91-rt49-v7+/build
PWD := $(shell pwd)
DEST := /home/damjan/rpi-kernel/rt-kernel/lib/modules/4.14.91-rt49-v7+/kernel/$(MDIR)

obj-m += DVGAfbdriver.o

default:
	$(MAKE) -I $(KDIR)/arch/arm/include/asm/ -C $(KDIR) M=$(PWD)

install:
	#@if test -f $(DEST)/$(TARGET).orig; then \
	#       echo "Backup of .ko already exists."; \
	#else \
	#       echo "Creating a backup of .ko."; \
	#       mv -v $(DEST)/$(TARGET) $(DEST)/$(TARGET).orig; \
	#fi
	su -c "cp $(TARGET) $(DEST) && /sbin/depmod -a"

revert:
	@echo "Reverting to the original .ko."
	@mv -v $(DEST)/$(TARGET).orig $(DEST)/$(TARGET)

clean:
	rm -f *.o $(TARGET) .*.cmd .*.flags *.mod.c

-include $(KDIR)/Rules.make

TARGET := timer_driver.ko

MDIR := arch/arm/timer_driver

CURRENT := $(shell uname -r)
KDIR := ~/rpi-kernel/linux
PWD := $(shell pwd)
DEST := ~/rpi-kernel/linux/kernel/$(MDIR)

obj-m := timer_driver.o

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

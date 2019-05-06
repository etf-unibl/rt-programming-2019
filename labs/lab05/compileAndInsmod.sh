#FOR GPIO_DRIVER
cd gpio_driver
make clean
make
sudo insmod gpio_driver.ko
sudo mknod /dev/gpio_driver c 243 0
sudo chmod 666 /dev/gpio_driver

#FOR TEST APP
cd ..
gcc -pthread main.c getch.c -o exe -L ~/wiringPi/wiringPi -lwiringPi -I ~/wiringPi/wiringPi

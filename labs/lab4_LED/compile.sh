$CC -pthread -g3 game_of_life_leds.c getch.c -o LED_host -L ~/wiringPi/wiringPi -lwiringPi -I ~/wiringPi/wiringPi

arm-linux-gnueabihf-strip ./LED_host -s -o LED_target

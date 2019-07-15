

#skripta kojom se sve kompajluje i insmoduje, nakon cega se pokrece testna aplikacija

cd mtl2_touchscreen_driver
#make
sudo insmod mtl2_touchscreen_driver.ko
cd ../DVGAdriver
#make
sh insmodScript.sh
cd ../testApp
gcc -o testAppOn testAppOn.c -lpthread
./testAppOn

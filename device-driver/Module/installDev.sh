sudo insmod testDriver.ko
sudo mknod /dev/testDriver c 250 0
sudo chmod 666 /dev/testDriver
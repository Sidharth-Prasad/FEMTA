gcc -o pig2i2c pig2i2c.c
sudo pigpiod -s 2
pigs no
pigs nb 0 0xC
./pig2i2c 3 2 </dev/pigpio0

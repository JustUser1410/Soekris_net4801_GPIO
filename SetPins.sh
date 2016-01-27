#!/bin/ash
# on my Soekris "bash" is called "ash"
#
# A little script that sets all GPIO pins to output
# and then sets all of the GPIO pins
echo "c 3 o" > /sys/kernel/tomas/gpio
echo "c 4 o" > /sys/kernel/tomas/gpio
echo "c 5 o" > /sys/kernel/tomas/gpio
echo "c 6 o" > /sys/kernel/tomas/gpio
echo "c 7 o" > /sys/kernel/tomas/gpio
echo "c 8 o" > /sys/kernel/tomas/gpio
echo "c 9 o" > /sys/kernel/tomas/gpio
echo "c 10 o" > /sys/kernel/tomas/gpio
echo "c 12 o" > /sys/kernel/tomas/gpio
echo "c 13 o" > /sys/kernel/tomas/gpio
echo "c 15 o" > /sys/kernel/tomas/gpio
echo "c 16 o" > /sys/kernel/tomas/gpio

echo "o 3 1" > /sys/kernel/tomas/gpio
echo "o 4 1" > /sys/kernel/tomas/gpio
echo "o 5 1" > /sys/kernel/tomas/gpio
echo "o 6 1" > /sys/kernel/tomas/gpio
echo "o 7 1" > /sys/kernel/tomas/gpio
echo "o 8 1" > /sys/kernel/tomas/gpio
echo "o 9 1" > /sys/kernel/tomas/gpio
echo "o 10 1" > /sys/kernel/tomas/gpio
echo "o 12 1" > /sys/kernel/tomas/gpio
echo "o 13 1" > /sys/kernel/tomas/gpio
echo "o 15 1" > /sys/kernel/tomas/gpio
echo "o 16 1" > /sys/kernel/tomas/gpio

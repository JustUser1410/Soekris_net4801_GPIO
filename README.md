# Soekris_net4801_GPIO
Kernel module to manipulate individual GPIO pins.
I used buildroot to compile everything
There is also a shell script to set the pins at once.

These are all possible functions that might be done with our kernel module.
Samples are working with pin 6, but this works for any other pin.

Command is:
echo "<command>" > /sys/kernel/tomas/gpio

In the <command> section you fill in one of the below:

"i 6"      would read the value of GPIO pin 6
"o 6 1"    would set pin 6
"o 6 0"    would clear pin 6
"c 6 i"    would set pin 6 to input (default)
"c 6 o"    would set pin 6 to output
"c 6 p 1"  would enable pull-up on pin 6 (default)
"c 6 p 0"  would disable pull-up on pin 6
"c 6 t 1"  would set output tipe to Push-pull on pin 6
"c 6 t 0"  would set output tipe to Open-Drain on pin 6 (default)




Pin assignment of the Net4801 GPIOs

                         +----+----+
                   3,3 V |  1 |  2 | 5 V
                         +----+----+
    GPIO0   Port 2 Bit 0 |  3 |  4 | Port 2 Bit 1   GPIO1
                         +----+----+
    GPIO2   Port 2 Bit 2 |  5 |  6 | Port 2 Bit 3   GPIO3
                         +----+----+
    GPIO4   Port 2 Bit 4 |  7 |  8 | Port 2 Bit 5   GPIO5
                         +----+----+
    GPIO6   Port 2 Bit 6 |  9 | 10 | Port 2 Bit 7   GPIO7
                         +----+----+
                     GND | 11 | 12 | Port 0 Bit 4   GPIO8
                         +----+----+
    GPIO9   Port 0 Bit 5 | 13 | 14 | GND
                         +----+----+
    GPIO10  Port 1 Bit 3 | 15 | 16 | Port 1 Bit 2   GPIO11
                         +----+----+
                     GND | 17 | 18 | unknown (docs say GND, but is 3.3V on my box)
                         +----+----+
                     TXD | 19 | 20 | RXD
                         +----+----+

Error_LED => GPIO20 which is pin 3. Other documents say it's pin 6, but none of them made my LED blink.

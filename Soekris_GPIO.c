#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>   /* Necessary because we use sysfs */
#include <linux/device.h>
#include <linux/io.h>

#define sysfs_dir  "tomas"
#define sysfs_file "gpio"

#define GPIO_BASE_CS5535   0x6600      /* rather than asking MSR_LBAR_GPIO which is harder to do */

#define SIO_INDEX            0x2e      /* index register of PC87366 Super I/O */
#define SIO_DATA             0x2f      /* data register of PC87366 Super I/O */

#define SIO_REG_SID          0x20      /* Configuration register containing ID */
#define SIO_SID_VALUE        0xe9      /* Expected ID value */

#define SIO_REG_LDN          0x07      /* Configuration register: logical device number selector */

#define SIO_LDN_GPIO         0x07      /* LDN Value to select GPIO configuration */

#define SIO_REG_ACTIVATE     0x30      /* (Shared) Device register: Activate */

#define SIO_REG_BASE_MSB_0   0x60      /* (Shared) Device register: I/O Port base, descriptor 0, MSB */
#define SIO_REG_BASE_LSB_0   0x61      /* (Shared) Device register: I/O Port base, descriptor 0, LSB */

#define SIO_GPIO_PIN_SELECT  0xf0      /* GPIO device-specific register: Pin select */
#define SIO_GPIO_PIN_CONFIG  0xf1      /* GPIO device-specific register: Pin configuration */

int gpio_base = 0;

void pin_clear(int pin);
void pin_set(int pin);
void pin_output_pullup_disable(int pin);
void pin_output_pullup_enable(int pin);
void pin_output_opendrain(int pin);
void pin_output_pushpull(int pin);
void pin_output_disable(int pin);
void pin_output_enable(int pin);
static int pinMap(int pin);
int pinRead(int pin);


static ssize_t
sysfs_store(struct device * dev,
            struct device_attribute * attr,
            const char * buffer,
            size_t count)
{
  // SAMPLES
        // "i 6"      would read the value of GPIO pin 6
        // "o 6 1"    would set pin 6
        // "c 6 i"    would set pin 6 to input (default)
        // "c 6 o"    would set pin 6 to output
        // "c 6 p 1"  would enable pull-up on pin 6 (default)
        // "c 6 p 0"  would disable pull-up on pin 6
        // "c 6 t 1"  would set output tipe to Push-pull on pin 6
        // "c 6 t 0"  would set output tipe to Open-Drain on pin 6 (default)
        char type;
        int pin;
        char val1;
        char val2;
        //type = -1;
        //pin  = -1;
        //val1 = -1;
        //val2 = -1;

    //printk(KERN_INFO "message from the user:'%s'\n", buffer);


    sscanf(buffer, "%c %d %c %c", &type, &pin, &val1, &val2);
    printk(KERN_INFO "your input %c %d %c %c\n", type, pin, val1, val2);
    if (type == 't')
      {
        int mpin = pinMap(pin);
        printk(KERN_INFO "mapped pin %d\n", mpin);
        return count;
      }
    // if first argument is "i"
    if (type == 'i')
    {
      // read the value of a pin
      int out = pinRead(pin);
      printk(KERN_INFO "Pin %d is set to %d\n", pin, out);
    }
    // if first argument is "o"
    else if (type == 'o')
      if (val1 == '1')
        pin_set(pin);
      else if (val1 == '0')
        pin_clear(pin);
      else
        printk(KERN_INFO "Wrong pin value, check User Manual\n");
    // if first argument is "c"
    else if (type == 'c')
    {
      // set/clear specified bit in config register
      if (val1 == 'i')
        pin_output_disable(pin);
      else if (val1 == 'o')
        pin_output_enable(pin);
      else if (val1 == 'p')
      {
        if (val2 == '1')
          pin_output_pullup_enable(pin);
        if (val2 == '0')
          pin_output_pullup_disable(pin);
      }
      else if (val1 == 't')
      {
        if (val2 == '1')
          pin_output_pushpull(pin);
        if (val2 == '0')
          pin_output_opendrain(pin);
      }
      else
        printk(KERN_INFO "Wrong configuration type, check User Manual\n");
    }
    else
      printk(KERN_INFO "Wrong action, check User Manual\n");
    return count;
}

                //(name, mode,    show,       store)
static DEVICE_ATTR(gpio, S_IWUGO, NULL, sysfs_store);

static struct kobject *gpio_obj = NULL;

static struct attribute *attrs[] = {
    &dev_attr_gpio.attr,
    NULL   /* need to NULL terminate the list of attributes */
};
static struct attribute_group attr_group = {
    .attrs = attrs,
};

// Initializes GPIO pins
static int pinInit(void)
{
    int i;

    outb(SIO_REG_SID, SIO_INDEX);	/* probe PC87366 */
	  i = inb(SIO_DATA);
	  if(i != SIO_SID_VALUE)
		  return -1;

    outb(SIO_REG_BASE_MSB_0, SIO_INDEX);	/* get MSB of GPIO I/O */
  	gpio_base = inb(SIO_DATA) << 8;
  	outb(SIO_REG_BASE_LSB_0, SIO_INDEX);	/* get LSB of GPIO I/O */
  	gpio_base |= inb(SIO_DATA);

    // Turn on GPIO

    outb(SIO_REG_LDN, SIO_INDEX);     /* logical device select */
  	outb(SIO_LDN_GPIO, SIO_DATA);     /* select GPIO */

    outb(SIO_REG_ACTIVATE, SIO_INDEX); /* activation for selected device, which is GPIO */
  	outb(0x01, SIO_DATA);              /* active */

    return 0;
}

// Initializes sysfs files
static int sysfsInit(void)
{
  int result = 0;
  //First we must create our kobject, which is a directory in /sys/...
  gpio_obj = kobject_create_and_add(sysfs_dir, kernel_kobj);
  if (gpio_obj == NULL)
  {
      printk (KERN_INFO "%s module failed to load: kobject_create_and_add failed\n", sysfs_file);
      return -ENOMEM;
  }

  /* now we create all files in the created subdirectory */
  result = sysfs_create_group(gpio_obj, &attr_group);
  if (result != 0)
  {
      /* creating files failed, thus we must remove the created directory! */
      printk (KERN_INFO "%s module failed to load: sysfs_create_group failed with result %d\n", sysfs_file, result);
      kobject_put(gpio_obj);
      return -ENOMEM;
  }

  printk(KERN_INFO "/sys/kernel/%s/%s created\n", sysfs_dir, sysfs_file);
  return result;
}

static int pinMap(int pin)
{
  /* mapping table */

	switch(pin)
	{
	case 3: return 32;
	case 4: return 33;
	case 5: return 34;
	case 6: return 35;
	case 7: return 36;
	case 8: return 37;
	case 9: return 38;
	case 10: return 39;
	case 12: return 4;
	case 13: return 5;
	case 15: return 19;
	case 16: return 18;
	default: return -1;
	}
}

static int pinConfig(int pin, int bit, int set)
{
  int output;
  int gpio_pin = pinMap(pin);

  if (gpio_base == 0)
    return -1;   /* not initialized */

  if (gpio_pin < 0)
  {
    printk(KERN_INFO "Selected pin does not exist\n");
    return -1;   /* selected pin does not exist */
  }

  //Select pin
  outb(SIO_GPIO_PIN_SELECT, SIO_INDEX);
  outb(gpio_pin, SIO_DATA); // not quite sure if this part is correct

  outb( SIO_GPIO_PIN_CONFIG, SIO_INDEX);
	output = inb(SIO_DATA);

	if(set)
	{
		output |= 1 << bit;    // set bit
	}
	else
	{
		output &= ~(1 << bit); // clear bit
	}

	outb(output, SIO_DATA);
  return 0;
}

void pin_output_enable(int pin)
{
	int i = pinConfig(pin, 0, 1);
  if (i >= 0)
    printk(KERN_INFO "Output enabled on pin %d\n", pin);
  else
    printk(KERN_INFO "Output was NOT enabled on pin %d\n", pin);
}

void pin_output_disable(int pin)
{
	int i = pinConfig(pin, 0, 0);
  if (i >= 0)
    printk(KERN_INFO "Input enabled on pin %d\n", pin);
  else
    printk(KERN_INFO "Input was NOT enabled on pin %d\n", pin);}

void pin_output_pushpull(int pin)
{
	int i = pinConfig(pin, 1, 1);
  if (i >= 0)
    printk(KERN_INFO "Push-pull enabled on pin %c\n", pin);
  else
    printk(KERN_INFO "Push-pull was NOT enabled on pin %c\n", pin);
}

void pin_output_opendrain(int pin)
{
	int i = pinConfig(pin, 1, 0);
  if (i >= 0)
    printk(KERN_INFO "Open-Drain enabled on pin %c\n", pin);
  else
    printk(KERN_INFO "Open-Drain was NOT enabled on pin %c\n", pin);
}

void pin_output_pullup_enable(int pin)
{
	int i = pinConfig(pin, 2, 1);
  if (i >= 0)
    printk(KERN_INFO "Pull-up enabled on pin %c\n", pin);
  else
    printk(KERN_INFO "Pull-up was NOT enabled on pin %c\n", pin);
}

void pin_output_pullup_disable(int pin)
{
	int i = pinConfig(pin, 2, 0);
  if (i >= 0)
    printk(KERN_INFO "Pull-up disabled on pin %c\n", pin);
  else
    printk(KERN_INFO "Pull-up was NOT enabled on pin %c\n", pin);
}

int pinRead(int pin)
{
  // Variables
  int gpio_pin;
  int offset;
  int gpio_value;
  // Map the pin
  gpio_pin = pinMap(pin);
  //gpio_pin = pin;
  // Check if GPIO is initialized
  if (gpio_base == 0)
    return -1; // Not initialized
  // Check if selected pin exists
  if (gpio_pin < 0){
    printk(KERN_INFO "Selected pin does not exist\n");
    return -1; // Selected pin is not in mapping
  }
  // Set the offset
  // =====================> Not sure about this part
  if(gpio_pin >= 32)
		offset = 0x09;
	else if(gpio_pin >= 18)
		offset = 0x05;
	else
		offset = 1;
  // Read GPIO value
  gpio_value = inb(gpio_base + offset);

  if ((gpio_value &= (1 <<(gpio_pin & 0x07))) > 0)
    return 1;
  else
    return 0;
}

// Sets pin to specified value (1 or 0)
int pinWrite(int pin, int set)
{
  // Variables
  int gpio_pin;
  int offset;
  int offset_in;
  int gpio_value;
  // Map the pin
  gpio_pin = pinMap(pin);
  //gpio_pin = pin;
  // Check if GPIO is initialized
  if (gpio_base == 0)
    return -1; // Not initialized
  // Check if selected pin exists
  if (gpio_pin < 0){
    printk(KERN_INFO "Selected pin does not exist\n");
    return -1; // Selected pin is not in mapping
  }
  // Set the offset
  // =====================> Not sure about this part
  if(gpio_pin >= 32){
		offset = 0x08;
    offset_in = 0x09;
  }
	else if(gpio_pin >= 18){
		offset = 0x04;
    offset_in = 0x05;
  }
	else{
		offset = 0x00;
    offset_in = 1;
  }
  // Read GPIO value
  gpio_value = inb(gpio_base + offset_in);
  // Change the bit that we need
  if (set)    //set pin
    gpio_value|= 1 <<(gpio_pin & 0x07);
  else        //clear pin
    gpio_value &= ~(1 << (gpio_pin & 0x07));
  // Whrite the new value to GPIO
  outb(gpio_value, gpio_base + offset);
  return 0;
}

// Sets pin
void pin_set(int pin)
{
	int i = pinWrite(pin, 1);
  if (i < 0)
    printk(KERN_INFO "Pin was not set\n");
  else
    printk(KERN_INFO "Pin was set\n");
}
// Clears pin
void pin_clear(int pin)
{
	int i = pinWrite(pin, 0);
  if (i < 0)
    printk(KERN_INFO "Pin was not cleared\n");
  else
    printk(KERN_INFO "Pin was cleader\n");
}

int __init gpio_init(void){
  printk(KERN_INFO "\n");
  int result = 0;
  result = sysfsInit();
  if (result == 0)
    pinInit();
  return result;
}

void __exit gpio_exit(void){
  kobject_put(gpio_obj);
  printk (KERN_INFO "/sys/kernel/%s/%s removed\n", sysfs_dir, sysfs_file);
}

module_init(gpio_init);
module_exit(gpio_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Tomas Aukstikalnis & Amir Kiumarsi");
MODULE_DESCRIPTION("GPIO use and configuration");

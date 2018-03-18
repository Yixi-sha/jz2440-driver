#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/io.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/cdev.h>



static volatile unsigned  long* gpfcon = NULL;
static volatile unsigned  long* gpfdat = NULL;
static dev_t led_major = 0;
static struct class* myLed_class = NULL;
static struct device* myLed_device = NULL;
static int led_min = 0;

static int myLed_open(struct inode *r_inode, struct file *r_file)
{
	led_min = MINOR(r_inode->i_rdev);/**get the minor of dev**/
	printk("led_minor is %d \n",led_min);
	printk( "this is myLed_open\n");
	switch (led_min)
	{
		case 0:
			(*gpfcon) &= (~( (0x3 << (4*2)) | (0x3 << (5*2)) | (0x3 << (6*2))));
			(*gpfcon) |= ( (0x1 << (4*2)) | (0x1 << (5*2)) | (0x1 << (6*2)));
			break;
		case 1:
			(*gpfcon) &= (~(0x3 << (4*2)));
			(*gpfcon) |= ( (0x1 << (4*2)));
			break;
		case 2:
			(*gpfcon) &= (~( 0x3 << (5*2)));
			(*gpfcon) |= ( (0x1 << (5*2)));
			break;
		case 3:
			(*gpfcon) &= (~( 0x3 << (6*2)));
			(*gpfcon) |= ( (0x1 << (6*2)));
			break;
	}
	
	return 0;
}

ssize_t  myLed_write (struct file* r_file, const char __user * r_user, size_t r_szie_t, loff_t * r_loff_t)
{
	int val = 0;
	int ret = 0;
	printk( "this is myLed_write\n ");
	ret = copy_from_user(&val, r_user,r_szie_t);
	printk( "val is %d \n ",val);
	
	if(val == 1)
	{		
		switch (led_min)
		{
		case 0:
			(*gpfdat) &= (~((0x1 << 4) | (0x1 << 5) | (0x1 << 6) ));
			break;
		case 1:
			(*gpfdat) &= (~((0x1 << 4)  ));
			break;
		case 2:
			(*gpfdat) &= (~((0x1 << 5)  ));
			break;
		case 3:
			(*gpfdat) &= (~((0x1 << 6)  ));
			break;
		}
	}
	else
	{
		
		switch (led_min)
		{
		case 0:
			(*gpfdat) |= ( (0x1 << 4) | (0x1 << 5) | (0x1 << 6) );
			break;
		case 1:
			(*gpfdat) |= ( (0x1 << 4)  );
			break;
		case 2:
			(*gpfdat) |= ( (0x1 << 5)  );
			break;
		case 3:
			(*gpfdat) |= ( (0x1 << 6)  );
			break;
		}
	}
	
	return 0;
}


/***define char driver file operation ***/
static struct file_operations myLed_fso = 
{
	.open		= myLed_open,
	.write      = myLed_write,

};

static struct cdev myledDev = {
	.owner = THIS_MODULE,
	.ops = &myLed_fso,
	};


static int leds_probe(struct platform_device *r_dev)
{
	int ret = 0;
	struct resource *res = platform_get_resource(r_dev, IORESOURCE_MEM, 0);
	printk("this probe\n");

	ret = alloc_chrdev_region(&led_major, 0, 1, "myButton");
	printk("button_major is %d \n", MAJOR(led_major));
	cdev_init(&myledDev, &myLed_fso);
	ret = cdev_add(&myledDev, led_major, 1);

	myLed_class = class_create(THIS_MODULE, "myLed"); /*create class*/
	if (IS_ERR(myLed_class))
		return PTR_ERR(myLed_class);

	myLed_device = device_create(myLed_class, NULL, MKDEV(MAJOR(led_major), 0), NULL, "myLed"); /*create device node*/
	if (IS_ERR(myLed_device))
		return PTR_ERR(myLed_device);


	gpfcon = (volatile unsigned long*)ioremap(res->start, res->end - res->start +1);
	gpfdat = gpfcon + 1;
	
	return 0;
}

static int leds_remove(struct platform_device *r_dev)
{
	printk("this remove\n");
	
	device_del(myLed_device);
	class_destroy(myLed_class);
	cdev_del(&myledDev);
	iounmap(gpfcon);
	return 0;
}

/*platform driver*/
static struct platform_driver gpio_leds_device_driver = {
	.probe		= leds_probe,
	.remove		= leds_remove,
	.driver		= {
		.name	= "mygpio-leds",
		.owner	= THIS_MODULE,
	}
};


static int __init gpio_leds_init(void)
{
	return platform_driver_register(&gpio_leds_device_driver);
}

static void __exit gpio_leds_exit(void)
{
	platform_driver_unregister(&gpio_leds_device_driver);
}

MODULE_LICENSE("GPL");
module_init(gpio_leds_init);
module_exit(gpio_leds_exit);




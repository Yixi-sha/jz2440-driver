#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/io.h>
#include <asm/uaccess.h>



static struct class* myLed_class = NULL;
static struct device* myLed_device = NULL;

static int led_major = 0;
static volatile unsigned  long* gpfcon = NULL;
static volatile unsigned  long* gpfdat = NULL;
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



/*** init function   **/
static int myLed_init (void)
{
	int minor = 0;
	led_major = register_chrdev(0, "yixi-sha led", &myLed_fso); /*  register char driver */
	printk("register_chrdev ret is %d \n", led_major);

	myLed_class = class_create(THIS_MODULE, "myLed"); /*create class*/
	if (IS_ERR(myLed_class))
		return PTR_ERR(myLed_class);

	myLed_device = device_create(myLed_class, NULL, MKDEV(led_major, 0), NULL, "myLed"); /*create device node*/
	if (IS_ERR(myLed_device))
		return PTR_ERR(myLed_device);

	for(minor = 1; minor <= 3; minor++)
	{
		myLed_device = device_create(myLed_class, NULL, MKDEV(led_major, minor), NULL, "myLed%d",minor); /*create device node*/
		if (IS_ERR(myLed_device))
			return PTR_ERR(myLed_device);
	}

	gpfcon = (volatile unsigned long*)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;
	
	return 0;
}

static void  myLed_exit (void) 
{
	unregister_chrdev(led_major, "yixi-sha led");

	device_del(myLed_device);
	class_destroy(myLed_class);
	iounmap(gpfcon);
}

MODULE_LICENSE("GPL");
module_init(myLed_init); /*define driver init */
module_exit(myLed_exit);





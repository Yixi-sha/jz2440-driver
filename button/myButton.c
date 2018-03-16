#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/io.h>
#include <asm/uaccess.h>

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/ioport.h>
#include <mach/gpio-fns.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/signal.h>
#include <linux/semaphore.h>
#include <linux/atomic.h>
#include <linux/timer.h>


static DECLARE_WAIT_QUEUE_HEAD(myButton_wait); /**wait queue**/
static volatile int ev_press = 0;


static dev_t button_major = 0;

static struct class* myButton_class = NULL;
static struct device* myButton_device = NULL;
static atomic_t  fCount  = ATOMIC_INIT(0);

static struct timer_list myButtonTimer;

struct pinDesc{
	unsigned int pin;
	unsigned int keyVal;
};

struct pinDesc pinsDes[4] = {
	{S3C2410_GPF(0), 0x00},
	{S3C2410_GPF(2), 0x00},
	{S3C2410_GPG(3), 0x00},
	{S3C2410_GPG(11), 0x00},
};

static unsigned char keyVal[4] = 
	{
		0, 0, 0, 0,
	};

static struct fasync_struct *myButton_fasync;


static struct pinDesc *irqPin = NULL;




static irqreturn_t myButton_irq(int irq, void *dev_id)
{
	/**10ms  HZ is 1s*/\
	irqPin = dev_id;				   	
	mod_timer(&myButtonTimer, jiffies + HZ/100);


	return IRQ_HANDLED;
}

static int myButton_open(struct inode *r_inode, struct file *r_file)
{
	int ret = 0;
	
	if(atomic_inc_return(&fCount) == 1)
	{
		ret = request_irq(IRQ_EINT0, myButton_irq,IRQ_TYPE_EDGE_BOTH, "S0", &pinsDes[0]);
		if(ret)
		{
			free_irq(IRQ_EINT0, &pinsDes[0]);
			return -EINVAL;
			
		}
		ret = request_irq(IRQ_EINT2, myButton_irq,IRQ_TYPE_EDGE_BOTH, "S2", &pinsDes[1]);
		if(ret)
		{
			free_irq(IRQ_EINT2, &pinsDes[1]);
			return -EINVAL;
		}
		ret = request_irq(IRQ_EINT11, myButton_irq,IRQ_TYPE_EDGE_BOTH, "S11", &pinsDes[2]);
		if(ret)
		{
			free_irq(IRQ_EINT11, &pinsDes[2]);
			return -EINVAL;
		}
		ret = request_irq(IRQ_EINT19, myButton_irq,IRQ_TYPE_EDGE_BOTH, "S19", &pinsDes[3]);
		if(ret)
		{
			free_irq(IRQ_EINT19, &pinsDes[3]);
			return -EINVAL;
		}
	}
	printk( "this is myLed_open f_count is %d\n",atomic_read(&fCount));
	return 0;
}

static ssize_t myButton_write (struct file *r_file, const char __user *r_user, size_t r_size_t, loff_t *r_loff_t)
{
	return 0;
}


static ssize_t myButton_read (struct file *r_lfie, char __user *r_user, size_t r_size_t, loff_t * r_loff_t)
{
	
	int ret = 0;
	if(r_size_t != sizeof(keyVal))
	{
		return -EINVAL;
	}
//	wait_event_interruptible(myButton_wait, ev_press);
	ret = copy_to_user(r_user, keyVal, sizeof(keyVal));
	ev_press = 0;
	return sizeof(keyVal);
}

static int myButton_release (struct inode *r_inode, struct file *r_file)
{
	
	if(atomic_dec_and_test(&fCount))
	{
		free_irq(IRQ_EINT0, &pinsDes[0]);
		free_irq(IRQ_EINT2, &pinsDes[1]);
		free_irq(IRQ_EINT11, &pinsDes[2]);
		free_irq(IRQ_EINT19, &pinsDes[3]);
	}
	

	printk( "this is myLed_close f_count is %d\n",atomic_read(&fCount));
	return 0;
}

/***poll ***/

unsigned int myButton_poll (struct file *r_file, struct poll_table_struct *r_poll_table_struct)
{
	unsigned int mask = 0;;

	poll_wait(r_file, &myButton_wait, r_poll_table_struct);

	if(ev_press)
	{
		mask |= POLLIN | POLLRDNORM;
	}

	return mask;
}

static int myButton_Fasync(int r_fd, struct file *r_file, int r_on)
{
	printk("button_major is myButton_Fasync \n");
	return fasync_helper(r_fd, r_file, r_on, &myButton_fasync);
}

/***define char driver file operation ***/
static struct file_operations mybutton_fso = 
{
	.open  = myButton_open,
	.release = myButton_release,
	.write = myButton_write,
	.read  = myButton_read,
	.poll = myButton_poll,
	.fasync = myButton_Fasync,
	.llseek = no_llseek,
};

struct cdev myButtonDev = {
	.owner = THIS_MODULE,
	.ops = &mybutton_fso,
	};



static void myButton_timer(unsigned long data)
{
	unsigned int status = 0;
	if(irqPin == NULL)
	{
		return;
	}
	status = s3c2410_gpio_getpin(irqPin->pin);

	
	
	if(irqPin->pin == S3C2410_GPF(0))
	{
		if(status)
			keyVal[0] = 0;
		else
			keyVal[0] = 1;
	}
	else if(irqPin->pin == S3C2410_GPF(2))
	{
		if(status)
			keyVal[1] = 0;
		else
			keyVal[1] = 1;
	}
	else if(irqPin->pin == S3C2410_GPG(3))
	{
		if(status)
			keyVal[2] = 0;
		else
			keyVal[2] = 1;
	}
	else if(irqPin->pin == S3C2410_GPG(11))
	{
		if(status)
			keyVal[3] = 0;
		else
			keyVal[3] = 1;
	}
	//printk(" %d %d %d %d ", keyVal[0], keyVal[1], keyVal[2], keyVal[3]);
	ev_press = 1; 
//	wake_up_interruptible_all(&myButton_wait);   
	irqPin = NULL;
	kill_fasync(&myButton_fasync, SIGIO, POLLIN);	
}


/*** init function   **/
static int __init myButton_init (void)
{
	int ret;
	button_major = 0;
	/**system auto 256 minor num of dev_t**/
	/*button_major = register_chrdev(button_major, "yixi-sha Button", &mybutton_fso); */ /*  register char driver */

	/*timer*/
	init_timer(&myButtonTimer);
	myButtonTimer.function = myButton_timer;		/* add timer handler function*/
	add_timer(&myButtonTimer); /***add a timer**/
	
	ret = alloc_chrdev_region(&button_major, 0, 1, "myButton");
	printk("button_major is %d \n", MAJOR(button_major));
	cdev_init(&myButtonDev, &mybutton_fso);
	ret = cdev_add(&myButtonDev, button_major, 1);
	
	if(MAJOR(button_major) < 0)
	{
		return button_major;
	}
	myButton_class = class_create(THIS_MODULE, "myButton"); /*create class*/
	if (IS_ERR(myButton_class))
		return PTR_ERR(myButton_class);

	myButton_device = device_create(myButton_class, NULL, MKDEV(MAJOR(button_major), 0), NULL, "myButton"); /*create device node*/
	if (IS_ERR(myButton_device))
		return PTR_ERR(myButton_device);

	
	
	return 0; 
}

static void  __exit myButton_exit (void) 
{
	del_timer(&myButtonTimer);
	device_del(myButton_device);
	class_destroy(myButton_class);
	/*unregister_chrdev(button_major, "yixi-sha Button");*/
	cdev_del(&myButtonDev);
	unregister_chrdev_region(button_major, 1);
	
}


MODULE_LICENSE("GPL");
module_init(myButton_init); /*define driver init */
module_exit(myButton_exit);



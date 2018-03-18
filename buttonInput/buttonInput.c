#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/timer.h>




static struct timer_list myButtonTimer;
static struct input_dev *inputButton = NULL;
static struct pinDesc *irqPin = NULL;

struct pinDesc{
	unsigned int pin;
	unsigned int keyVal;
	unsigned int keyName;
};

static struct pinDesc pinsDes[4] = {
	{S3C2410_GPF(0), 0x00, KEY_L},
	{S3C2410_GPF(2), 0x00, KEY_S},
	{S3C2410_GPG(3), 0x00, KEY_ENTER},
	{S3C2410_GPG(11), 0x00, KEY_LEFTSHIFT},
};

static irqreturn_t myButton_irq(int irq, void *dev_id)
{
	/**10ms  HZ is 1s*/
	irqPin = dev_id;				   	
	mod_timer(&myButtonTimer, jiffies + HZ/100);
	return IRQ_HANDLED;
}


static int init_irq(void)
{
	int ret = request_irq(IRQ_EINT0, myButton_irq,IRQ_TYPE_EDGE_BOTH, "S0", &pinsDes[0]);
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

	return ret;
}

static void myButton_timer(unsigned long data)
{
	unsigned int status = 0;
	if((irqPin == NULL) || (inputButton == NULL))
	{
		return;
	}
	status = s3c2410_gpio_getpin(irqPin->pin);	
	if(irqPin->pin == S3C2410_GPF(0))
	{
		if(status)
			input_event(inputButton, EV_KEY, irqPin->keyName, 0);
		else
			input_event(inputButton, EV_KEY, irqPin->keyName, 1);
	}
	else if(irqPin->pin == S3C2410_GPF(2))
	{
		if(status)
			input_event(inputButton, EV_KEY, irqPin->keyName, 0);
		else
			input_event(inputButton, EV_KEY, irqPin->keyName, 1);
	}
	else if(irqPin->pin == S3C2410_GPG(3))
	{
		if(status)
			input_event(inputButton, EV_KEY, irqPin->keyName, 0);
		else
			input_event(inputButton, EV_KEY, irqPin->keyName, 1);
	}
	else if(irqPin->pin == S3C2410_GPG(11))
	{
		if(status)
			input_event(inputButton, EV_KEY, irqPin->keyName, 0);
		else
			input_event(inputButton, EV_KEY, irqPin->keyName, 1);
	}
	input_sync(inputButton);
	irqPin = NULL;
}


static int __init buttonInput_init(void)
{
	inputButton = input_allocate_device();
	if(inputButton == NULL)
	{
		printk("input_allocate_device fail \n");
		
		return -ENOMEM; 
	}

	inputButton->name = "yixi-sha input test";
	/*key event*/
	__set_bit(EV_KEY, inputButton->evbit);
	__set_bit(EV_REP,inputButton->evbit); /*repeat*/
	/*set key event*/
	__set_bit(KEY_L,inputButton->keybit);
	__set_bit(KEY_S,inputButton->keybit);
	__set_bit(KEY_ENTER,inputButton->keybit);
	__set_bit(KEY_LEFTSHIFT,inputButton->keybit);	
	

	/*register*/
	if(input_register_device(inputButton))
	{
		input_free_device(inputButton);
		printk("input_register_device fail \n");
		return  -ENOMEM;
	}

	init_timer(&myButtonTimer);
	myButtonTimer.function = myButton_timer;		/* add timer handler function*/
	add_timer(&myButtonTimer); /***add a timer**/
	
	if(init_irq() != 0)
	{
		input_free_device(inputButton);
		printk("init irq fail \n");
		return  -ENOMEM;
	}
	
	
	return 0;
}


static void __exit buttonInput_Exit(void)
{
	free_irq(IRQ_EINT0, &pinsDes[0]);
	free_irq(IRQ_EINT2, &pinsDes[1]);
	free_irq(IRQ_EINT11, &pinsDes[2]);
	free_irq(IRQ_EINT19, &pinsDes[3]);
	del_timer(&myButtonTimer);
	input_unregister_device(inputButton);
	input_free_device(inputButton);
}


MODULE_LICENSE("GPL");
module_init(buttonInput_init);
module_exit(buttonInput_Exit);


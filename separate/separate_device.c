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

static struct resource s3c2440_led_resource[] = 
{
	[0] = 
		{
			.start = 0x56000050,
			.end   = 0x56000050 + 8 -1,
			.flags = IORESOURCE_MEM,
		},
};

void	led_release(struct device *r_dev)
{
	printk("this release\n");
}


static struct platform_device led_dev = 
{
	.name = "mygpio-leds",
	.id   = -1,
	.num_resources = ARRAY_SIZE(s3c2440_led_resource),
	.resource = s3c2440_led_resource,
	.dev = 
		{
			.release = led_release,
		},
};

static int __init gpio_leds_init(void)
{
	return platform_device_register(&led_dev);
}

static void __exit gpio_leds_exit(void)
{
	platform_device_unregister(&led_dev);
}

MODULE_LICENSE("GPL");
module_init(gpio_leds_init);
module_exit(gpio_leds_exit);



#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>



static struct input_dev *input_dev;
static int pipe, maxp;
struct usb_mouse {
	struct urb *irq;

	signed char *data;
	dma_addr_t data_dma;
};
static struct usb_mouse *mouse;



static void usb_mouse_irq(struct urb *urb)
{
	int i = 0;
	static int cnt = 0;
	printk("date cunt %d : \n",++cnt);
	for( i = 0;i<maxp; i++)
	{
		printk("%02x  ",mouse->data[i]);
	}
	printk("\n");

	
	usb_submit_urb(mouse->irq, GFP_KERNEL);
}

static int usb_mouse_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	
	
	printk("this is my usb probe\n");
	interface = intf->cur_altsetting;
	/*judge*/
	if (interface->desc.bNumEndpoints != 1)
			return -ENODEV;
	endpoint = &interface->endpoint[0].desc;
		if (!usb_endpoint_is_int_in(endpoint))
			return -ENODEV;

	/*input dev*/
	input_dev = input_allocate_device();	
	if(input_dev == NULL)
		{
			printk("input_allocate_device fail \n");
			
			return -ENOMEM; 
		}
	input_dev->name = "yixi-sha usb test";
	/*key event*/
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(EV_REP,input_dev->evbit); /*repeat*/
	/*set key event*/
	__set_bit(KEY_L,input_dev->keybit);
	__set_bit(KEY_S,input_dev->keybit);
	__set_bit(KEY_ENTER,input_dev->keybit);

	
	
	if(input_register_device(input_dev))
	{
		input_free_device(input_dev);
		printk("input_register_device fail \n");
		return  -ENOMEM;
	}



	/*date transtate 3 source distion length*/
	/*source : some point*/
	pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
	/*length*/
	maxp = usb_maxpacket(dev, pipe, usb_pipeout(pipe));
	/*distion*/
	mouse = kzalloc(sizeof(struct usb_mouse), GFP_KERNEL);
	if (!mouse)
	{
		kfree(mouse);
		input_free_device(input_dev);
		printk("kzalloc fail \n");
		return  -ENOMEM;
	}
	mouse->data = usb_alloc_coherent(dev, 8, GFP_ATOMIC, &mouse->data_dma);
	if (!mouse->data)
	{
		kfree(mouse);
		input_free_device(input_dev);
		printk("kzalloc fail \n");
		return  -ENOMEM;
	}
	
	/*set urb*/

	mouse->irq = usb_alloc_urb(0, GFP_KERNEL);
	if (!mouse->irq)
	{
		kfree(mouse);
		input_free_device(input_dev);
		printk("kzalloc fail \n");
		return  -ENOMEM;
	}


	usb_to_input_id(dev, &input_dev->id);

	usb_fill_int_urb(mouse->irq, dev, pipe, mouse->data,maxp, \
			 usb_mouse_irq, mouse, endpoint->bInterval);
	mouse->irq->transfer_dma = mouse->data_dma;
	mouse->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	usb_set_intfdata(intf, mouse);

	if (usb_submit_urb(mouse->irq, GFP_KERNEL))
		return -EIO;
	
	
	return 0;
}


static void usb_mouse_disconnect(struct usb_interface *intf)
{

	struct usb_mouse *mouse = usb_get_intfdata (intf);

	usb_set_intfdata(intf, NULL);
	if (mouse) {
		usb_kill_urb(mouse->irq);
		usb_free_urb(mouse->irq);
		usb_free_coherent(interface_to_usbdev(intf), 8, mouse->data, mouse->data_dma);
		kfree(mouse);
		printk("this is my usb disconnect\n");
		input_unregister_device(input_dev);
		input_free_device(input_dev);
		printk("this is my usb disconnect\n");
	}
	
}


static struct usb_device_id usb_mouse_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },
	
};


//MODULE_DEVICE_TABLE (usb, usb_mouse_id_table);

static struct usb_driver usb_mouse_driver = {
	.name		= "usbmouse",
	.probe		= usb_mouse_probe,
	.disconnect	= usb_mouse_disconnect,
	.id_table	= usb_mouse_id_table,
};



/*static int __init usb_myMouse_init(void)
{
	 
	return usb_register(&usb_mouse_driver);	
}
static void __exit usb_myMouse_exit(void)
{
	usb_deregister(&usb_mouse_driver);
}*/

//module_init(usb_myMouse_init);
//module_exit(usb_myMouse_exit);
MODULE_LICENSE("GPL");

module_usb_driver(usb_mouse_driver);

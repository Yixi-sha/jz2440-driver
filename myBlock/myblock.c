#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/mutex.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/gfp.h>

#include <asm/uaccess.h>
#include <asm/dma.h>

static int major;

static struct gendisk *myram_gendisk;
static struct request_queue *myram_queue;
static DEFINE_SPINLOCK(myram_lock);

static unsigned char *ramblock_buf;

#define RAMBLOCK_SIZE (1024 * 1024)


static void do_ram_request(struct request_queue *q)
{
	struct request *req;
	static int r_cnt = 0;
	static int w_cnt = 0;
	
	req = blk_fetch_request(q);
	while(req)
	{
		unsigned long start = blk_rq_pos(req) *512;
		unsigned long len  = req->__data_len;

		if(rq_data_dir(req) == READ)
		{
			memcpy(req->buffer,ramblock_buf + start, len);
			printk("r_cnt is %d \n",++r_cnt);
		}
		else
		{
			memcpy(ramblock_buf + start, req->buffer, len);
			printk("w_cnt is %d \n",++w_cnt);
		}
				
		if (!__blk_end_request_cur(req, 0))
			req = blk_fetch_request(q);
			
	}
}



static int mygetgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	geo->heads     = 2;
	geo->cylinders = 32;
	geo->sectors   = RAMBLOCK_SIZE/2/32/512;
	return 0;
}


static const struct block_device_operations block_fops =
{
	.owner		= THIS_MODULE,
	.getgeo     = mygetgeo,
};

static int __init myRamBlock_init(void)
{

	major = register_blkdev(0, "yixiram");
	if(major < 0)
	{
		 return -ENOMEM;
	}

    myram_gendisk = alloc_disk(16);
	if (!myram_gendisk)
	{
		unregister_blkdev(major, "yixiram");
	}

 
	myram_queue = blk_init_queue(do_ram_request, &myram_lock);
	if (!myram_queue)
	{
		printk("fail fail\n");
	}
	myram_gendisk->queue = myram_queue;
	
	myram_gendisk->major = major;
    myram_gendisk->first_minor = 0; 
    sprintf(myram_gendisk->disk_name, "yixiram");
	myram_gendisk->fops = &block_fops;
    
	set_capacity(myram_gendisk, RAMBLOCK_SIZE / 512);

	ramblock_buf = kzalloc(RAMBLOCK_SIZE, GFP_KERNEL);
	if(ramblock_buf == NULL)
	{
		printk("fail fail\n");
	}
	
	add_disk(myram_gendisk);
	printk("myRamBlock_init  \n");
	return 0;
}

static void __exit myRamBlock_exit(void)
{
	del_gendisk(myram_gendisk);
    put_disk(myram_gendisk);
	unregister_blkdev(major, "yixiram");
	blk_cleanup_queue(myram_queue);
	printk("myRamBlock_exit  \n");
	kfree(ramblock_buf);
}




MODULE_LICENSE("GPL");
module_init(myRamBlock_init);
module_exit(myRamBlock_exit);

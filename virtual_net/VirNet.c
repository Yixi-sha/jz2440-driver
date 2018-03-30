
/*
 * 参考 drivers\net\cs89x0.c
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <linux/ip.h>

static struct net_device *vnet_dev;





static struct net_device_stats *net_get_stats(struct net_device *dev)
{
	return &dev->stats;
}

static int set_mac_address(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;

	if (netif_running(dev))
		return -EBUSY;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

	printk("set mac is %s",addr->sa_data);

	return 0;
}

static void net_rx(struct net_device *dev,struct sk_buff *skb)
{
	unsigned char *type;
	struct iphdr *ih;
	__be32 *saddr, *daddr, tmp;
	unsigned char	tmp_dev_addr[ETH_ALEN];
	struct ethhdr *ethhdr;
		
	struct sk_buff *rx_skb;
	printk("yixi - 1 \n");
			
		
		/* change "source /disttion" mac   adders */
	ethhdr = (struct ethhdr *)skb->data;
	memcpy(tmp_dev_addr, ethhdr->h_dest, ETH_ALEN);
	memcpy(ethhdr->h_dest, ethhdr->h_source, ETH_ALEN);
	memcpy(ethhdr->h_source, tmp_dev_addr, ETH_ALEN);
	printk("yixi - 2 \n");
		/*  */    
	ih = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
	saddr = &ih->saddr;
	daddr = &ih->daddr;
	
	tmp = *saddr;
	*saddr = *daddr;
	*daddr = tmp;
		
	//((u8 *)saddr)[2] ^= 1; /* change the third octet (class C) */
	//((u8 *)daddr)[2] ^= 1;
	type = skb->data + sizeof(struct ethhdr) + sizeof(struct iphdr);
	//printk("tx package type = %02x\n", *type);
	// 
	*type = 0; /* 0 define reply */
	printk("yixi - 3 \n");	
	ih->check = 0;		   /* and rebuild the checksum (ip needs it) */
	ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);
		
		// consruct sk_buff
	rx_skb = dev_alloc_skb(skb->len + 2);
	skb_reserve(rx_skb, 2); /* align IP on 16B boundary */	
	memcpy(skb_put(rx_skb, skb->len), skb->data, skb->len);
	printk("yixi - 4 \n");
	/* Write metadata, and then pass to the receive level */
	rx_skb->dev = dev;
	rx_skb->protocol = eth_type_trans(rx_skb, dev);
	rx_skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
	printk("yixi - 5 \n");

	
	netif_rx(rx_skb);
	dev->stats.rx_packets++;
	dev->stats.rx_bytes += skb->len;
}


static netdev_tx_t	myndo_start_xmit (struct sk_buff *skb, struct net_device *dev)
{

	static int cnt = 0;
	printk("cnt is %d \n",cnt++);
	netif_stop_queue(dev);
	dev->stats.tx_bytes += skb->len;
	dev->stats.tx_packets ++;
	

	net_rx(dev,skb);

	dev_kfree_skb (skb);
	netif_wake_queue(dev);
	
	return NETDEV_TX_OK;
}

static const struct net_device_ops net_ops = 
{
	.ndo_start_xmit = myndo_start_xmit,
	.ndo_get_stats		= net_get_stats,
	.ndo_set_mac_address 	= set_mac_address,
};



static int virt_net_init(void)
{

	vnet_dev = alloc_etherdev(0);  /* alloc_etherdev */

	vnet_dev->netdev_ops	= &net_ops;
	vnet_dev->flags         |= IFF_NOARP;
	

	register_netdev(vnet_dev);
	
	return 0;
}

static void virt_net_exit(void)
{
	unregister_netdev(vnet_dev);
	free_netdev(vnet_dev);
}

module_init(virt_net_init);
module_exit(virt_net_exit);


MODULE_LICENSE("GPL");


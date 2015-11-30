#include <linux/init.h>
#include <linux/module.h>

#include <linux/kernel.h>
#include <linux/fs.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
//#include <mach/gpio-bank.h>
#include <mach/regs-gpio.h>
#include <asm/io.h>
#include <linux/regulator/consumer.h>
//#include "gps.h"
#include <linux/delay.h>

#define RST_DEBUG
#ifdef RST_DEBUG
#define DPRINTK(x...) printk("RST_CTL DEBUG:" x)
#else
#define DPRINTK(x...)
#endif

#define DRIVER_NAME "rst"

static int rst_gpios[] = {
EXYNOS4_GPZ(6),
EXYNOS4_GPL0(2),

};
#define RST_NUM		ARRAY_SIZE(rst_gpios)

int rst_open(struct inode *inode,struct file *filp)
{
	DPRINTK("Device Opened Success!\n");
	return nonseekable_open(inode,filp);
}

int rst_close(struct inode *inode,struct file *filp)
{
	DPRINTK("Device Closed Success!\n");
	return 0;
}

int rst_pm(bool enable)
{
	int ret = 0;
	printk("debug: RST PM return %d\r\n" , ret);
	return ret;
};

long rst_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
	printk("debug: rst_ioctl cmd is %d\n" , cmd);

	switch(cmd)
	{
		case 0:
			//gpio_set_value(rst_gpios, 0);
			//break;
		case 1:
			//gpio_set_value(rst_gpios, 1);
			if(arg>RST_NUM)
				{
				   return -EINVAL;
				}
			gpio_set_value(rst_gpios[arg], cmd);
			break;

		default:
			return -EINVAL;
	}

	return 0;
}

static struct file_operations rst_ops = {
	.owner 	= THIS_MODULE,
	.open 	= rst_open,
	.release= rst_close,
	.unlocked_ioctl = rst_ioctl,
};

static struct miscdevice rst_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.fops	= &rst_ops,
	.name	= "rst",
};


static int rst_probe(struct platform_device *pdev)
{
	int ret,i;
	char *banner = "rst Initialize\n";

	printk(banner);
        for(i=0;i<RST_NUM;i++)
		{
		    ret = gpio_request(rst_gpios[i], "RST");//recommend gpio		 
            if(ret)
			{
			      printk("%s: request GPIO %d for rst failed, ret = %d\n", DRIVER_NAME,
					rst_gpios[i], ret);
			 return ret;
		   }

		  s3c_gpio_cfgpin(rst_gpios[i], S3C_GPIO_OUTPUT);//set as output
		 // gpio_set_value(rst_gpios[i],0);//init  as close
       	}
         gpio_set_value(rst_gpios[0],0);//init bt vreg as low
         gpio_set_value(rst_gpios[1],1);//init TP_EN as high
	ret = misc_register(&rst_dev);
	if(ret<0)
	{
		printk("rst:register device failed!\n");
		goto exit;
	}
	
	
	printk("rst:register device sucess!\n");
	
	return 0;

exit:
	misc_deregister(&rst_dev);
	return ret;
}

static int rst_remove (struct platform_device *pdev)
{
	misc_deregister(&rst_dev);	

	return 0;
}

static int rst_suspend (struct platform_device *pdev, pm_message_t state)
{
	DPRINTK("rst suspend:power off!\n");
	return 0;
}

static int rst_resume (struct platform_device *pdev)
{
	DPRINTK("rst resume:power on!\n");
	return 0;
}

static struct platform_driver rst_driver = {
	.probe = rst_probe,
	.remove = rst_remove,
	.suspend = rst_suspend,
	.resume = rst_resume,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static void __exit rst_exit(void)
{
	platform_driver_unregister(&rst_driver);
}

static int __init rst_init(void)
{
	return platform_driver_register(&rst_driver);
}

module_init(rst_init);
module_exit(rst_exit);

MODULE_LICENSE("Dual BSD/GPL");

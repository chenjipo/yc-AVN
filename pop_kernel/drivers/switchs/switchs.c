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

#define SWITCH_DEBUG
#ifdef SWITCH_DEBUG
#define printk(x...) printk("SWITCH_CTL DEBUG:" x)
#else
#define printk(x...)
#endif

#define DRIVER_NAME "switchs"
#define RESET3G_GPIO	9

static int switch_gpios[] = {
	EXYNOS4_GPX1(4),	//AUDIO_SWITCH_A
	EXYNOS4_GPK3(0),	//AUDIO_SWITCH_B
	EXYNOS4_GPK3(3),	//AUDIO_SWITCH_C
	EXYNOS4_GPK3(5),	//AUDIO_SWITCH_D
	EXYNOS4_GPZ(5),		//VEDIO_SWITCH_A
	EXYNOS4_GPK3(1),	//VEDIO_SWITCH_B
	EXYNOS4_GPK3(4),	//VEDIO_SWITCH_C
	EXYNOS4_GPK3(6),	//VEDIO_SWITCH_D
	EXYNOS4_GPX1(1),	//GONGFANG
	EXYNOS4_GPK1(0),	//3G-RESET
};

#define SWITCH_NUM		ARRAY_SIZE(switch_gpios)

int switchs_open(struct inode *inode,struct file *filp)
{
	//printk("Device Opened Success!\n");
	return nonseekable_open(inode,filp);
}

int switchs_release(struct inode *inode,struct file *filp)
{
	//printk("Device Closed Success!\n");
	return 0;
}

int switchs_pm(bool enable)
{
	int ret = 0;
	printk("debug: SWITCH PM return %d\r\n" , ret);
	return ret;
};

long switchs_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
	switch(cmd)
	{
		case 0:
		case 1:
			if (arg > SWITCH_NUM) {
				return -EINVAL;
			}

			gpio_set_value(switch_gpios[arg], cmd);
			break;

		default:
			return -EINVAL;
	}

	printk("switchs_ioctl arg = %d , cmd = %d\n" , arg , cmd);

	return 0;
}

static struct file_operations switchs_ops = {
	.owner 	= THIS_MODULE,
	.open 	= switchs_open,
	.release= switchs_release,
	.unlocked_ioctl 	= switchs_ioctl,
};

static struct miscdevice switchs_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.fops	= &switchs_ops,
	.name	= "switchs",
};


static int switchs_probe(struct platform_device *pdev)
{
	int ret, i;
	char *banner = "switchs Initialize\n";

//	printk(banner);

	for(i=0; i<SWITCH_NUM; i++)
	{
		ret = gpio_request(switch_gpios[i], "SWITCH");
		if (ret) {
			printk("%s: request GPIO %d for SWITCH failed, ret = %d\n", DRIVER_NAME,
					switch_gpios[i], ret);
			return ret;
		}
		s3c_gpio_cfgpin(switch_gpios[i], S3C_GPIO_OUTPUT);
		//gpio_set_value(switch_gpios[i], 0);
	}

	for(i=0; i<SWITCH_NUM; i++)
    {   
		if(i != RESET3G_GPIO)
		gpio_set_value(switch_gpios[i], 0); 
    }
	
	gpio_set_value(switch_gpios[RESET3G_GPIO], 1);	//DC33_EN init high level

	ret = misc_register(&switchs_dev);
	if(ret<0)
	{
		printk("switchs:register device failed!\n");
		goto exit;
	}
	
	
	printk("switchs:register device sucess!\n");
	
	return 0;

exit:
	misc_deregister(&switchs_dev);
	return ret;
}

static int switchs_remove (struct platform_device *pdev)
{
	misc_deregister(&switchs_dev);	

	return 0;
}

static int switchs_suspend (struct platform_device *pdev, pm_message_t state)
{
	printk("switchs suspend:power off!\n");
	return 0;
}

static int switchs_resume (struct platform_device *pdev)
{
	printk("switchs resume:power on!\n");
	return 0;
}

static struct platform_driver switchs_driver = {
	.probe = switchs_probe,
	.remove = switchs_remove,
	.suspend = switchs_suspend,
	.resume = switchs_resume,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static void __exit switchs_exit(void)
{
	platform_driver_unregister(&switchs_driver);
}

static int __init switchs_init(void)
{
	int ret = 0 ;
	
	ret =  platform_driver_register(&switchs_driver);
	if(ret == 0)
		printk("register switchs succeed\n");
	else
		printk("register switchs failed\n");

	return ret;
}

module_init(switchs_init);
module_exit(switchs_exit);

MODULE_LICENSE("Dual BSD/GPL");

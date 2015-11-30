#include "stm_isp_dev.h"


struct file_operations stm_isp_dev_FileOps = 
{
	.owner = THIS_MODULE,
//	.open = stm_isp_open,
	.unlocked_ioctl = stm_isp_ioctl,
//	.read = stm_isp_read,
//	.write = stm_isp_write,
//	.release = stm_isp_release,
};



static int __init stm_isp_init(void)
{
	int res = 0;

	struct class *myclass = class_create(THIS_MODULE, "stm_isp");

	s3c_gpio_cfgpin(MCU_BOOT, S3C_GPIO_OUTPUT); //boot	
	s3c_gpio_cfgpin(MCU_RST, S3C_GPIO_OUTPUT); //rst

	gpio_set_value(MCU_BOOT, 0);
	gpio_set_value(MCU_RST, 0);

	res = register_chrdev(0, "stm_isp", &stm_isp_dev_FileOps);	
	device_create(myclass, NULL, MKDEV(res, 0), NULL, "stm_isp");
	printk("Kernel:STM_ISP Devices Init OK!\r\n");
	return 0;
}


static void __exit stm_isp_exit(void)
{
	gpio_set_value(MCU_BOOT, 0);
	gpio_set_value(MCU_RST, 1);

//	stm_isp_dev_FileOps.release();
}


module_init(stm_isp_init);
module_exit(stm_isp_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("HZR_XHH ~~ !*o*!\n");





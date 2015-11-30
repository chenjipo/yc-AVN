#include "stm_isp_ctl.h"



long stm_isp_ioctl(struct file *filp,unsigned int cmd,unsigned long arg)
{
	printk("debug: STM_ISP Control Cmd is %d\n", cmd);

	switch (cmd)
	{
		case 0:
			stm_reset();
			printk("Kernel:STM Reset Control!\n");
			break;
		case 1:
			stm_boot();
			printk("Kernel:STM Boot Control!\n");
			break;
	}

	return 0;
}


void stm_reset(void)
{
	gpio_set_value(MCU_BOOT, 0);
	mdelay(1000);
	gpio_set_value(MCU_RST, 1);
	mdelay(1000);
	gpio_set_value(MCU_RST, 0);
}


void stm_boot(void)
{
	gpio_set_value(MCU_BOOT, 1);
	mdelay(1000);
	gpio_set_value(MCU_RST, 1);
	mdelay(1000);
	gpio_set_value(MCU_RST, 0);
}



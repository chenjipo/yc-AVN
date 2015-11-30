#ifndef	_STM_ISP_CTL_H
#define _STM_ISP_CTL_H


//#include <stdlib.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>

#include <linux/kernel.h>

#include <linux/slab.h> // shengliang: for kzalloc() etc.
#include <linux/hrtimer.h>

#include <linux/unistd.h>

#include <linux/input.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>


#define MCU_BOOT	EXYNOS4_GPC1(1)        
#define MCU_RST		EXYNOS4_GPX0(0)





long stm_isp_ioctl(struct file *filp,unsigned int cmd,unsigned long arg);
void stm_reset(void);
void stm_boot(void);




#endif



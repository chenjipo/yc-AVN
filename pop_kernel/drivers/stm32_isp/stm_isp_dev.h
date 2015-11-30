#ifndef _STM_ISP_DEV_H
#define _STM_ISP_DEV_H

//#include <stdlib.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/mfd/core.h>

//#include <termios.h>
#include <linux/unistd.h>

#include <linux/input.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

#include <linux/delay.h>



#include "stm_isp_ctl.h"
//#include "stm_isp_usart.h"


static int __init stm_isp_init(void);
static void __exit stm_isp_exit(void);









#endif



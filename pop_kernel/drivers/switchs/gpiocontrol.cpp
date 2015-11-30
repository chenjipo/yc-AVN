#define LOG_NDEBUG 0
#define LOG_TAG "SENDSIG"
#define TAG "SENDSIG"
//#include <utils/Log.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
//#include "sendSIG.h"
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <termios.h>
//#include <android/log.h>
#include <sys/ioctl.h>
//#include <jni.h>
#include <assert.h>
#include <linux/input.h>

#define MUTE_GPIO 9
static const int on = 1;
static const int off = 0;

#define G_ON 	SIGUSR1
#define G_OFF 	SIGUSR2
#define S_ON   	45
#define S_OFF   46

//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , TAG, __VA_ARGS__)
//#define printf(...) __android_log_print(ANDROID_LOG_INFO   , TAG, __VA_ARGS__)


static int pid = -1;
static int getSigPid();
static int setSig(int sig);
static int setMute();
static int setPlay();
static int sendSig(int sig);

int main() {
	int ctl = 0;
	while(1){
	printf("input gpio control\n");
	scanf("%d",&ctl);
	printf("ctl = %d\n",ctl);
	if(ctl == 0)
		setMute();
	else 
		setPlay();
	}
	return 0;
}


int setMute()
{
	int fd_switch = open("/dev/switchs", O_RDWR|O_NDELAY|O_NOCTTY);
	
	if(fd_switch <=0 ){
		printf("open /dev/switchs Error\n");
		return -1;
	}
	else{ 
		printf("open /dev/switchs Sucess\n");
	}

	ioctl(fd_switch, off, MUTE_GPIO);
	printf("set MUTE\n");
	return 0;
}

int setPlay()
{
	int fd_switch = open("/dev/switchs", O_RDWR|O_NDELAY|O_NOCTTY);
	
	if(fd_switch <=0 ){
		printf("open /dev/switchs Error\n");
		return -1;
	}
	else{ 
		printf("open /dev/switchs Sucess\n");
	}

	ioctl(fd_switch, on, MUTE_GPIO);
	printf("SET PLAY\n");
	return 0;

}


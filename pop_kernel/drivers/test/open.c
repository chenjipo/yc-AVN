#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include	<sys/time.h>


//#include <linux/delay.h>


#define MIN_FREQ		87100
#define MAX_FREQ		108000

#define MIN_FREQ_AM		531
#define MAX_FREQ_AM		1620


int Seek_AllFreq(int fd, int MinFreq, int MaxFreq);
int Seek_AllFreq_AM(int fd, int MinFreq, int MaxFreq);
main(int argc,char *argv[])
{
	int fd,ret,Fm_Am_State;
	int cmd = 0;
	unsigned long Hz = 0;
	unsigned long fm = 0;

	if(argc > 1)
	{

		//		printf("name:%s\n",argv[0]);
		printf("cmd:%s\n",argv[1]);
		//		printf("fm:%s\n",argv[2]);
	}

//	cmd =argv[1][0] - 48;
    cmd =atoi(argv[1]);	

	if((fd = open("/dev/tda7786", O_RDWR|O_NOCTTY|O_NDELAY))<0)
		printf("open dev fucking fail!\n");
	else{
		switch(cmd)
		{
			case 0:		//初始化收音机
				ret = ioctl(fd, 5, Hz);
				printf("Return =%d\n",(int)ret);
				break;
			case 1:		//关闭收音机（其实就是好贱格地重启了模块）
				ret = ioctl(fd, 4, Hz);
				printf("Return =%d\n",(int)ret);
				break;
			case 2:		//跳转到指定频段
				Hz +=(argv[2][0]-48) *100000;
				Hz +=(argv[2][1]-48) *10000;
				Hz +=(argv[2][2]-48) *1000;
				Hz +=(argv[2][3]-48) *100;
				Hz +=(argv[2][4]-48) *10;
				Hz +=(argv[2][5]-48) *1;
				printf("Hz =%d\n",(int)Hz/100);
				fm = ioctl(fd, 0, Hz);
				printf("Return =%d\n",(int)fm);
				printf("Return Hz =%d\n",Hz);
				break;
			case 3:		//向上自动搜台
				fm = ioctl(fd, 6, Hz);
				printf("Open Printf Present Freq = %d\n",(int)fm);
				printf("Return =%d\n",(int)fm);
				break;
			case 4:		//向下自动搜台
				fm = ioctl(fd, 7, Hz);
				printf("Open Printf Present Freq = %d\n",(int)fm);
				printf("Return =%d\n",(int)fm);
				break;
			case 5:		//向上手动搜台
				fm = ioctl(fd, 8, Hz);
				printf("Open Printf Present Freq = %d\n",(int)Hz);
//				printf("Open Printf Vaild Flag = %d\n",(int)fm);
				printf("Return =%d\n",(int)fm);
				break;
			case 6:		//向下手动搜台
				fm = ioctl(fd, 9, Hz);
				printf("Open Printf Present Freq = %d\n",(int)fm);
				printf("Return =%d\n",(int)fm);
				break;
			case 7:		//搜索所有有效频段  FM 
				Seek_AllFreq(fd, MIN_FREQ, MAX_FREQ);
				break;
			case 8:
				Hz +=(argv[2][0]-48) *100000;
				Hz +=(argv[2][1]-48) *10000;
				Hz +=(argv[2][2]-48) *1000;
				Hz +=(argv[2][3]-48) *100;
				Hz +=(argv[2][4]-48) *10;
				Hz +=(argv[2][5]-48) *1;
				fm = ioctl(fd, 8, Hz);
				printf("Present Status = %d\n",(int)fm);
//				mdelay(50);
//				fm = ioctl(fd, 1, Hz);			
//				printf("Return =%d\n",(int)fm);
				break;
			case 9:		//向上手动搜台  AM 
				fm = ioctl(fd, 10, Hz);
				printf("Open Printf Present Freq = %d\n",(int)Hz);
//				printf("Open Printf Vaild Flag = %d\n",(int)fm);
				printf("Return =%d\n",(int)fm);
				break;
			case 10:		//
				Fm_Am_State = ioctl(fd, 11, Hz);
				if(Fm_Am_State == 0)
					printf("Open Printf Present state is FM\n");
				if(Fm_Am_State == 1)
					printf("Open Printf Present state is AM\n");
				break;
			case 11:
				Seek_AllFreq_AM(fd, MIN_FREQ_AM, MAX_FREQ_AM);
				break;
			default:
				break;
		}
	}

}



int Seek_AllFreq(int fd, int MinFreq, int MaxFreq)
{
	long l_FeedBack = 0;
//	int l_DelayCounter = 0;
	unsigned long l_Freq = MinFreq;

	ioctl(fd, 0, (unsigned long)MinFreq);
	sleep(1);
	while (1)
	{
//		l_DelayCounter++;
		l_FeedBack = ioctl(fd, 8, l_Freq);
//		printf("FeedBack Return = %d\n", (int)(l_FeedBack));
		if (l_FeedBack == 1)
		{
//			printf("Seek All Freq fucking ok!\n");
			printf ("FeedBack Freq = %d\n", l_Freq);
//			break;
		}
//		else if ((l_FeedBack >= MIN_FREQ) && (l_FeedBack <= MAX_FREQ))
//		{
//			printf("Good Freq--Hz =%d\n",(int)(l_FeedBack/100));
//		}
		if (l_Freq >= MaxFreq)
		{
			printf("Seek All Freq  Ok!\n");
			break;
		}
//		if (l_DelayCounter >= 200)
//			break;

		l_Freq += 100;
			
	}
	return 0;
}

int Seek_AllFreq_AM(int fd, int MinFreq, int MaxFreq)
{
	long l_FeedBack = 0;
//	int l_DelayCounter = 0;
	unsigned long l_Freq = MinFreq;

	ioctl(fd, 0, (unsigned long)MinFreq);
	sleep(1);
	while (1)
	{
//		l_DelayCounter++;
		l_FeedBack = ioctl(fd, 10, l_Freq);
//		printf("FeedBack Return = %d\n", (int)(l_FeedBack));
		if (l_FeedBack == 1)
		{
//			printf("Seek All Freq fucking ok!\n");
			printf ("----------good statuation Freq = %d\n", l_Freq);
//			break;
		}
		printf ("now the  Freq  is = %d\n", l_Freq);
//		else if ((l_FeedBack >= MIN_FREQ) && (l_FeedBack <= MAX_FREQ))
//		{
//			printf("Good Freq--Hz =%d\n",(int)(l_FeedBack/100));
//		}
		if (l_Freq >= MaxFreq)
		{
			printf("Seek All Freq  Ok!\n");
			break;
		}
//		if (l_DelayCounter >= 200)
//			break;

		l_Freq += 9;
			
	}
	return 0;
}


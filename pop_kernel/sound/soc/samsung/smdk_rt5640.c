/* 
********************************************************************** Copyright (C) year - year, Corporation name 
  	 All rights reserved. 
  	 File Name  :	smdk_rt5640.c   
	 Description: platform define of codec , for voice and stream                  
  	 Version : V0.1
  	 Author  : Tanglin
  	 Date    : 2015-4-21

	 version : V0.2
	 Date	:2015-8-11
	 Description: use pll as clk source
********************************************************************** 
*/ 

#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <sound/soc.h>
#include <sound/pcm_params.h>

#include <asm/mach-types.h>
#include <mach/map.h>
#include <mach/regs-audss.h>

#include "../codecs/rt5640.h"   //by lin, include rt5640.h
#include "i2s.h"

#define ASRC_M					//by lin, for ASRC FUNCTION
#define FREQ_IN 4096000
#define SEL_PLL

#if 0
#define printa(x...) 	printk(x)
#else
#define printa(x...)
#endif

/*add by lin , for mute switch*/
//#define MUTE_EVENT
//#ifdef MUTE_EVENT
//#define MUTE_CTRL
#ifdef MUTE_CTRL
#include <linux/kernel.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/miscdevice.h>
#include <mach/regs-gpio.h>
#include <asm/io.h>
#include <linux/delay.h>
#define MUTE_GPIO EXYNOS4_GPX1(1)	//mute gongfang
#endif

#ifdef MUTE_ENENT
static void MUTE_init()
{
	s3c_gpio_cfgpin(MUTE_GPIO, S3C_GPIO_OUTPUT);
	gpio_set_value(MUTE_GPIO, 0);		
	printa("MUTE level = 0\n");
}
#endif


#ifdef NORMAL_UNMUTE
static void MUTE_init()
{
	s3c_gpio_cfgpin(MUTE_GPIO, S3C_GPIO_OUTPUT);
	gpio_set_value(MUTE_GPIO, 1);		
	printa("MUTE level = 1\n");
}
#endif


int dev_ver = 0;

static int set_epll_rate(unsigned long rate)
{
	struct clk *fout_epll;

	fout_epll = clk_get(NULL, "fout_epll");
	if (IS_ERR(fout_epll)) {
		printk(KERN_ERR "%s: failed to get fout_epll\n", __func__);
		return PTR_ERR(fout_epll);
	}

	if (rate == clk_get_rate(fout_epll))
		goto out;

	clk_set_rate(fout_epll, rate);
out:
	clk_put(fout_epll);

	return 0;
}

//static int set_mout_audss_rate(unsigned long rate)
//{
//	struct clk *fout_epll;
//  unsigned long epll_rate;
//  int i;
// 	u32 clk_div = readl(S5P_CLKDIV_AUDSS);
// 	int div;
//
//	fout_epll = clk_get(NULL, "fout_epll");
//	if (IS_ERR(fout_epll)) {
//		printk(KERN_ERR "%s: failed to get fout_epll\n", __func__);
//		return PTR_ERR(fout_epll);
//	}
//  epll_rate = clk_get_rate(fout_epll);
//  
//  for(i=1;i<16;i++)
//  {
//    if(epll_rate/i <= rate)
//      break;
//  }
////  if(i>1)
////  {
////   if(abs(epll_rate/(i-1)-rate) > abs(epll_rate/i-rate))
////      div = i-1;
////    else
////      div = i;
////  }
////  else
////    div = 1;
//  div = i;
//	clk_div &= ~(S5P_AUDSS_CLKDIV_RP_MASK	| S5P_AUDSS_CLKDIV_BUSCLK_MASK);
//	clk_div |= (div-1) << S5P_AUDSS_CLKDIV_BUSCLK_SHIFT;
//  writel(clk_div, S5P_CLKDIV_AUDSS);
//  printk("%s rate_epll=%ld want_rate=%ld div=%d bus_clk=%ld", __func__, epll_rate, rate, div, epll_rate/div);
//	return 0;
//}


static int mixtile_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	int bfs, rfs, ret, psr;
	unsigned long rclk;

	printa("into mixtile_hw_params\n");

	#ifndef ASRC_M
	/* add by lin, not ASRC */
	int sysclk = 256 * params_rate(params);
	#endif

	#ifdef ASRC_M
	/* add by lin, ASRC*/ 
	int sysclk = 512 * params_rate(params);
	int freq_in = FREQ_IN;
	int freq_out = sysclk;
	#endif

	printa("check 1 sysclk = %d\n" , sysclk);

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_U24:
	case SNDRV_PCM_FORMAT_S24:
		bfs = 48;
		break;
	case SNDRV_PCM_FORMAT_U16_LE:
	case SNDRV_PCM_FORMAT_S16_LE:
		bfs = 32;
		break;
	default:
		return -EINVAL;
	}

	printa("check 2 , bfs = %d\n",bfs);
		
	switch (params_rate(params)) {
	case 16000:
	case 22050:
	case 24000:
	case 32000:
	case 44100:
	case 48000:
	case 88200:
	case 96000:
		if (bfs == 48)
			rfs = 384;
		else
			rfs = 256;
		break;
	case 64000:
		rfs = 384;
		break;
	case 8000:
	case 11025:
	case 12000:
		if (bfs == 48)
			rfs = 768;
		else
			rfs = 512;
		break;
	default:
		return -EINVAL;
	}

	printa("check 3 , rfs = %d\n",rfs);
	
	rclk = params_rate(params) * rfs;
	printa("check 4 , rclk = %d\n",rclk);

	switch (rclk) {
	case 4096000:
	case 5644800:
	case 6144000:
	case 8467200:
	case 9216000:
		psr = 8;
		break;
	case 8192000:
	case 11289600:
	case 12288000:
	case 16934400:
	case 18432000:
		psr = 4;
		break;
	case 22579200:
	case 24576000:
	case 33868800:
	case 36864000:
		psr = 2;
		break;
	case 67737600:
	case 73728000:
		psr = 1;
		break;
	default:
		printk(KERN_ERR "Not yet supported!\n");
		return -EINVAL;
	}

	printa("check , psr = %d\n",psr);

	set_epll_rate(rclk * psr);

//  set_mout_audss_rate(rclk * psr);

	/* Set the Codec DAI configuration */
	//设置芯片模式 I2S、SLAVE_MODE
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS); 
//by lin 	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_LEFT_J | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS); 
	if (ret < 0)
		return ret;

	/* 设置CPU I2S模块 MASTER*/
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
//by lin 	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_LEFT_J | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	/* 选择CPU I2S模块 时钟源 I2S_CLK 寄存器 IISMOD[10] */
	ret = snd_soc_dai_set_sysclk(cpu_dai, SAMSUNG_I2S_RCLKSRC_0/* SAMSUNG_I2S_RCLKSRC_1 */, 0, SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;

	/* I2S CDCLK脚为时钟输出 */
	ret = snd_soc_dai_set_sysclk(cpu_dai, SAMSUNG_I2S_CDCLK, 0/* rfs */, SND_SOC_CLOCK_OUT);
	//ret = snd_soc_dai_set_sysclk(cpu_dai, SAMSUNG_I2S_CDCLK, rfs , SND_SOC_CLOCK_OUT);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_clkdiv(cpu_dai, SAMSUNG_I2S_DIV_BCLK, bfs);
	if (ret < 0)
		return ret;

#ifdef SEL_PLL
	    /*set pll param*/
		printa("PLL INPUT\n");
	    snd_soc_dai_set_pll(codec_dai, 0, RT5640_PLL1_S_MCLK, freq_in, freq_out);
	    //end by lin

		//ret = snd_soc_dai_set_sysclk(codec_dai, 0, sysclk, RT5640_SCLK_S_PLL1);
		ret = snd_soc_dai_set_sysclk(codec_dai , RT5640_SCLK_S_PLL1 , sysclk, 0);
        if(ret < 0){
        dev_err(codec_dai->dev,"codec_dai clock not set\n");
        return ret;
		}
#endif

#ifdef SEL_MCLK
        printa("MCLK INPUT\n");
        //ret = snd_soc_dai_set_sysclk(codec_dai, 0, sysclk, RT5640_SCLK_S_MCLK);
        ret = snd_soc_dai_set_sysclk(codec_dai, RT5640_SCLK_S_MCLK , sysclk, 0);
        if(ret < 0){
                dev_err(codec_dai->dev,"codec_dai clock not set\n");
        		return ret;
    	}
#endif

	return 0;
}

/*
 * SMDK WM8580 DAI operations.
 */
static struct snd_soc_ops mixtile_ops = {
	.hw_params = mixtile_hw_params,
};

/*add by lin*/
/* add by lin for MUTE function */
static int rt5640_down_event(struct snd_soc_dapm_widget *w,
			     struct snd_kcontrol *kcontrol, int event)
{
#ifdef MUTE_EVENT
	struct rt5640_priv *rt5640 = snd_soc_codec_get_drvdata(w->codec);

	switch (event) {
/*	case SND_SOC_DAPM_POST_PMU:
		gpio_set_value(MUTE_GPIO, 1);
		printa("gong fang UN-MUTE ,POST_PMU\n");
		break;*/
	case SND_SOC_DAPM_PRE_PMD:
		gpio_set_value(MUTE_GPIO, 0); 
		printa("gong fang MUTE , PRE_PMD \n");
		break;
	default:
		return 0;
	}
#endif
	return 0;
}

static int rt5640_up_event(struct snd_soc_dapm_widget *w,
			     struct snd_kcontrol *kcontrol, int event)
{
	struct rt5640_priv *rt5640 = snd_soc_codec_get_drvdata(w->codec);

#ifdef MUTE_EVENT
	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		gpio_set_value(MUTE_GPIO, 1);
		printa("gong fang UN-MUTE ,POST_PMU\n");
		break;
/*	case SND_SOC_DAPM_PRE_PMD:
		gpio_set_value(MUTE_GPIO, 0); 
		printa("gong fang MUTE , PRE_PMD \n");
		break;*/
	default:
		return 0;
	}
#endif
	return 0;
}



/* mixtilev310 Playback widgets */
static const struct snd_soc_dapm_widget rt5640_dapm_widgets_pbk[] = {
//	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_LINE("Speaker_L", rt5640_down_event),
	SND_SOC_DAPM_LINE("Speaker_R", rt5640_down_event),

	SND_SOC_DAPM_POST("Speaker_L_up", rt5640_up_event),
	SND_SOC_DAPM_POST("Speaker_R_up", rt5640_up_event),
};

/* mixtilev310 RX connections */
static const struct snd_soc_dapm_route audio_map_rx[] = {
	/* Headphone */
	//{ "Headphone Jack", NULL, "HP_L" },
	//{ "Headphone Jack", NULL, "HP_R" },

	/* Speaker */
	/*{ "Speaker_L", NULL, "SPK_LP" },
	{ "Speaker_L", NULL, "SPK_LN" },
	{ "Speaker_R", NULL, "SPK_RP" },
	{ "Speaker_R", NULL, "SPK_RN" },*/
		
	{ "Speaker_L", NULL, "LOUTL" },
	{ "Speaker_R", NULL, "LOUTR" },	

	{ "Speaker_L_up", NULL, "LOUTL" },
	{ "Speaker_R_up", NULL, "LOUTR" },	
};

/* mixtilev310 Capture widgets */
static const struct snd_soc_dapm_widget rt5640_dapm_widgets_cpt[] = {
	//SND_SOC_DAPM_MIC("Mic Jack", NULL),
	SND_SOC_DAPM_MIC("Mic main", NULL),
	//SND_SOC_DAPM_LINE("Line Input 3 (FM)", NULL),
};

/* mixtilev310 TX connections */
static const struct snd_soc_dapm_route audio_map_tx[] = {
	/* Line Input 3 (FM)*/
/*	{ "LINPUT3", NULL, "Line Input 3 (FM)" },
	{ "RINPUT3", NULL, "Line Input 3 (FM)" },*/

	{ "IN1P", NULL, "Mic main"},
	//{ "RINPUT2", NULL, "Mic Jack"},
};

static const struct snd_soc_dapm_route audio_map_tx_htpc[] = {
	/* Line Input 3 (FM)*/
/*	{ "LINPUT3", NULL, "Line Input 3 (FM)" },
	{ "RINPUT3", NULL, "Line Input 3 (FM)" },*/

	{ "IN1P", NULL, "Mic main"},
	//{ "LINPUT2", NULL, "Mic Jack"},
};

static int mixtile_rt5640_init_paiftx(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dapm_context *dapm = &codec->dapm;

	printa("mixtile capture new control\n");
	snd_soc_dapm_new_controls(dapm, rt5640_dapm_widgets_cpt, ARRAY_SIZE(rt5640_dapm_widgets_cpt));

	printa("mixtile capture add_routes\n");
  if(dev_ver == 1)
    snd_soc_dapm_add_routes(dapm, audio_map_tx_htpc, ARRAY_SIZE(audio_map_tx_htpc));
  else
    snd_soc_dapm_add_routes(dapm, audio_map_tx, ARRAY_SIZE(audio_map_tx));
  
  	printa("mixtile capture snd_soc_dapm_sync\n");
	snd_soc_dapm_sync(dapm);

	

	return 0;
}

static int mixtile_rt5640_init_paifrx(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dapm_context *dapm = &codec->dapm;

	printa("mixtile play new_controls\n"); 
	snd_soc_dapm_new_controls(dapm, rt5640_dapm_widgets_pbk, ARRAY_SIZE(rt5640_dapm_widgets_pbk));

	printa("mixtile play add_routes\n");
	snd_soc_dapm_add_routes(dapm, audio_map_rx, ARRAY_SIZE(audio_map_rx));

	printa("mixtile play snd_soc_dapm_sync\n");
  	snd_soc_dapm_sync(dapm);
	
	return 0;
}


/* goni Playback widgets */
static const struct snd_soc_dapm_widget goni_dapm_widgets_pbk[] = {
//	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_LINE("goni_LOUT_L", rt5640_down_event),
	//SND_SOC_DAPM_LINE("goni_LOUT_R", rt5640_down_event),
};

/* goni RX connections */
static const struct snd_soc_dapm_route goni_map_rx[] = {
	{ "goni_LOUT_L", NULL, "HPOL" },
	//{ "goni_LOUT_R", NULL, "HPOR" },	
};

static int goni_rt5640_init_paifrx(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dapm_context *dapm = &codec->dapm;

	printa("goni play new_controls\n"); 
	snd_soc_dapm_new_controls(dapm, goni_dapm_widgets_pbk, ARRAY_SIZE(goni_dapm_widgets_pbk));

	printa("goni play add_routes\n");
	snd_soc_dapm_add_routes(dapm, goni_map_rx, ARRAY_SIZE(goni_map_rx));

	printa("goni play snd_soc_dapm_sync\n");
  	snd_soc_dapm_sync(dapm);
	
	return 0;
}


/* add by lin, for 3G voice */
static int goni_voice_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	int ret = 0;

    #ifdef MUTE_CTRL
    printa("before hw setting, set mute\n");
    gpio_set_value(MUTE_GPIO, 0);
    printa("sleep 10\n");
    msleep(10);
    #endif

	printa("goni , params_rate = %d\n",params_rate(params));

	if (params_rate(params) != 8000)
	{	
		printa("error , return, params_rate(params) != 8000");
		return -EINVAL;
	}

	/* add by lin, ASRC */ 
	int sysclk =512 * params_rate(params);
	int freq_in = FREQ_IN;
	int freq_out = sysclk;


	printa("goni , format = %d\n", params_format(params));
	/* Set the Codec DAI configuration */
	//设置芯片模式 I2S、SLAVE_MODE
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_DSP_A | SND_SOC_DAIFMT_IB_NF| SND_SOC_DAIFMT_CBS_CFS); 
	if (ret < 0)
		return ret;
	
	/* add by lin , for voice clk */
	printa("freq_in = %d , freq_out = %d\n",freq_in,freq_out);

#ifdef SEL_PLL
	{		
		printa("Pll input\n");
		ret = snd_soc_dai_set_pll(codec_dai, 0, RT5640_PLL1_S_MCLK, freq_in, freq_out);
		if(ret < 0){
				dev_err(codec_dai->dev,"error pll not set\n");
				return ret;
			}

		//ret = snd_soc_dai_set_sysclk(codec_dai, 0, sysclk, RT5640_SCLK_S_PLL1);
		ret = snd_soc_dai_set_sysclk(codec_dai, RT5640_SCLK_S_PLL1, sysclk,0);
		if(ret < 0){
				dev_err(codec_dai->dev,"codec_dai clock not set\n");
				return ret;
			}		
	}
#endif

#ifdef SEL_MCLK
	{
		printa("MCLK input\n");
		//ret = snd_soc_dai_set_sysclk(codec_dai, 0, sysclk, RT5640_SCLK_S_MCLK);
		ret = snd_soc_dai_set_sysclk(codec_dai, RT5640_SCLK_S_MCLK, sysclk, 0);
		if(ret < 0){
				dev_err(codec_dai->dev,"codec_dai clock not set\n");
		return ret;
		}
	} 
#endif

		#ifdef MUTE_CTRL
        printa("after pll setting, sleep 100\n");
        msleep(10);
        printa("set play\n");
        gpio_set_value(MUTE_GPIO, 1);
        #endif

	return 0;
}


static const struct snd_pcm_hardware dummy_voice_hardware = {
	.info			= SNDRV_PCM_INFO_INTERLEAVED ,
	.channels_min	= 1,
	.channels_max   = 2,
	.formats		= SNDRV_PCM_FMTBIT_S16_LE,	
	.rate_min		= 8000,
	.rate_max		= 16000,
	.period_bytes_min	= 16,
	.period_bytes_max	= 1024,
	.periods_min		= 2,
	.periods_max		= 16,
	.buffer_bytes_max	= 4*1024,
	//.fifo_size		= 32,
};


/*static int dummy_voice_startup(struct snd_pcm_substream *substream)
{
	
	int ret;

	printa("dummy_voice_startup\n");
	snd_soc_set_runtime_hwparams(substream, &dummy_voice_hardware);

	/* Ensure that buffer size is a multiple of period size 
	struct snd_pcm_runtime *runtime = substream->runtime;
	ret = snd_pcm_hw_constraint_integer(runtime,SNDRV_PCM_HW_PARAM_PERIODS);
	*/
	/*return 0;
}*/


static struct snd_soc_ops goni_voice_ops = {
	//.startup = dummy_voice_startup,
	.hw_params = goni_voice_hw_params,
};
/*end by lin for 3G voice */

enum {
	PRI_PLAYBACK = 0,
	PRI_CAPTURE,
	VOICE_PLAYBACK,
};

static struct snd_soc_dai_link mixtile_dai[] = {
	[PRI_PLAYBACK] = { /* Primary Playback i/f */
		.name = "RT5640 Playback",				//by lin
		.stream_name = "Playback",
		.cpu_dai_name = "samsung-i2s.0",		//i2s0
		.codec_dai_name = "rt5640-aif1",  		//by lin, snd_soc_dai_driver, pcm1
		.platform_name = "samsung-audio",		//dma
		.codec_name = "rt5640.4-001c",			//by lin, i2c_driver, i2c4,rt5640-001c
		.init = mixtile_rt5640_init_paifrx,
		.ops = &mixtile_ops,
	},
	[PRI_CAPTURE] = { /* Primary Capture i/f */
		.name = "RT5640 Capture",				//by lin 
		.stream_name = "Capture",				//
		.cpu_dai_name = "samsung-i2s.0",		//i2s0
		.codec_dai_name = "rt5640-aif1",		//by lin, snd_soc_dai_driver, pcm1
		.platform_name = "samsung-audio",		//dma
		.codec_name = "rt5640.4-001c",			//by lin, i2c_driver,i2c4,rt5640-001c
		.init = mixtile_rt5640_init_paiftx,
		.ops = &mixtile_ops,
	},
	[VOICE_PLAYBACK] = { 
		.name = "3G-voice",
		.stream_name = "Voice",
		.cpu_dai_name = "snd-soc-dummy",
		.codec_dai_name = "rt5640-aif2",
		.codec_name = "rt5640.4-001c",
		//.init = goni_rt5640_init_paifrx,		/*lin test
		.ops = &goni_voice_ops,
	},		
};

static struct snd_soc_card mixtile_card = {
	.name = "iTOP-4412-Audio",					
	.dai_link = mixtile_dai,
	.num_links = ARRAY_SIZE(mixtile_dai),
};

static struct platform_device *mixtile_snd_device;

static int __init mixtile_audio_init(void)
{
	int ret;

	/*add by lin*/
#ifdef MUTE_EVENT
	MUTE_init();
#endif

	mixtile_snd_device = platform_device_alloc("soc-audio", -1);
	if (!mixtile_snd_device)
		return -ENOMEM;

	platform_set_drvdata(mixtile_snd_device, &mixtile_card);
	ret = platform_device_add(mixtile_snd_device);

	if (ret)
		platform_device_put(mixtile_snd_device);

	return ret;
}
module_init(mixtile_audio_init);

static void __exit mixtile_audio_exit(void)
{
	platform_device_unregister(mixtile_snd_device);
}
module_exit(mixtile_audio_exit);

MODULE_AUTHOR("lin");
MODULE_DESCRIPTION("ALSA SoC SMDK rt5640");
MODULE_LICENSE("GPL");

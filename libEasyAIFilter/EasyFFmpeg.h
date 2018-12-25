
#ifndef _EASYFFMPEG_H_
#define _EASYFFMPEG_H_

#include <stdio.h>

#ifdef _WIN32
#include <tchar.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

extern "C"
{
#include "libavcodec/avfft.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/mathematics.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"
#include "libavutil/samplefmt.h"
#include "libavdevice/avdevice.h"  //摄像头所用
#include "libavfilter/avfilter.h"
#include "libavutil/error.h"
#include "libavutil/mathematics.h"  
#include "libavutil/time.h"  
#include "libavutil/fifo.h"
#include "libavutil/audio_fifo.h"   //这里是做分片时候重采样编码音频用的
//#include "libavfilter/avfiltergraph.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "inttypes.h"
#include "stdint.h"
};

#ifdef _WIN32
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avfilter.lib")
#pragma comment(lib,"avutil.lib")
//#pragma comment(lib,"swresample.lib")
//#pragma comment(lib,"swscale.lib")
#endif

#ifdef _WIN32
#define     __DELAY(x)  Sleep(x)
#else
#define     __DELAY(x)  usleep(x*1000)
#endif

typedef enum {
	NALU_TYPE_SLICE = 1,
	NALU_TYPE_DPA = 2,
	NALU_TYPE_DPB = 3,
	NALU_TYPE_DPC = 4,
	NALU_TYPE_IDR = 5,
	NALU_TYPE_SEI = 6,
	NALU_TYPE_SPS = 7,
	NALU_TYPE_PPS = 8,
	NALU_TYPE_AUD = 9,
	NALU_TYPE_EOSEQ = 10,
	NALU_TYPE_EOSTREAM = 11,
	NALU_TYPE_FILL = 12,
} NaluType;

#define    MAX_URL_LEN 512

#endif

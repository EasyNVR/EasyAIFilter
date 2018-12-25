/*
Copyright (c) 2013-2019 EasyDarwin.ORG.  All rights reserved.
Github: https://github.com/EasyDarwin
WEChat: EasyDarwin
Website: http://www.easydarwin.org
*/
#ifndef __EASY_PROCESS_H__
#define __EASY_PROCESS_H__

#include "EasyFFmpeg.h"
#include "EasyAIFilterAPI.h"
#include <string>
#include "EasyQueue.h"
#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;

#ifdef _WIN32
#define     __DELAY(x)  Sleep(x)
#else
#define     __DELAY(x)  usleep(x*1000)
#endif

class EasyProcess
{
public:
	EasyProcess(EASY_DATA_INFO_T* mediaInfo);

	~EasyProcess();

	void SetCallback(EasyAIFilterCallBack callback);

	void Callback(EASY_AV_Frame* frame)
	{
		if (callback_ && frame)
		{
			callback_(mediaInfo_.pUserPtr, frame);
		}
	}

	AVPacket* Transcode(char* srcData, int srcSize);

	int SendData(EASY_AV_Frame* frame);

private:
	int Init();
	int initDecodecContext();
	int initEncodeContext();
	int initFilter(AVCodecContext * codecContext);
	AVFrame* decode(AVPacket* packet);
	AVPacket *encode(AVFrame *filterFrame);
public:
	EasyQueue				pQueueObj_;
	int						threadFlag_;
	AVCodecContext*			encodeContext_;

private:
	EASY_DATA_INFO_T		mediaInfo_;
	AVCodecID				videoCodecId_;
	AVFilterContext*		buffersinkCtx_;
	AVFilterContext*		buffersrcCtx_;
	AVFilterGraph*			filterGraph_;
	AVFilterInOut*			outputs_;
	AVFilterInOut*			inputs_;
	AVCodecContext*			decodeContext_;
	EasyAIFilterCallBack	callback_;
	bool					getFirstKeyFrame_;
#ifdef _WIN32
	HANDLE					hThread_;
#else
	pthread_t				hThread_;
#endif
};

#endif //__EASY_PROCESS_H__
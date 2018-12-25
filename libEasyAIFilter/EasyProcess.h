/*
Copyright (c) 2013-2019 EasyDarwin.ORG.  All rights reserved.
Github: https://github.com/EasyDarwin
WEChat: EasyDarwin
Website: http://www.easydarwin.org
*/
#ifndef __EASY_PROCESS_H__
#define __EASY_PROCESS_H__

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

	int SendData(EASY_AV_Frame* frame);

public:
	EasyQueue				pQueueObj_;
	int						threadFlag_;

private:
	EASY_DATA_INFO_T		mediaInfo_;
	EasyAIFilterCallBack	callback_;
	bool					getFirstKeyFrame_;
#ifdef _WIN32
	HANDLE					hThread_;
#else
	pthread_t				hThread_;
#endif
};

#endif //__EASY_PROCESS_H__
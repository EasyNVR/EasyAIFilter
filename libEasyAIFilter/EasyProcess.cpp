/*
Copyright (c) 2013-2019 EasyDarwin.ORG.  All rights reserved.
Github: https://github.com/EasyDarwin
WEChat: EasyDarwin
Website: http://www.easydarwin.org
*/
#include "EasyProcess.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
static DWORD WINAPI Run(LPVOID lpParam)
#else
static void *Run(void *lpParam)
#endif
{
	EasyProcess *handle = (EasyProcess *)lpParam;

	if (NULL == handle)
	{
#ifdef _WIN32
		return -1;
#else
		return (void*)-1;
#endif
	}

	handle->threadFlag_ = 0x02;

	while (true)
	{
		if (handle->threadFlag_ == 0x03)        break;

		EASY_AV_Frame* frame = EasyQueue_dequeue(&handle->pQueueObj_);
		if (frame)
		{
			//TODO:: Do the things by userself! Then Callback the result data. the result video data must be H264 data, audio is AAC format.
			//This is the example, callback the orginal data.
			handle->Callback(frame);

			free(frame->pBuffer);
			free(frame);
		}
		else
		{
			__DELAY(10);
		}
	}

	handle->threadFlag_ = 0x00;

#ifdef _WIN32
	return 0;
#else
	pthread_detach(pthread_self());
	return (void*)0;
#endif
}

EasyProcess::EasyProcess(EASY_DATA_INFO_T* mediaInfo)
{
	memcpy(&mediaInfo_, mediaInfo, sizeof(EASY_DATA_INFO_T));

	callback_ = NULL;
	threadFlag_ = 0;
	hThread_ = 0;
	getFirstKeyFrame_ = false;
	EasyQueue_Init(&pQueueObj_);
}

EasyProcess::~EasyProcess()
{
	//Destroy Run Thread
	if (threadFlag_ != 0x00)
	{
		threadFlag_ = 0x03;
		while (threadFlag_ != 0x00) { __DELAY(10); }
	}

#ifdef _WIN32
	if (NULL != hThread_)
	{
		CloseHandle(hThread_);
		hThread_ = NULL;
	}
#endif

	EasyQueue_Destroy(&pQueueObj_);
}


int EasyProcess::SendData(EASY_AV_Frame* frame)
{
	int ret = 0;

	if (!frame || frame->u32AVFrameLen <= 0) return -1;

	if(!getFirstKeyFrame_ && frame->u32AVFrameFlag == EASY_SDK_VIDEO_FRAME_FLAG && frame->u32VFrameType == EASY_SDK_VIDEO_FRAME_I)
	{
		getFirstKeyFrame_ = true;
	}

	if(!getFirstKeyFrame_)
	{
		return 0;
	}

	if(EasyQueue_size(&pQueueObj_) == MAX_QUEUE_BUFF_SIZE)
	{
		printf("The current buffer is full. Drop this frame!\n");
		return 0;
	}

	EASY_AV_Frame* avFrame = (EASY_AV_Frame*)malloc(sizeof(EASY_AV_Frame));
	memcpy(avFrame, frame, sizeof(EASY_AV_Frame));
	avFrame->pBuffer = (Easy_U8*)malloc(avFrame->u32AVFrameLen);
	memcpy(avFrame->pBuffer, frame->pBuffer, frame->u32AVFrameLen);

	ret = EasyQueue_enqueue(avFrame, &pQueueObj_);
	if (ret < 0)
	{
		free(avFrame->pBuffer);
		free(avFrame);
		printf("The current buffer is full. Drop this frame!\n");
	}

	return 0;
}

void EasyProcess::SetCallback(EasyAIFilterCallBack callback)
{
	callback_ = callback;

	if (0 == hThread_)
	{
		threadFlag_ = 0x01;
#ifdef _WIN32
		hThread_ = CreateThread(NULL, 0, Run, this, 0, NULL);
		if (NULL == hThread_)		threadFlag_ = 0x00;
#else
		pthread_create(&hThread_, NULL, Run, this);
#endif
		while (threadFlag_ != 0x02 && threadFlag_ != 0x00) { __DELAY(10); }
	}
}
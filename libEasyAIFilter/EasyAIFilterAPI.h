/*
	Copyright (c) 2013-2019 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef _EASY_AI_FILTER_API_H
#define _EASY_AI_FILTER_API_H

#include "EasyTypes.h"


typedef int (Easy_APICALL *EasyAIFilterCallBack)(void* userPtr, EASY_AV_Frame* frame);

typedef struct __EASY_DATA_INFO_T
{
	void*	 pUserPtr;		/*pUserPtr, shouldn't modify it*/
	Easy_U32 u32ChannelID;	/*The Channel ID in EasyNVR channel config*/
	Easy_U32 u32VideoCodec;	
	Easy_U32 u32Width;
	Easy_U32 u32Height;
	Easy_U32 u32AudioCodec;
	Easy_U32 u32AudioSamplerate;
	Easy_U32 u32AudioChannel;
	Easy_U32 u32AudioBitsPerSample;
}EASY_DATA_INFO_T;

#ifdef __cplusplus
extern "C" 
{
#endif
	/* Init a filter. 
	 * logLevel: 0 - donot output log   1 - output log
	 */
	Easy_API Easy_Handle Easy_APICALL EasyAIFilter_Init(EASY_DATA_INFO_T* dataInfo, EasyAIFilterCallBack callback, int logLevel);

	/* Input the source data */
	Easy_API int Easy_APICALL EasyAIFilter_SendData(Easy_Handle handle, EASY_AV_Frame* frame);

	/* Deinit this handle*/
	Easy_API int Easy_APICALL EasyAIFilter_Deinit(Easy_Handle handle);
#ifdef __cplusplus
};
#endif

#endif //_EASY_AI_FILTER_API_H

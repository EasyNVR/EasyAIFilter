/*
Copyright (c) 2013-2019 EasyDarwin.ORG.  All rights reserved.
Github: https://github.com/EasyDarwin
WEChat: EasyDarwin
Website: http://www.easydarwin.org
*/
#include "EasyAIFilterAPI.h"
#include <cstdio>
#include "EasyProcess.h"

#ifdef _WIN32
#include <WinSock2.h>
#endif


/* 创建EasyAIFilter句柄  返回0表示成功，返回非0表示失败 */
Easy_API Easy_Handle Easy_APICALL EasyAIFilter_Init(EASY_DATA_INFO_T* dataInfo, EasyAIFilterCallBack callback, int logLevel)
{
	if(dataInfo == NULL)
	{
		return 0;
	}

	EasyProcess *client = new EasyProcess(dataInfo);
	if (client == NULL)
	{
		printf("%s Create handle failed! need retry!\n", __FUNCTION__);
		return 0;
	}
	client->SetCallback(callback);

	return client;
}

/* 输入原始流数据 */
Easy_API int Easy_APICALL EasyAIFilter_SendData(Easy_Handle handle, EASY_AV_Frame* frame)
{
	if (handle == NULL)
	{
		return -1;
	}

	EasyProcess* pClient = (EasyProcess*)handle;

	if (pClient->SendData(frame) < 0)
	{
		return -2;
	}

	return 0;
}

/* 释放EasyAIFilter 参数为EasyAIFilter句柄 */
Easy_API int Easy_APICALL EasyAIFilter_Deinit(Easy_Handle handle)
{
	if (handle == NULL)
	{
		printf("EasyDataProcess_Deinit failed! handle is null!\n");
		return -1;
	}

	delete (EasyProcess*)handle;

	return 0;
}

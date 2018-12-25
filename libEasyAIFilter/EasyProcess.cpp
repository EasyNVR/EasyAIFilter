/*
Copyright (c) 2013-2019 EasyDarwin.ORG.  All rights reserved.
Github: https://github.com/EasyDarwin
WEChat: EasyDarwin
Website: http://www.easydarwin.org
*/
#include "EasyProcess.h"

#define MAX_FRAME_SIZE 4096000

static int FindVideoNaluStartPos(const unsigned char* data, int length)
{
	int i = 0;
	int index = 0;

	for (i = 0; i < length - 4; i++)
	{
		int nalu = 0;
		if ((data[i] & 0xFF) == 0x0 && (data[i + 1] & 0xFF) == 0x0 && (data[i + 2] & 0xFF) == 0x0 && (data[i + 3] & 0xFF) == 0x1)
		{
			nalu = (data[i + 4] & 0x1F);
		}
		else if ((data[i] & 0xFF) == 0x0 && (data[i + 1] & 0xFF) == 0x0 && (data[i + 2] & 0xFF) == 0x1)
		{
			nalu = (data[i + 3] & 0x1F);
		}

		if ((nalu == NALU_TYPE_SLICE) || (nalu == NALU_TYPE_SPS) || (nalu == NALU_TYPE_PPS) || (nalu == NALU_TYPE_SEI) || (nalu == NALU_TYPE_IDR))
		{
			return i;
		}
	}

	return 0;
}

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

	unsigned char* buffer = (unsigned char*)malloc(MAX_FRAME_SIZE);
	memset(buffer, 0, MAX_FRAME_SIZE);

	handle->threadFlag_ = 0x02;

	while (true)
	{
		if (handle->threadFlag_ == 0x03)        break;

		EASY_AV_Frame* frame = EasyQueue_dequeue(&handle->pQueueObj_);
		if (frame)
		{
			if (EASY_SDK_VIDEO_FRAME_FLAG == frame->u32AVFrameFlag)
			{
				AVPacket* packet = handle->Transcode((char*)frame->pBuffer, frame->u32AVFrameLen);
				if (packet)
				{
					EASY_AV_Frame outFrame;
					memset(&outFrame, 0, sizeof(EASY_AV_Frame));
					outFrame.u32TimestampSec = frame->u32TimestampSec;
					outFrame.u32TimestampUsec = frame->u32TimestampUsec;
					outFrame.u32AVFrameFlag = EASY_SDK_VIDEO_FRAME_FLAG;

					int index = FindVideoNaluStartPos(packet->data, packet->size);
					if (handle->encodeContext_->extradata_size > 0 && (packet->flags & AV_PKT_FLAG_KEY))
					{
						memcpy(buffer, handle->encodeContext_->extradata, handle->encodeContext_->extradata_size);
						memcpy(buffer + handle->encodeContext_->extradata_size, packet->data + index, packet->size - index);
						outFrame.pBuffer = buffer;
						outFrame.u32AVFrameLen = handle->encodeContext_->extradata_size + packet->size - index;
					}
					else
					{
						outFrame.pBuffer = packet->data + index;
						outFrame.u32AVFrameLen = packet->size - index;
					}

					outFrame.u32VFrameType = (packet->flags & AV_PKT_FLAG_KEY) ? EASY_SDK_VIDEO_FRAME_I : EASY_SDK_VIDEO_FRAME_P;
					handle->Callback(&outFrame);

					av_packet_unref(packet);
					av_free(packet);
				}
			}
			else if(EASY_SDK_AUDIO_FRAME_FLAG == frame->u32AVFrameFlag)
			{
				handle->Callback(frame);
			}

			free(frame->pBuffer);
			free(frame);
			frame = NULL;
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
	if(mediaInfo_.u32VideoCodec == EASY_SDK_VIDEO_CODEC_H264)
	{
		videoCodecId_ = AV_CODEC_ID_H264;
	}
	else
	{
		videoCodecId_ = (AVCodecID)mediaInfo_.u32VideoCodec;
	}

	buffersinkCtx_ = NULL;
	buffersrcCtx_ = NULL;
	filterGraph_ = NULL;
	encodeContext_ = NULL;
	outputs_ = NULL;
	inputs_ = NULL;
	decodeContext_ = NULL;
	callback_ = NULL;
	threadFlag_ = 0;
	hThread_ = 0;
	getFirstKeyFrame_ = false;
	EasyQueue_Init(&pQueueObj_);

	av_log_set_level(AV_LOG_QUIET);
	Init();
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

	if (filterGraph_)
	{
		while (filterGraph_->nb_filters)
			avfilter_free(filterGraph_->filters[0]);
		avfilter_graph_free(&filterGraph_);
		filterGraph_ = NULL;
	}

	if (outputs_)
	{
		avfilter_inout_free(&outputs_);
		outputs_ = NULL;
	}

	if (inputs_)
	{
		avfilter_inout_free(&inputs_);
		inputs_ = NULL;
	}

	if (encodeContext_)
	{
		avcodec_close(encodeContext_);
		avcodec_free_context(&encodeContext_);
		encodeContext_ = NULL;
	}

	if (decodeContext_)
	{
		avcodec_close(decodeContext_);
		avcodec_free_context(&decodeContext_);
		decodeContext_ = NULL;
	}
}

int EasyProcess::Init()
{
	int ret = 0;
	ret = initDecodecContext();
	if (ret < 0)
	{
		return ret;
	}

	ret = initEncodeContext();
	if (ret < 0)
	{
		return ret;
	}

	ret = initFilter(encodeContext_);
	if (ret < 0)
	{
		return ret;
	}

	return  ret;
}

int EasyProcess::initDecodecContext()
{
	int ret = 0;
	printf("videoCodecId_=%d %dx%d\n", videoCodecId_, mediaInfo_.u32Width, mediaInfo_.u32Height);
	AVCodec *  codec = avcodec_find_decoder(videoCodecId_);
	decodeContext_ = avcodec_alloc_context3(codec);
	decodeContext_->codec = codec;
	decodeContext_->time_base.num = 1;
	decodeContext_->time_base.den = 25;
	decodeContext_->width = mediaInfo_.u32Width;
	decodeContext_->height = mediaInfo_.u32Height;
	if (codec->capabilities & AV_CODEC_CAP_TRUNCATED)
		decodeContext_->flags |= AV_CODEC_FLAG_TRUNCATED;

	ret = avcodec_open2(decodeContext_, codec, 0);
	return ret;
}

int EasyProcess::initEncodeContext()
{
	int ret = 0;
	AVCodec *  codec = avcodec_find_encoder(videoCodecId_);
	encodeContext_ = avcodec_alloc_context3(codec);
	encodeContext_->codec = codec;
	encodeContext_->time_base.num = 1;
	encodeContext_->time_base.den = 25;
	encodeContext_->width = mediaInfo_.u32Width;
	encodeContext_->height = mediaInfo_.u32Height;
	encodeContext_->pix_fmt = AV_PIX_FMT_YUV420P;;

	encodeContext_->gop_size = 30;
	encodeContext_->has_b_frames = 0;
	encodeContext_->max_b_frames = 0;
	encodeContext_->codec_id = codec->id;
	encodeContext_->me_subpel_quality = 0;
	encodeContext_->refs = 1;
	encodeContext_->trellis = 0;
	encodeContext_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	ret = avcodec_open2(encodeContext_, codec, 0);
	return ret;
}

int EasyProcess::initFilter(AVCodecContext * codecContext)
{
	char args[512] = {0};
	int ret = 0;
	const AVFilter *abuffersrc = avfilter_get_by_name("buffer");
	const AVFilter *abuffersink = avfilter_get_by_name("buffersink");
	outputs_ = avfilter_inout_alloc();
	inputs_ = avfilter_inout_alloc();

	string  filters_descr = "drawtext=fontfile=FreeSerif.ttf:fontcolor=black:fontsize=50:text=EasyNVR AI+:x=50:y=50";
	enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV420P };

	filterGraph_ = avfilter_graph_alloc();
	if (!outputs_ || !inputs_ || !filterGraph_)
	{
		ret = AVERROR(ENOMEM);
		return ret;
	}

	/* buffer video source: the decoded frames from the decoder will be inserted here. */
	sprintf(args, 
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		mediaInfo_.u32Width, mediaInfo_.u32Height, codecContext->pix_fmt,
		codecContext->time_base.num, codecContext->time_base.den,
		codecContext->sample_aspect_ratio.num, codecContext->sample_aspect_ratio.den);

	ret = avfilter_graph_create_filter(&buffersrcCtx_, abuffersrc, "in",
		args, NULL, filterGraph_);
	if (ret < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source: %d\n", ret);
		return ret;
	}

	ret = avfilter_graph_create_filter(&buffersinkCtx_, abuffersink, "out", NULL, NULL, filterGraph_);
	if (ret < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink. ret=%d\n", ret);
		return ret;
	}

	ret = av_opt_set_int_list(buffersinkCtx_, "pix_fmts", pix_fmts, AV_PIX_FMT_YUV420P, AV_OPT_SEARCH_CHILDREN);
	if (ret < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format: %d \n", ret);
		return ret;
	}

	/* Endpoints for the filter graph. */
	outputs_->name = av_strdup("in");
	outputs_->filter_ctx = buffersrcCtx_;
	outputs_->pad_idx = 0;
	outputs_->next = NULL;
	inputs_->name = av_strdup("out");
	inputs_->filter_ctx = buffersinkCtx_;
	inputs_->pad_idx = 0;
	inputs_->next = NULL;

	if ((ret = avfilter_graph_parse_ptr(filterGraph_, filters_descr.c_str(), &inputs_, &outputs_, NULL)) < 0)
	{
		return ret;
	}

	if ((ret = avfilter_graph_config(filterGraph_, NULL)) < 0)
	{
		return ret;
	}

	return 0;
}

AVFrame* EasyProcess::decode(AVPacket* packet)
{
	int ret = 0;
	AVCodecContext* codecContext = decodeContext_;
	AVFrame* pSrcFrame = av_frame_alloc();
	AVFrame *filtFrame = av_frame_alloc();

	if (!codecContext || !codecContext->codec)
	{
		goto error;
	}

	ret = avcodec_send_packet(codecContext, packet);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
	{
		goto error;
	}

	ret = avcodec_receive_frame(codecContext, pSrcFrame);
	if (ret < 0 && ret != AVERROR_EOF)
	{
		goto error;
	}

	if (av_buffersrc_add_frame(buffersrcCtx_, pSrcFrame) < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "buffe src add frame error!\n");
		goto error;
	}

	ret = av_buffersink_get_frame(buffersinkCtx_, filtFrame);
	if (ret < 0)
	{
		goto error;
	}

	av_frame_unref(pSrcFrame);
	av_frame_free(&pSrcFrame);
	pSrcFrame = NULL;
	return filtFrame;
error:
	if (filtFrame)
	{
		av_frame_unref(filtFrame);
		av_frame_free(&filtFrame);
		filtFrame = NULL;
	}
	if (pSrcFrame)
	{
		av_frame_unref(pSrcFrame);
		av_frame_free(&pSrcFrame);
		pSrcFrame = NULL;
	}
	return NULL;
}


AVPacket *EasyProcess::encode(AVFrame *filterFrame)
{
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	av_init_packet(packet);
	packet->data = NULL;
	packet->size = 0;

	int ret = avcodec_send_frame(encodeContext_, filterFrame);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
	{
		av_packet_unref(packet);
		av_free(packet);
		return NULL;
	}

	ret = avcodec_receive_packet(encodeContext_, packet);
	if (ret < 0 && ret != AVERROR_EOF)
	{
		char err[512] = { 0 };
		av_strerror(ret, err, 511);
		printf("%s %d err=%s\n", __FUNCTION__, __LINE__, err);
		av_packet_unref(packet);
		av_free(packet);
		return NULL;
	}

	return packet;
}

AVPacket* EasyProcess::Transcode(char* srcData, int srcSize)
{
	AVPacket* packet = av_packet_alloc();
	if (packet == NULL)
	{
		return NULL;
	}

	packet->data = (unsigned char*)srcData;
	packet->size = srcSize;

	AVFrame*  frame = decode(packet);
	if (frame)
	{
		AVPacket* outPacket = encode(frame);
		if (!outPacket)
		{
			return NULL;
		}

		av_frame_unref(frame);
		av_frame_free(&frame);
		av_packet_unref(packet);
		av_free(packet);
		return outPacket;
	}

	av_packet_unref(packet);
	av_free(packet);

	return NULL;
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
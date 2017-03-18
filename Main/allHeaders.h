#pragma once

#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <stdio.h>

#define LOG_LEVEL_1
#ifdef LOG_LEVEL_1
#define LOG_ERROR
#endif
#ifdef LOG_LEVEL_2
#define LOG_ERROR
#define LOG_WARRING
#endif
#ifdef LOG_LEVEL_3
#define LOG_ERROR
#define LOG_WARRING
#define LOG_NORMAL
#endif

#define CV_FOURCC_DEFAULT CV_FOURCC('D', 'I', 'V', 'X')

typedef struct _TrackState		//定义一个跟踪的Rect
{
	std::string targetName;		//目标名
	cv::Rect initialBBox;		//输出起始跟踪框位置
	cv::Rect boundingBox;		//跟踪目标的boundingbox
	int startFrameInd;			//跟踪起始帧号
	int endFrameInd;			//跟踪结束帧号
	int	framecount;				//跟踪帧数计数
}TrackState;

typedef enum _FrameHandleWay	//预处理各个主要步骤的处理信息输出
{
	PerformFail,				//0
	PerformReadFrame,			//1
	PerformDetect,				//2	
	PerformRecognize,			//3
	PerformTrack				//4	
}FrameHandleWay;



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

typedef struct _TrackState		//����һ�����ٵ�Rect
{
	std::string targetName;		//Ŀ����
	cv::Rect initialBBox;		//�����ʼ���ٿ�λ��
	cv::Rect boundingBox;		//����Ŀ���boundingbox
	int startFrameInd;			//������ʼ֡��
	int endFrameInd;			//���ٽ���֡��
	int	framecount;				//����֡������
}TrackState;

typedef enum _FrameHandleWay	//Ԥ���������Ҫ����Ĵ�����Ϣ���
{
	PerformFail,				//0
	PerformReadFrame,			//1
	PerformDetect,				//2	
	PerformRecognize,			//3
	PerformTrack				//4	
}FrameHandleWay;



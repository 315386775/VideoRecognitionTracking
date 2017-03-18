
//-----------------------------------【程序说明】----------------------------------------------  
/*
 MainFrame.cpp : 定义控制台应用程序的入口点。
----------------------------------------------------------------------------------------------
 author: 315386775
---------------------------------------------------------------------------------------------- */
#pragma once
#include "stdafx.h"
#include "Combination.h"
#include "Recognition\Classifier.h"


int _tmain(int argc, _TCHAR* argv[])
{
	
	//路径：【视频、保存、caffe模型、人脸检测器、目标人物】
	string videoFile = "E:\\VideoSystem\\test1.flv";
	string capturedPath = "E:\\VideoSystem\\saveinfo\\";
	string caffeModelPath = "E:\\VideoSystem\\model\\";
	string haarCascadePath = "D:\\Opencv\\sources\\data\\haarcascades\\";

	//string haarCascadePath = "D:\\Opencv\\build\\x64\\vc12\\bin\\data\\";
	//string haarCascadePath = "D:\\Opencv\\sources\\data\\haarcascades_cuda\\";

	string targetName = "xi";

	//参数：【输出大小、帧率】
	int input_params[3] = { 960, 540, 25 };  
	
	VideoSystem videosysAPI;

	if (videosysAPI.InitClassifiers(haarCascadePath, caffeModelPath) != 1)
	{
		#ifdef LOG_ERRER
			std::cout <<"分类器初始化错误！请检查相关路径是否正确。" << std::endl;
		#endif
	}
	if (videosysAPI.setCaptureSzie(input_params) != 1)
	{
		#ifdef LOG_ERRER
			std::cout << "Error. 请输入必要的参数" << std::endl;
		#endif
	}

	videosysAPI.setInputAndOutput(videoFile, capturedPath);
	
	videosysAPI.videoAnnotation(targetName);

	videosysAPI.videoSaveClips(videoFile, capturedPath, targetName); 
	
	return 0;
}


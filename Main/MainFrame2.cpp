
//-----------------------------------������˵����----------------------------------------------  
/*
 MainFrame.cpp : �������̨Ӧ�ó������ڵ㡣
----------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------- */
#pragma once
#include "stdafx.h"
#include "Combination.h"
#include "Recognition\Classifier.h"


int _tmain(int argc, _TCHAR* argv[])
{
	
	//·��������Ƶ�����桢caffeģ�͡������������Ŀ�����
	string videoFile = "E:\\VideoSystem\\test1.flv";
	string capturedPath = "E:\\VideoSystem\\saveinfo\\";
	string caffeModelPath = "E:\\VideoSystem\\model\\";
	string haarCascadePath = "D:\\Opencv\\sources\\data\\haarcascades\\";

	//string haarCascadePath = "D:\\Opencv\\build\\x64\\vc12\\bin\\data\\";
	//string haarCascadePath = "D:\\Opencv\\sources\\data\\haarcascades_cuda\\";

	string targetName = "xi";

	//�������������С��֡�ʡ�
	int input_params[3] = { 960, 540, 25 };  
	
	VideoSystem videosysAPI;

	if (videosysAPI.InitClassifiers(haarCascadePath, caffeModelPath) != 1)
	{
		#ifdef LOG_ERRER
			std::cout <<"��������ʼ�������������·���Ƿ���ȷ��" << std::endl;
		#endif
	}
	if (videosysAPI.setCaptureSzie(input_params) != 1)
	{
		#ifdef LOG_ERRER
			std::cout << "Error. �������Ҫ�Ĳ���" << std::endl;
		#endif
	}

	videosysAPI.setInputAndOutput(videoFile, capturedPath);
	
	videosysAPI.videoAnnotation(targetName);

	videosysAPI.videoSaveClips(videoFile, capturedPath, targetName); 
	
	return 0;
}


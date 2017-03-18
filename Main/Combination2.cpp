//-----------------------------------【程序说明】----------------------------------------------  
/*
作者：LiRui
版本: 0.1
程序名称: 视频人脸识别及跟踪
创建时间：2016年4月7日 
程序说明：
----------------------------------------------------------------------------------------------

修改日志：添加了Struck算法的人脸检测跟踪
程序说明：检测到人脸后开始跟踪，如果失去跟踪目标可按B键重新检测
修改时间：2016年7月5日 HaoXintong
----------------------------------------------------------------------------------------------
修改日志：添加了人脸清晰度判断以及人脸识别
程序说明：检测到人脸后进行清晰度判断
修改时间：2016年7月15日 李亚伟
---------------------------------------------------------------------------------------------- */

#pragma once
#include "stdafx.h"
#include "Combination.h"
#include <fstream>


using namespace cv;
using namespace std;

int VideoSystem::InitClassifiers(string haarPath, string caffePath)
{
	//初始化haar人脸检测分类器和caffe人脸识别分类器，若初始化失败，返回0，否则返回1
	return (haarFaceDetector_.LoadCascade(haarPath, false) && cnnFaceClassifier_.LoadModelFile(caffePath));
}

int VideoSystem::setInputAndOutput(string videoFile, string savePath)
{
	#ifdef LOG_NORMAL
		cout <<"输入视频：\n"<<videoFile << endl;
	#endif

	inVideo_.open(videoFile); //打开视频	
	if (!inVideo_.isOpened())
	{
		#ifdef LOG_ERROR
			cout << "fail to open the video file:!" << videoFile << endl;
		#endif
		return 0;
	}

	totalFrames_ = inVideo_.get(CV_CAP_PROP_FRAME_COUNT) - 10;

	#ifdef LOG_NORMAL
		cout << "整个视频共" << totalFrames_ << "帧" << endl;
	#endif
	int startPos = videoFile.find_last_of("\\");
	int endPos = videoFile.find_last_of(".");
	saveName_ = savePath + videoFile.substr(startPos + 1, (endPos - 1 - startPos));
	char saveStr[100];
	sprintf(saveStr, "%s_out.avi", saveName_);
	outVideo_.open(saveStr, CV_FOURCC_DEFAULT, outFrameRate_, geometryImgSize_, true);  //保存标注后的视频
	return 1;
}

int VideoSystem::setCaptureSzie(int input_size[3])
{
	geometryImgSize_ = cv::Size(input_size[0], input_size[1]);
	outFrameRate_ = input_size[2];
	return 1;
}

int VideoSystem::setSampleFrequnency(int frequency)
{
	if (frequency <= 0)
	{
        #ifdef LOG_ERROR
			cout << "Error:the sampling frequency must is larger than zero！" << endl;
		#endif
		return 0;
	}
	int sampleFreq;
	sampleFreq = frequency;
	return 1;
}

int VideoSystem ::getNextFrame(Mat *frame)
{
	if (!inVideo_.isOpened()||frame==NULL)   
	{
		return 0;
	}
	inVideo_ >> (*frame);		
	return 1;
}

void VideoSystem::saveFrame(Mat frame, int frameInd)
{
	Mat gray;
	char frame_ind[20];
	sprintf(frame_ind, "_frame_%d.jpg", frameInd);
	cvtColor(frame, gray, CV_BGR2GRAY);
	imwrite(saveName_ + frame_ind, gray);
}

void VideoSystem::saveFaces(Mat faceImg, int frameInd)
{
	Mat gray;
	char faces_ind[20];
	sprintf(faces_ind, "_faces_%d.jpg", frameInd);
	cvtColor(faceImg, gray, CV_BGR2GRAY);
	imwrite(saveName_ + faces_ind, gray);
}

//输出顺序：目标名称，起始帧数，持续帧数，目标框坐标
void VideoSystem::saveCsv(TrackState saveinfo_)
{
	FILE *outCsv;
	outCsv = fopen( "testout_.csv", "a");
	fprintf(outCsv, "%s, %d, %d, %d, %d, %d, %d \n",saveinfo_.targetName,
												 saveinfo_.startFrameInd, 
												 saveinfo_.endFrameInd - saveinfo_.startFrameInd, 
												 saveinfo_.initialBBox.x,
												 saveinfo_.initialBBox.y,
												 saveinfo_.initialBBox.x + saveinfo_.initialBBox.width,
												 saveinfo_.initialBBox.y + saveinfo_.initialBBox.height);
	videoClips.push_back(saveinfo_);
	fclose(outCsv);
}

void VideoSystem::writeCurrFrame()
{
	if (writeFrame_)
		outVideo_ << imgForShow_;
}

void VideoSystem::videoSaveClips(string videoFile, string savePath, string targetName)
{
	int Num = 0;
	char saveString[100];
	vector<TrackState> ::iterator iter;
	for (iter = videoClips.begin(); iter <= videoClips.end();iter++,Num++)
	{	
		sprintf(saveString, "%s_%s_out%d.avi",saveName_,targetName, Num);
		writeVideoClips(saveString, iter->startFrameInd, iter->endFrameInd);	
	}
}

void VideoSystem::writeVideoClips(string saveString, int startframe,int endframe)
{
	int temp = startframe;
	VideoWriter videoWriter(saveString, CV_FOURCC_DEFAULT, outFrameRate_, geometryImgSize_, true);
	inVideo_.set(CV_CAP_PROP_POS_FRAMES,startframe);
	while (temp <= endframe)
	{
		if (convertToReadFrame() == 0)
			break;
		videoWriter << imgForShow_;
		cv::imshow("videoClips", imgForShow_);
		cvWaitKey(1);
		temp++;
	}
}

bool VideoSystem::rectifyTrackTarget()
{
	string recogName = cnnFaceClassifier_.recognizeFaces(currTrackState.boundingBox, currFrame_, imgForShow_);
	readFrame_ = true;
	if (recogName != currTrackState.targetName)
	{
		currTrackState.framecount = 0;
		return false;
	}
	else
	{
		currTrackState.framecount++;
		return true;
	}
}

bool VideoSystem::recognizeDetectedTarget()
{
	string recogName = cnnFaceClassifier_.recognizeFaces(detectedFaces[0], currFrame_, imgForShow_);
	if (recogName == interestName_)
	{
		//名字改变，或者前后检测间隔大于5s的时候输出上次的信息，并且更新结构体中的信息
		if (recogName != currTrackState.targetName
			|| (frameInd_ - (currTrackState.startFrameInd + currTrackState.framecount)) >= 120)
		{
			if (currTrackState.startFrameInd != 0)
				saveCsv(currTrackState);
			currTrackState.startFrameInd = frameInd_;
			currTrackState.initialBBox = detectedFaces[0];

		}
		currTrackState.framecount = 0;
		currTrackState.boundingBox = detectedFaces[0];
		currTrackState.targetName = recogName;
		currTrackState.startFrameInd = frameInd_;
		detectedFaces.clear();
		return true;
	}
	else
		return false;
}

FrameHandleWay VideoSystem::convertToReadFrame()
{
	//读取视频帧，并更新frame_forShow 和detectedFaces
	if (getNextFrame(&currFrame_) != 1)
	{
		#ifdef LOG_ERROR
			cout << "无法读取视频帧，请检查视频文件是否打开或其他"<< endl;
		#endif
		return PerformFail;
	}
	frameInd_ += 1;
	#ifdef LOG_NOMARL
		cout << "frame:" << frameInd_ << endl;
	#endif
	resize(currFrame_, currFrame_, geometryImgSize_, 0, 0, CV_INTER_LINEAR); //读取视频帧并进行尺寸归一化	
	imgForShow_ = currFrame_.clone();
	return  PerformDetect;
}

FrameHandleWay VideoSystem::convertToDetect()
{
	detectedFaces.clear();	//清空
	if (!haarFaceDetector_.detectFaces(imgForShow_, &detectedFaces))//检测到的人脸向量 不为空，执行识别
		return PerformRecognize;
		//return PerformDetect;
	else
		return PerformDetect;		
}

FrameHandleWay VideoSystem::convertToRecognize()
{
	bool nextSwitchModel;
	if (currTrackState.framecount % 50 == 0 && readFrame_ == false)
	{
		#ifdef LOG_NORMAL
			cout << "矫正检验" << endl;
		#endif
		nextSwitchModel = rectifyTrackTarget();
	}
	else
	{
		#ifdef LOG_NORMAL
			cout << "检测识别" << endl;
		#endif
		nextSwitchModel = recognizeDetectedTarget();
	}
	if (nextSwitchModel)
		return PerformTrack;
	else
		return PerformDetect;
}

FrameHandleWay VideoSystem::convertToTrack()
{
	if (currTrackState.framecount > 0 && currTrackState.framecount % 50 == 0)
	{
		return PerformRecognize;
	}
	currTrackState.endFrameInd = frameInd_;
	return svmTracker_.trackFaces(currFrame_, &currTrackState, imgForShow_); //跟踪	 
}

//程序选择方式入口，方式1：检测；方式2：识别；方式3：跟踪
//输入参数：感兴趣目标名字
//返回：
int VideoSystem::videoAnnotation( string trackName)
{
	char Control_byhand;
	processway = PerformDetect;
	writeFrame_ = true;
	readFrame_ = true;
	frameInd_ = 0;
	interestName_ = trackName;
	bool stop = false;   //定义一个用来控制读取视频循环结束的变量 
	while(!stop)
	{
		if (frameInd_ >= totalFrames_ )
		{
			saveCsv(currTrackState);//视频结束时输出最后一段的信息
			stop = true;
		}
		if (readFrame_) 
			if (convertToReadFrame() == 0 )
				break;

		//switch (processway)
		switch (processway)
		{

		case PerformDetect:
		{	
			#ifdef LOG_NORMAL
				cout << "---检测" << endl;
			#endif
			processway = convertToDetect();
			if (processway == PerformDetect)
				break;
		}

		case PerformRecognize:
		{
			#ifdef LOG_NORMAL
				cout << "---识别" << endl;
			#endif
			processway = convertToRecognize();
			if (processway==PerformDetect)
				break;
		}

		case PerformTrack:
		{
			#ifdef LOG_NORMAL
				cout << "--跟踪" << endl;
			#endif
			processway = convertToTrack();
			if (processway != PerformTrack) //跳出跟踪，写CSV文件
			{
				//saveCsvFile(currTrackState);
				char temp[500];
				sprintf(temp, "Name:%s,startFrameInd:%d,trackCount:%d", currTrackState.targetName, currTrackState.startFrameInd, currTrackState.framecount);
				#ifdef LOG_NORMAL
					cout << temp << endl;
				#endif
			}
			break;
		}

		default:
			#ifdef LOG_ERROR
				cout << "--无效" << endl;
			#endif
		}

		outVideo_ << imgForShow_;
		cv::imshow("VideoAnnotation", imgForShow_);
		Control_byhand = waitKey(1);
		if (Control_byhand == 'd')      //手动控制切换至跟踪模式
		{  
			processway = PerformDetect;
		}	
	} //while 循环读取帧
	return 1;
}


#pragma once
#include "stdafx.h"
#include "Detection\detection.h"
#include "Tracking.h"
#include "Recognition\Classifier.h"
#include "allHeaders.h"


class VideoSystem
{
	public:
		int InitClassifiers(string haarPath, string caffePath);
		int setInputAndOutput(string videoFile, string savePath);
		int setSampleFrequnency(int frequency);		           //未用到
		int videoAnnotation(string targetName);		
		int setCaptureSzie(int input_size[3]);
		void videoSaveClips(string videoFile, string savePath, string targetName);
		void writeVideoClips(string saveString, int startframe, int endframe);
		bool rectifyTrackTarget();
		bool recognizeDetectedTarget();
		FrameHandleWay processway;             
		TrackState currTrackState;              
		cv::VideoCapture inVideo_;
		cv::VideoWriter outVideo_;

	private:

		void saveFaces(cv::Mat faceImg, int frameInd);
		void saveFrame(cv::Mat frame, int frameInd);
		void saveCsv(TrackState saveinfo_);
		int getNextFrame(cv::Mat *frame);
		void writeCurrFrame();

		FrameHandleWay convertToReadFrame();
		FrameHandleWay convertToDetect();
		FrameHandleWay convertToRecognize();
		FrameHandleWay convertToTrack();

		Detection haarFaceDetector_;		    //haar特征人脸检测器
		Classifier cnnFaceClassifier_;	    	//caffe cnn人脸分类器
		Tracking svmTracker_;				    //struck SVM - kcf 跟踪器
		
		
		bool readFrame_;						//读视频帧
		bool writeFrame_;						//写视频标记
		long totalFrames_;					    //视频总帧数
		long frameInd_;						    //用于标记当前图像帧号
		int outFrameRate_;				    	//输出视频帧率
		string saveName_;				
		string interestName_;
		cv::Mat currFrame_;						//当前帧图像
		cv::Mat imgForShow_;					//用于显示结果图像
		cv::Size geometryImgSize_;			    //处理过程中使用的归一化尺寸
		
		vector<cv::Rect> detectedFaces;			//检测到的视频
		vector<TrackState> videoClips;			//保存剪辑视频的帧数
};
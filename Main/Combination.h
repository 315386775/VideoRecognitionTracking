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
		int setSampleFrequnency(int frequency);		           //δ�õ�
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

		Detection haarFaceDetector_;		    //haar�������������
		Classifier cnnFaceClassifier_;	    	//caffe cnn����������
		Tracking svmTracker_;				    //struck SVM - kcf ������
		
		
		bool readFrame_;						//����Ƶ֡
		bool writeFrame_;						//д��Ƶ���
		long totalFrames_;					    //��Ƶ��֡��
		long frameInd_;						    //���ڱ�ǵ�ǰͼ��֡��
		int outFrameRate_;				    	//�����Ƶ֡��
		string saveName_;				
		string interestName_;
		cv::Mat currFrame_;						//��ǰ֡ͼ��
		cv::Mat imgForShow_;					//������ʾ���ͼ��
		cv::Size geometryImgSize_;			    //���������ʹ�õĹ�һ���ߴ�
		
		vector<cv::Rect> detectedFaces;			//��⵽����Ƶ
		vector<TrackState> videoClips;			//���������Ƶ��֡��
};
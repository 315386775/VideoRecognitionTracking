//-----------------------------------������˵����----------------------------------------------  
/*

---------------------------------------------------------------------------------------------- */

#pragma once
#include "stdafx.h"
#include "Combination.h"
#include <fstream>


using namespace cv;
using namespace std;

int VideoSystem::InitClassifiers(string haarPath, string caffePath)
{
	//��ʼ��haar��������������caffe����ʶ�������������ʼ��ʧ�ܣ�����0�����򷵻�1
	return (haarFaceDetector_.LoadCascade(haarPath, false) && cnnFaceClassifier_.LoadModelFile(caffePath));
}

int VideoSystem::setInputAndOutput(string videoFile, string savePath)
{
	#ifdef LOG_NORMAL
		cout <<"������Ƶ��\n"<<videoFile << endl;
	#endif

	inVideo_.open(videoFile); //����Ƶ	
	if (!inVideo_.isOpened())
	{
		#ifdef LOG_ERROR
			cout << "fail to open the video file:!" << videoFile << endl;
		#endif
		return 0;
	}

	totalFrames_ = inVideo_.get(CV_CAP_PROP_FRAME_COUNT) - 10;

	#ifdef LOG_NORMAL
		cout << "������Ƶ��" << totalFrames_ << "֡" << endl;
	#endif
	int startPos = videoFile.find_last_of("\\");
	int endPos = videoFile.find_last_of(".");
	saveName_ = savePath + videoFile.substr(startPos + 1, (endPos - 1 - startPos));
	char saveStr[100];
	sprintf(saveStr, "%s_out.avi", saveName_);
	outVideo_.open(saveStr, CV_FOURCC_DEFAULT, outFrameRate_, geometryImgSize_, true);  //�����ע�����Ƶ
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
			cout << "Error:the sampling frequency must is larger than zero��" << endl;
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

//���˳��Ŀ�����ƣ���ʼ֡��������֡����Ŀ�������
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
		//���ָı䣬����ǰ����������5s��ʱ������ϴε���Ϣ�����Ҹ��½ṹ���е���Ϣ
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
	//��ȡ��Ƶ֡��������frame_forShow ��detectedFaces
	if (getNextFrame(&currFrame_) != 1)
	{
		#ifdef LOG_ERROR
			cout << "�޷���ȡ��Ƶ֡��������Ƶ�ļ��Ƿ�򿪻�����"<< endl;
		#endif
		return PerformFail;
	}
	frameInd_ += 1;
	#ifdef LOG_NOMARL
		cout << "frame:" << frameInd_ << endl;
	#endif
	resize(currFrame_, currFrame_, geometryImgSize_, 0, 0, CV_INTER_LINEAR); //��ȡ��Ƶ֡�����гߴ��һ��	
	imgForShow_ = currFrame_.clone();
	return  PerformDetect;
}

FrameHandleWay VideoSystem::convertToDetect()
{
	detectedFaces.clear();	//���
	if (!haarFaceDetector_.detectFaces(imgForShow_, &detectedFaces))//��⵽���������� ��Ϊ�գ�ִ��ʶ��
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
			cout << "��������" << endl;
		#endif
		nextSwitchModel = rectifyTrackTarget();
	}
	else
	{
		#ifdef LOG_NORMAL
			cout << "���ʶ��" << endl;
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
	return svmTracker_.trackFaces(currFrame_, &currTrackState, imgForShow_); //����	 
}

//����ѡ��ʽ��ڣ���ʽ1����⣻��ʽ2��ʶ�𣻷�ʽ3������
//�������������ȤĿ������
//���أ�
int VideoSystem::videoAnnotation( string trackName)
{
	char Control_byhand;
	processway = PerformDetect;
	writeFrame_ = true;
	readFrame_ = true;
	frameInd_ = 0;
	interestName_ = trackName;
	bool stop = false;   //����һ���������ƶ�ȡ��Ƶѭ�������ı��� 
	while(!stop)
	{
		if (frameInd_ >= totalFrames_ )
		{
			saveCsv(currTrackState);//��Ƶ����ʱ������һ�ε���Ϣ
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
				cout << "---���" << endl;
			#endif
			processway = convertToDetect();
			if (processway == PerformDetect)
				break;
		}

		case PerformRecognize:
		{
			#ifdef LOG_NORMAL
				cout << "---ʶ��" << endl;
			#endif
			processway = convertToRecognize();
			if (processway==PerformDetect)
				break;
		}

		case PerformTrack:
		{
			#ifdef LOG_NORMAL
				cout << "--����" << endl;
			#endif
			processway = convertToTrack();
			if (processway != PerformTrack) //�������٣�дCSV�ļ�
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
				cout << "--��Ч" << endl;
			#endif
		}

		outVideo_ << imgForShow_;
		cv::imshow("VideoAnnotation", imgForShow_);
		Control_byhand = waitKey(1);
		if (Control_byhand == 'd')      //�ֶ������л�������ģʽ
		{  
			processway = PerformDetect;
		}	
	} //while ѭ����ȡ֡
	return 1;
}


//-----------------------------------������˵����----------------------------------------------  
/*

---------------------------------------------------------------------------------------------- */

#pragma once

#include "Combination.h"
#include <fstream>
#define DETECT 1
#define ClASSIFER 2
#define TRACK 3

void VideoSystem::videoMainFunction(string videoFile, string capturedPath, string model_file,
	string trained_file, string mean_file, string label_file, string face_cascade_name, string eyes_cascade_name, string trackName)
{	
	//viedoCapture(capturedPath,vedioPath);

	setInputAndOutput(videoFile, capturedPath);

	setSampleFrequnency(50);

	videoAnnotation(model_file,
		trained_file, mean_file, label_file, face_cascade_name, eyes_cascade_name, trackName);

}

//��ȡ������Ƶ֡���������ò���ͼƬ����
uint8_t VideoSystem::setInputAndOutput(string videoFile, string savePath)
{
	capture.open(videoFile); //����Ƶ	
	if (!capture.isOpened())
	{
		cout << "fail to open the video file:!" << videoFile << endl;
		return 0;
	}
	//��ȡ����֡��
	totalFrames = capture.get(CV_CAP_PROP_FRAME_COUNT);
	cout << "������Ƶ��" << totalFrames << "֡" << endl;
	//�����������
	int startPos = videoFile.find_last_of("\\");
	int endPos = videoFile.find_last_of(".");
	saveName = savePath + videoFile.substr(startPos + 1, (endPos - 1 - startPos));
	return 1;
}

//���ò���Ƶ��
uint8_t VideoSystem::setSampleFrequnency(int frequency)
{
	if (frequency <= 0)
	{
		cout << "Error:the sampling frequency must is larger than zero��" << endl;
		return 0;
	}
	int sampleFreq;
	sampleFreq = frequency;
	return 1;
}

//������Ƶ֡
void VideoSystem::saveFrame(Mat frame, int frameInd)
{
	Mat gray;
	char frame_ind[20];
	sprintf(frame_ind, "_frame_%d.jpg", frameInd);
	//�ҶȻ�
	cvtColor(frame, gray, CV_BGR2GRAY);
	imwrite(saveName + frame_ind, gray);
}

//��������
void VideoSystem::saveFaces(Mat faceImg, int facesInd)
{
	Mat gray;
	char faces_ind[20];
	sprintf(faces_ind, "_faces_%d.jpg", facesInd);
	//�ҶȻ�
	cvtColor(faceImg, gray, CV_BGR2GRAY);
	//resize(gray,gray,Size(480,320),0,0,CV_INTER_LINEAR);
	imwrite(saveName + faces_ind, gray);
}

//����ʶ����ٵ�������Ϣ��CSV�ļ��У�BegΪ��ʼ֡��EndΪ����֡
void VideoSystem::saveCsvBeg(std::vector<Rect> faces, string *label, int frame_Begin)
{
	Rect rect(faces[0]);
	char filename[100];
	int check_frameBe = frame_Begin;
	ofstream ftest("output.csv", ios::app);  // �ļ���������׷�ӵ���ʽ�����ȥ
	//if ((frame_Begin - check_frame)>=0 && (frame_Begin - check_frame)<=5)
	//{
	//	//����µĿ�ʼ֡��ȥ����֡С�ڵ���5���Ͳ����б��档
	//}
	//else
	//{
	//	ftest << *label << "," << "begin" << "," << frame_Begin << "," << rect.x << "," << 
	//		rect.y << "," << rect.x + rect.width << "," << rect.y + rect.height << endl;
	//}
	ftest << *label << "," << "begin" << "," << frame_Begin << "," << rect.x << "," <<
		rect.y << "," << rect.x + rect.width << "," << rect.y + rect.height << endl;
}

void VideoSystem::saveCsvEnd(Rect track_boxing, string* label, int frame_End)
{
	int check_frame = frame_End;
	ofstream ftest("output.csv", ios::app);  // �ļ���������׷�ӵ���ʽ�����ȥ
	//if ((check_frameBe - frame_End) >= 0 && (check_frameBe - frame_End) <= 5)
	//{
	//	//�����б���
	//}
	//else
	//{
	//	ftest << *label << "," << "end" << "," << frame_End << "," << track_boxing.x << "," << track_boxing.y << "," << 
	//		track_boxing.x + track_boxing.width << "," << track_boxing.y + track_boxing.height << endl;
	//	//sprintf("capture4.csv", "%s","%d", *label, frame_Ind); // C4996
	//}
	ftest << *label << "," << "end" << "," << frame_End << "," << track_boxing.x << "," << track_boxing.y << "," <<
		track_boxing.x + track_boxing.width << "," << track_boxing.y + track_boxing.height << endl;
}

//����ѡ��ʽ��ڣ���ʽ1����⣻��ʽ2��ʶ�𣻷�ʽ3������
uint8_t VideoSystem::videoAnnotation(string model_file,
	string trained_file, string mean_file, string label_file, string face_cascade_name, string eyes_cascade_name, string trackName)
{
	
	Classifier Myclass;
	myclass.load(model_file, trained_file, mean_file, label_file);
	Mydetect.LoadCascade(face_cascade_name, eyes_cascade_name);
	Mat frame, gray;
	char Control_byhand;
	processway = 1;
	int FrameRate = 25;
	
	VideoWriter outVideo("out1.avi", CV_FOURCC('D', 'I', 'V', 'X'), 25.0, Size(960, 540), true);  //�����ע�����Ƶ

	for (int i = 0; i<500; i++)  //totalFrames
	{
		capture >> frame;	
		resize(frame, frame, Size(960,540), 0, 0, CV_INTER_LINEAR);
		
		Mat frame_out = frame.clone();   //���֡�������Ĵ�����frame_out������

		switch (processway)
		{
			case DETECT:
			{		
				faces = Mydetect.detectFaces(frame_out, &processway, &frame_out);                                //���
				tracking_init = 0;
				if (faces.size()!=0)
					info_bb.face_bb = Rect(faces[0].x, faces[0].y, faces[0].width, faces[0].height);   
				break;
			}
			case ClASSIFER:
			{
				
				info_bb.label = Myclass.classFaces(info_bb.face_bb, frame, &processway, trackName);         //ʶ��				
				string label_name = info_bb.label;
				string* label = &label_name;
				if (*label == trackName)
				{
					saveCsvBeg(faces, label, i);     // ����ʶ�𵽸���Ȥ�������ʼ֡��CSV�ļ���
				}
				break;
			}
			case TRACK:
			{				
				info_bb.track_boxing = Mytracker.trackFaces(i, frame, &(info_bb.face_bb), &tracking_init, &processway, &frame_out); //����
				string label_name = info_bb.label;
				string* label = &label_name;
				if (*label == trackName)
				{
					saveCsvEnd(info_bb.track_boxing, label, i);     // ����ʶ�𵽸���Ȥ����Ľ���֡��CSV�ļ���
				}
				break;
			}
			default:
				cout << "Some mistakes may be occurred" << endl;
		}   //switchѡ��
	
		imshow("detected-track", frame_out);

		waitKey(1);

		outVideo << frame_out;     //����Ƶ֡���浽�������Ƶ����

		Control_byhand = waitKey(1);   

		if (Control_byhand == 'b')      //�ֶ������л�������ģʽ
		{  
			processway = 1;	
		}	
	}   //ѭ����ȡ֡

	return 1;
}


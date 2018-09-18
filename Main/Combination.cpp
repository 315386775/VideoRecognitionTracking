//-----------------------------------【程序说明】----------------------------------------------  
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

//读取整个视频帧数，并设置捕获图片名称
uint8_t VideoSystem::setInputAndOutput(string videoFile, string savePath)
{
	capture.open(videoFile); //打开视频	
	if (!capture.isOpened())
	{
		cout << "fail to open the video file:!" << videoFile << endl;
		return 0;
	}
	//获取整个帧数
	totalFrames = capture.get(CV_CAP_PROP_FRAME_COUNT);
	cout << "整个视频共" << totalFrames << "帧" << endl;
	//设置输出名字
	int startPos = videoFile.find_last_of("\\");
	int endPos = videoFile.find_last_of(".");
	saveName = savePath + videoFile.substr(startPos + 1, (endPos - 1 - startPos));
	return 1;
}

//设置采样频率
uint8_t VideoSystem::setSampleFrequnency(int frequency)
{
	if (frequency <= 0)
	{
		cout << "Error:the sampling frequency must is larger than zero！" << endl;
		return 0;
	}
	int sampleFreq;
	sampleFreq = frequency;
	return 1;
}

//保存视频帧
void VideoSystem::saveFrame(Mat frame, int frameInd)
{
	Mat gray;
	char frame_ind[20];
	sprintf(frame_ind, "_frame_%d.jpg", frameInd);
	//灰度化
	cvtColor(frame, gray, CV_BGR2GRAY);
	imwrite(saveName + frame_ind, gray);
}

//保存人脸
void VideoSystem::saveFaces(Mat faceImg, int facesInd)
{
	Mat gray;
	char faces_ind[20];
	sprintf(faces_ind, "_faces_%d.jpg", facesInd);
	//灰度化
	cvtColor(faceImg, gray, CV_BGR2GRAY);
	//resize(gray,gray,Size(480,320),0,0,CV_INTER_LINEAR);
	imwrite(saveName + faces_ind, gray);
}

//保存识别跟踪的坐标信息到CSV文件中，Beg为初始帧，End为结束帧
void VideoSystem::saveCsvBeg(std::vector<Rect> faces, string *label, int frame_Begin)
{
	Rect rect(faces[0]);
	char filename[100];
	int check_frameBe = frame_Begin;
	ofstream ftest("output.csv", ios::app);  // 文件中数据已追加的形式保存进去
	//if ((frame_Begin - check_frame)>=0 && (frame_Begin - check_frame)<=5)
	//{
	//	//如果新的开始帧减去结束帧小于等于5，就不进行保存。
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
	ofstream ftest("output.csv", ios::app);  // 文件中数据已追加的形式保存进去
	//if ((check_frameBe - frame_End) >= 0 && (check_frameBe - frame_End) <= 5)
	//{
	//	//不进行保存
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

//程序选择方式入口，方式1：检测；方式2：识别；方式3：跟踪
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
	
	VideoWriter outVideo("out1.avi", CV_FOURCC('D', 'I', 'V', 'X'), 25.0, Size(960, 540), true);  //保存标注后的视频

	for (int i = 0; i<500; i++)  //totalFrames
	{
		capture >> frame;	
		resize(frame, frame, Size(960,540), 0, 0, CV_INTER_LINEAR);
		
		Mat frame_out = frame.clone();   //深拷贝帧，后续的处理在frame_out上面做

		switch (processway)
		{
			case DETECT:
			{		
				faces = Mydetect.detectFaces(frame_out, &processway, &frame_out);                                //检测
				tracking_init = 0;
				if (faces.size()!=0)
					info_bb.face_bb = Rect(faces[0].x, faces[0].y, faces[0].width, faces[0].height);   
				break;
			}
			case ClASSIFER:
			{
				
				info_bb.label = Myclass.classFaces(info_bb.face_bb, frame, &processway, trackName);         //识别				
				string label_name = info_bb.label;
				string* label = &label_name;
				if (*label == trackName)
				{
					saveCsvBeg(faces, label, i);     // 保存识别到感兴趣人物的起始帧到CSV文件中
				}
				break;
			}
			case TRACK:
			{				
				info_bb.track_boxing = Mytracker.trackFaces(i, frame, &(info_bb.face_bb), &tracking_init, &processway, &frame_out); //跟踪
				string label_name = info_bb.label;
				string* label = &label_name;
				if (*label == trackName)
				{
					saveCsvEnd(info_bb.track_boxing, label, i);     // 保存识别到感兴趣人物的结束帧到CSV文件中
				}
				break;
			}
			default:
				cout << "Some mistakes may be occurred" << endl;
		}   //switch选择
	
		imshow("detected-track", frame_out);

		waitKey(1);

		outVideo << frame_out;     //将视频帧保存到输出的视频流中

		Control_byhand = waitKey(1);   

		if (Control_byhand == 'b')      //手动控制切换检测跟踪模式
		{  
			processway = 1;	
		}	
	}   //循环读取帧

	return 1;
}


#pragma once
#include "detection.h"

using namespace cv;
using namespace std;

int Detection::faceDetectDrawByHaar(Mat &img)
{	
	RNG rng(12345);
	Mat frame_gray;
	if (img.channels() == 3)
	{
		cvtColor(img, frame_gray, CV_BGR2GRAY); 
	}
	else if (img.channels() == 4)
	{
		cvtColor(img, frame_gray, CV_BGR2GRAY); 
	}
	else
	{
		frame_gray = img;
	}
	equalizeHist(frame_gray, frame_gray);	//Histogram equalization
	int flags = CV_HAAR_DO_CANNY_PRUNING;
	// the min Object;  
	Size minFeatureSize = Size(40, 40);
	// the vlaue must be over one;  
	float searchScaleFactor = 1.1f;
	int minNeighbors = 4;

	face_cascade.detectMultiScale(frame_gray, faces, searchScaleFactor, minNeighbors, 0 | flags, minFeatureSize);
	
	for (vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++)
		rectangle(img, cvPoint(r->x, r->y), cvPoint(r->x + r->width - 1, r->y + r->height - 1), Scalar(255, 0, 255), 3, 8, 0);
	  
	for (int i = 0; i < (int)faces.size(); i++) 
	{
		if (faces[i].x < 0)
			faces[i].x = 0;
		if (faces[i].y < 0)
			faces[i].y = 0;
		if (faces[i].x + faces[i].width > img.cols)
			faces[i].x = img.cols - faces[i].width;
		if (faces[i].y + faces[i].height > img.rows)
			faces[i].y = img.rows - faces[i].height;
	}

	return faces.size(); //detect the faces
}

void Detection::LoadCascade(string haarPath, bool enableEyeDetector)
{	
	
	try {  
		face_cascade.load(haarPath + "haarcascade_frontalface_alt.xml");
	}
	catch (cv::Exception &e) {}
	if (face_cascade.empty()) 
	{
		cerr << "ERROR: Could not load Face Detection cascade classifier [" << haarPath << "]!" << endl;
		cerr << "Copy the file from your OpenCV data folder (eg: 'D:\\OpenCV\\data\\haarcascades') into this WebcamFaceRec folder." << endl;
		exit(1);
	}
	#ifdef LOG_NORMAL
		cout << "Loaded the Face Detection cascade classifier [" << haarPath << "]." << endl;
	#endif // LOG_NORMAL

	try {   
		enableEyeDetector = true;
		eyes_cascade.load(haarPath + "haarcascade_eye_tree_eyeglasses.xml");
	}
	catch (cv::Exception &e) {}
	if (eyes_cascade.empty()) 
	{
		cerr << "ERROR: Could not load 1st Eye Detection cascade classifier [" << haarPath << "]!" << endl;
		cerr << "Copy the file from your OpenCV data folder (eg: 'D:\\OpenCV\\data\\haarcascades') into this WebcamFaceRec folder." << endl;
		exit(1);
	}
	#ifdef LOG_NORMAL
		cout << "Loaded the 1st Eye Detection cascade classifier [" << haarPath << "]." << endl;
	#endif // LOG_NORMAL
}

float Detection::ImgisClear(Mat faceImg)
{
	int countup = 0;
	int countdown = 0;
	int tempGradient;
	int height = faceImg.rows - 1;
	int width = faceImg.cols - 1;
	for (int j = 1; j < height; j++)
	{
		uchar * data0 = faceImg.ptr(j - 1);
		uchar * data1 = faceImg.ptr(j);
		uchar * data2 = faceImg.ptr(j + 1);
		for (int i = 1; i < width; i++)
		{
			tempGradient = (abs(data0[i - 1] + (data0[i] << 1) + data0[i + 1] - data2[i - 1] - (data2[i] << 1) - data2[i + 1])
				+ abs(data0[i - 1] + (data1[i - 1] << 1) + data2[i - 1] - data0[i + 1] - (data1[i + 1] << 1) - data2[i + 1]));

			if (tempGradient > 200)
			{
				countup++;
			}
			else if (tempGradient > 10)
				countdown++;
		}
	}

	if (countdown < 5 || countup < 5) //|| countdown / countup > 15)
	{
		return 0;
	}
	else
		return countup / countdown;
}

bool Detection:: detectFaces(Mat img, vector<Rect> *detectedFaces)
{
	int faceNum = faceDetectDrawByHaar(img);

	if (faceNum != 0)
	{
		float facesscore = -1.f;
		Mat faceImg;
		for (vector<Rect>::iterator it = faces.begin();it!=faces.end();it++)
		{ 
			img(*it).copyTo(faceImg);
			/*if (ImgisClear(faceImg) == 1)
				(*detectedFaces).push_back(*it);*/
			if (ImgisClear(faceImg) > facesscore){
				facesscore = ImgisClear(faceImg);
				face = *it;
			}
		}
		(*detectedFaces).push_back(face);
	}
	faces.clear(); 
	return (*detectedFaces).empty();
}
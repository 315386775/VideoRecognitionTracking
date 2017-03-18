#pragma once

#include <iostream>
#include <stdio.h>
#include "opencv2/opencv.hpp"
#include "../allHeaders.h"




class Detection
{
public:
	
	bool detectFaces(cv::Mat img, std::vector<cv::Rect> *detectedFaces);
	
	float ImgisClear(cv::Mat faceImg);

	void LoadCascade(std::string haarPath, bool enableEyeDetector);

private:
	cv::Rect face;
	std::vector<cv::Rect> faces;
	int faceDetectDrawByHaar(cv::Mat & img);	
	cv::CascadeClassifier face_cascade;
	cv::CascadeClassifier eyes_cascade;
};

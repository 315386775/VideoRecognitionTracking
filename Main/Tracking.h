#pragma once
#include "Tracking\Tracker.h"
#include"Tracking\kcftracker.hpp"
#include"Tracking\Config.h"
#include "allHeaders.h"


class Tracking{

public:
	Tracking();

	FrameHandleWay trackFaces(cv::Mat frame, TrackState *target,  cv::Mat &result);

private:
	
	Tracker struck_tracker;
	KCFTracker kcf_tracker;
	cv::Rect face_box;

	
	float nscore;      //���õ÷֣��ж�ѡ��ʽ

	cv::Rect floatRectToCvRect(const FloatRect &r);


};
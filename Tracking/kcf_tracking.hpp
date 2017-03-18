#pragma once


#include <opencv2/opencv.hpp>
#include <string>

class kcf_Tracker
{
public:
	//void tracking_process(cv::Rect box_a, cv::Mat kcf_frame, bool INITFRAME);
    kcf_Tracker()  {}
    virtual  ~kcf_Tracker() { }

	cv::Rect init(const cv::Rect &roi, cv::Mat image) ;
    cv::Rect  update( cv::Mat image);


protected:
    cv::Rect_<float> _roi;
};




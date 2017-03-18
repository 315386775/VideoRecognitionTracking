#pragma once
#include "stdafx.h"
#include "Tracking.h"
#include <fstream>

using namespace std;
using namespace cv;

Tracking::Tracking(){
	nscore = 0.f;
}


/*
程序功能：视频中的目标跟踪，和检测相比能够有效提速
 输入：当前帧图像，单个待跟踪目标信息，保存画框结果图像
 输出：目标位置，在图像中标记跟踪的帧数
*/
FrameHandleWay Tracking::trackFaces(Mat frame, TrackState *target, Mat &result)
{

	if (target->framecount % 4 == 0 || target->framecount < 15) //跟踪次数是4的倍数，或者小于15，调用struct跟踪
	{
		//初始化struck跟踪器
		if (target->framecount == 0)
		{
			struck_tracker.m_bb = FloatRect(target->boundingBox.x, target->boundingBox.y, target->boundingBox.width, target->boundingBox.height);  //根据bb设置要跟踪的目标
			struck_tracker.Reset();
			struck_tracker.Initialise(frame, struck_tracker.m_bb);
		}

		if (struck_tracker.IsInitialised()) //调用struck跟踪 ,并更新KCF
		{
			struck_tracker.Track(frame, struck_tracker.m_bb);
			face_box = floatRectToCvRect(struck_tracker.m_bb);
			face_box = kcf_tracker.init(face_box, frame);
			target->boundingBox = floatRectToCvRect(struck_tracker.m_bb);
		}

	}
	else            //否则调用KCF
	{
		face_box = kcf_tracker.update(frame, face_box);
		struck_tracker.m_bb = FloatRect(float(face_box.x), float(face_box.y), float(face_box.width), float(face_box.height));
		target->boundingBox = face_box;
	}

	//添加跟踪框和文本信息
	char temp[100];
	sprintf(temp, "track: %d", target->framecount);
	rectangle(result, Point(target->boundingBox.x, target->boundingBox.y),
		Point(target->boundingBox.x + target->boundingBox.width, target->boundingBox.y + target->boundingBox.height),
		Scalar(0, 255, 255), 5, 8);// 画黄框
	putText(result, temp, Point(target->boundingBox.x + target->boundingBox.width, target->boundingBox.y + target->boundingBox.height), FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255));

	target->framecount++;
	nscore = struck_tracker.Bestscore_;
	
	cout << "nscore=" << nscore << endl;
	
	/*if (target->framecount % 50==0)
	{
		return PerformRecognize;
	}*/
	if (nscore < 0.4) //跳出跟踪
	{
		
		return PerformDetect;
	}
	else
		return PerformTrack;

}

cv::Rect Tracking::floatRectToCvRect(const FloatRect &r)
{
	return cv::Rect(r.XMin(), r.YMin(), r.Width(), r.Height());
}
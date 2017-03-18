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
�����ܣ���Ƶ�е�Ŀ����٣��ͼ������ܹ���Ч����
 ���룺��ǰ֡ͼ�񣬵���������Ŀ����Ϣ�����滭����ͼ��
 �����Ŀ��λ�ã���ͼ���б�Ǹ��ٵ�֡��
*/
FrameHandleWay Tracking::trackFaces(Mat frame, TrackState *target, Mat &result)
{

	if (target->framecount % 4 == 0 || target->framecount < 15) //���ٴ�����4�ı���������С��15������struct����
	{
		//��ʼ��struck������
		if (target->framecount == 0)
		{
			struck_tracker.m_bb = FloatRect(target->boundingBox.x, target->boundingBox.y, target->boundingBox.width, target->boundingBox.height);  //����bb����Ҫ���ٵ�Ŀ��
			struck_tracker.Reset();
			struck_tracker.Initialise(frame, struck_tracker.m_bb);
		}

		if (struck_tracker.IsInitialised()) //����struck���� ,������KCF
		{
			struck_tracker.Track(frame, struck_tracker.m_bb);
			face_box = floatRectToCvRect(struck_tracker.m_bb);
			face_box = kcf_tracker.init(face_box, frame);
			target->boundingBox = floatRectToCvRect(struck_tracker.m_bb);
		}

	}
	else            //�������KCF
	{
		face_box = kcf_tracker.update(frame, face_box);
		struck_tracker.m_bb = FloatRect(float(face_box.x), float(face_box.y), float(face_box.width), float(face_box.height));
		target->boundingBox = face_box;
	}

	//��Ӹ��ٿ���ı���Ϣ
	char temp[100];
	sprintf(temp, "track: %d", target->framecount);
	rectangle(result, Point(target->boundingBox.x, target->boundingBox.y),
		Point(target->boundingBox.x + target->boundingBox.width, target->boundingBox.y + target->boundingBox.height),
		Scalar(0, 255, 255), 5, 8);// ���ƿ�
	putText(result, temp, Point(target->boundingBox.x + target->boundingBox.width, target->boundingBox.y + target->boundingBox.height), FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255));

	target->framecount++;
	nscore = struck_tracker.Bestscore_;
	
	cout << "nscore=" << nscore << endl;
	
	/*if (target->framecount % 50==0)
	{
		return PerformRecognize;
	}*/
	if (nscore < 0.4) //��������
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
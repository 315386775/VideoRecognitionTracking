#include "../stdafx.h"
#include "RawFeatures.h"
#include "Config.h"
#include "Sample.h"
#include "Rect.h"

#include <iostream>

using namespace Eigen;
using namespace cv;

static const int kPatchSize = 16;

RawFeatures::RawFeatures(const Config& conf) :
	m_patchImage(kPatchSize, kPatchSize, CV_8UC1)
{
	SetCount(kPatchSize*kPatchSize);
}

void RawFeatures::UpdateFeatureVector(const Sample& s)
{
	IntRect rect = s.GetROI(); // note this truncates to integers
	cv::Rect roi(rect.XMin(), rect.YMin(), rect.Width(), rect.Height());
	cv::resize(s.GetImage().GetImage(0)(roi), m_patchImage, m_patchImage.size());
	//equalizeHist(m_patchImage, m_patchImage);
	
	int ind = 0;
	for (int i = 0; i < kPatchSize; ++i)
	{
		uchar* pixel = m_patchImage.ptr(i);
		for (int j = 0; j < kPatchSize; ++j, ++pixel, ++ind)
		{
			m_featVec[ind] = ((double)*pixel)/255;
		}
	}
}

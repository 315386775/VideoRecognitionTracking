#pragma once

#include <algorithm>
#include <vector>  

#include "caffe/caffe.hpp"
#include "caffe/util/io.hpp"
#include "caffe/blob.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include "boost/smart_ptr/shared_ptr.hpp"
#include "../allHeaders.h"

// Caffe's required library
#pragma comment(lib, "caffe.lib")

// enable namespace
//using namespace std;

using namespace boost;
using namespace caffe;
using std::string;

/* Pair (label, confidence) representing a prediction. */
typedef std::pair<string, float> Prediction;
#define CPU_ONLY		
class Classifier   
{
public:
	Classifier();
	Classifier(const string& model_file,
		const string& trained_file,
		const string& mean_file,
		const string& label_file);
	
	~Classifier();

	//string classFaces(Rect face, Mat frame, int *w, string name);
	int LoadModelFile(string caffePath);
	string recognizeFaces(cv::Rect targetBBox, cv::Mat frame, cv::Mat imgForShow);
	
	std::vector<Prediction> Classify(const cv::Mat& img, int N = 5);

private:
	void SetMean(const string& mean_file);

	int InitCaffeNet();

	std::vector<float> Predict(const cv::Mat& img);

	void WrapInputLayer(std::vector<cv::Mat>* input_channels);
	
	void Preprocess(const cv::Mat& img,
		std::vector<cv::Mat>* input_channels);

	string model_file_;
	string trained_file_;
	string mean_file_;
	string label_file_;
	boost::shared_ptr<Net<float> > net_;
	cv::Size input_geometry_;
	int num_channels_;
	cv::Mat mean_;
	std::vector<string> labels_;	
};




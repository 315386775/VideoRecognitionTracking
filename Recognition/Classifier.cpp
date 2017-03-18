#pragma once

#include "Classifier.h"
#include <iomanip>
#include "../head.h"
using namespace cv;

Classifier::~Classifier()
{
}

Classifier::Classifier()
{

}

int Classifier::LoadModelFile(string caffePath)
{
	model_file_ = caffePath + "test_full_test.prototxt";
	trained_file_ = caffePath + "test_full_iter_38000.caffemodel";
	mean_file_ = caffePath + "test_mean.binaryproto";
	label_file_ = caffePath + "synset_words.txt";

	if (InitCaffeNet())
		return 1;
}

int Classifier::InitCaffeNet()
{

#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
#else
	Caffe::set_mode(Caffe::GPU);
#endif

	/* Load the network. */
	net_.reset(new Net<float>(model_file_, TEST));
	net_->CopyTrainedLayersFrom(trained_file_);

	CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
	CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";

	Blob<float>* input_layer = net_->input_blobs()[0];
	int num_inputs = net_->num_inputs();
	int num_outputs = net_->num_outputs();


	num_channels_ = input_layer->channels();


	CHECK(num_channels_ == 3 || num_channels_ == 1) << "Input layer should have 1 or 3 channels.";
	input_geometry_ = cv::Size(input_layer->width(), input_layer->height());

	/* Load the binaryproto mean file. */
	SetMean(mean_file_);

	/* Load labels. */
	std::ifstream labels(label_file_.c_str());
	CHECK(labels) << "Unable to open labels file " << label_file_;
	string line;
	while (std::getline(labels, line))
		labels_.push_back(string(line));

	Blob<float>* output_layer = net_->output_blobs()[0];
	std::cout << output_layer->channels();
	CHECK_EQ(labels_.size(), output_layer->channels())
		<< "Number of labels is different from the output layer dimension.";
	return 1;
}

Classifier::Classifier(const string& model_file,
						const string& trained_file,
						const string& mean_file,
						const string& label_file)
{

	model_file_ = model_file;
	trained_file_ = trained_file;
	mean_file_ = mean_file;
	label_file_ = label_file;
	InitCaffeNet();
}


string Classifier::recognizeFaces( Rect targetBBox,Mat frame,Mat imgForShow)
{
	vector<Prediction> predictions = Classify(frame(targetBBox), 1);  //face recognition
	Prediction p = predictions[0];

	if (p.second >= 0.6)
	{
		putText(imgForShow, p.first, Point(targetBBox.x, targetBBox.y), FONT_HERSHEY_PLAIN, 2, Scalar(255, 0, 0));
		return p.first;
	}
	else
		return "None";

}

static bool PairCompare(const std::pair<float, int>& lhs,
	const std::pair<float, int>& rhs) 
{
	return lhs.first > rhs.first;
}

/* Return the indices of the top N values of vector v. */
static std::vector<int> Argmax(const std::vector<float>& v, int N) {
	std::vector<std::pair<float, int> > pairs;
	for (size_t i = 0; i < v.size(); ++i)
		pairs.push_back(std::make_pair(v[i], static_cast<int>(i)));
	std::partial_sort(pairs.begin(), pairs.begin() + N, pairs.end(), PairCompare);

	std::vector<int> result;
	for (int i = 0; i < N; ++i)
		result.push_back(pairs[i].second);
	return result;
}


/* Return the top N predictions. */
std::vector<Prediction> Classifier::Classify(const cv::Mat& img, int N) {
	std::vector<float> output = Predict(img);

	N = std::min<int>(labels_.size(), N);
	std::vector<int> maxN = Argmax(output, N);
	std::vector<Prediction> predictions;
	for (int i = 0; i < N; ++i) {
		int idx = maxN[i];
		predictions.push_back(std::make_pair(labels_[idx], output[idx]));
	}

	return predictions;
}


/* Load the mean file in binaryproto format. */
void Classifier::SetMean(const string& mean_file) {

	Blob<float> mean_blob;
	BlobProto blob_proto;
	float *mean_ptr;
	unsigned int num_pixel;

	bool succeed = ReadProtoFromBinaryFile(mean_file, &blob_proto);
	if (succeed)
	{
		mean_blob.FromProto(blob_proto);
		CHECK_EQ(mean_blob.channels(), num_channels_)
			<< "Number of channels of mean file doesn't match input layer.";


		num_pixel = mean_blob.count(); /* NCHW=1x3x256x256=196608 */
		//mean_ptr = (float *)mean_blob.cpu_data();
		mean_ptr = mean_blob.mutable_cpu_data();
		
		/* The format of the mean file is planar 32-bit float BGR or grayscale. */
		std::vector<cv::Mat> channels;
		for (int i = 0; i < num_channels_; ++i) 
		{
			/* Extract an individual channel. */
			cv::Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1, mean_ptr);
			//cv::Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1);
			//memcpy(channel.data, data, mean_blob.width()*mean_blob.height()*sizeof(float));
			channels.push_back(channel);

			//imshow("img", channel);
			//waitKey(0);

			mean_ptr += mean_blob.height() * mean_blob.width();
		}
		
		/* Merge the separate channels into a single image. */
		//cv::Mat mean(mean_blob.height(), mean_blob.width(), CV_32FC1);//;//
		cv::Mat mean;
		cv::merge(channels, mean);
		
		/* Compute the global mean pixel value and create a mean image
		* filled with this value. */
		cv::Scalar channel_mean = cv::mean(mean);//mean);//channels[0]
		mean_ = cv::Mat(input_geometry_, mean.type(), channel_mean);
		
		//imshow("img1", mean_);
		//waitKey(0);
	}


}

std::vector<float> Classifier::Predict(const cv::Mat& img) 
{
	Blob<float>* input_layer = net_->input_blobs()[0];
	input_layer->Reshape(1, num_channels_,input_geometry_.height, input_geometry_.width);
	/* Forward dimension change to all layers. */
	net_->Reshape();

	std::vector<cv::Mat> input_channels;
	WrapInputLayer(&input_channels);

	Preprocess(img, &input_channels);

	net_->Forward(0);

	/* Copy the output layer to a std::vector */
	Blob<float>* output_layer = net_->output_blobs()[0];
	const float* begin = output_layer->cpu_data();
	const float* end = begin + output_layer->channels();
	return std::vector<float>(begin, end);
}

/* Wrap the input layer of the network in separate cv::Mat faces
* (one per channel). This way we save one memcpy operation and we
* don't need to rely on cudaMemcpy2D. The last preprocessing
* operation will write the separate channels directly to the input
* layer. */
void Classifier::WrapInputLayer(std::vector<cv::Mat>* input_channels) {
	Blob<float>* input_layer = net_->input_blobs()[0];

	int width = input_layer->width();
	int height = input_layer->height();
	float* input_data = input_layer->mutable_cpu_data();
	for (int i = 0; i < input_layer->channels(); ++i) {
		cv::Mat channel(height, width, CV_32FC1, input_data);
		input_channels->push_back(channel);
		input_data += width * height;
	}
}

void Classifier::Preprocess(const cv::Mat& img,
	std::vector<cv::Mat>* input_channels) {
	/* Convert the input image to the input image format of the network. */
	cv::Mat sample;
	if (img.channels() == 3 && num_channels_ == 1)
		cv::cvtColor(img, sample, cv::COLOR_BGR2GRAY);
	else if (img.channels() == 4 && num_channels_ == 1)
		cv::cvtColor(img, sample, cv::COLOR_BGRA2GRAY);
	else if (img.channels() == 4 && num_channels_ == 3)
		cv::cvtColor(img, sample, cv::COLOR_BGRA2BGR);
	else if (img.channels() == 1 && num_channels_ == 3)
		cv::cvtColor(img, sample, cv::COLOR_GRAY2BGR);
	else
		sample = img;

	cv::Mat sample_resized;
	if (sample.size() != input_geometry_)
		cv::resize(sample, sample_resized, input_geometry_);
	else
		sample_resized = sample;

	cv::Mat sample_float;
	if (num_channels_ == 3)
		sample_resized.convertTo(sample_float, CV_32FC3);
	else
		sample_resized.convertTo(sample_float, CV_32FC1);

	//imshow("img1", sample_float);
	//waitKey(0);

	cv::Mat sample_normalized;

	cv::subtract(sample_float, mean_, sample_normalized);

	/* This operation will write the separate BGR planes directly to the
	* input layer of the network because it is wrapped by the cv::Mat
	* faces in input_channels. */
	cv::split(sample_normalized, *input_channels);

	CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
		== net_->input_blobs()[0]->cpu_data())
		<< "Input channels are not wrapping the input layer of the network.";
}




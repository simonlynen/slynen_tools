/*
 * feature_provider.h
 *
 *  Created on: 26 Aug 2011
 *      Author: slynen
 */

#ifndef FEATURE_PROVIDER_H_
#define FEATURE_PROVIDER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <opencv2/opencv.hpp>
#include <brisk/brisk.h>
#include <boost/shared_ptr.hpp>
#include <slynen_tools/m128i_fio.h>
#include <slynen_tools/STLhelpers.h>
#include <slynen_tools/filetypes.h>
#include <queue>
#include <slynen_tools/concurrent_queue.h>
#include <boost/thread.hpp>

void providedetectorextractor(	cv::Ptr<cv::BriskFeatureDetector>& detector, cv::Ptr<cv::BriskDescriptorExtractor>& descriptorExtractor, std::string& BRISKpattern , int threshold, int octaves, bool rotationInvariant=true, bool scaleInvariant=true){
	detector = new cv::BriskFeatureDetector(threshold, octaves);
	descriptorExtractor = new cv::BriskDescriptorExtractor(BRISKpattern, rotationInvariant, scaleInvariant);
}

void providedetectorextractor(	boost::shared_ptr<cv::BriskFeatureDetector>& detector, boost::shared_ptr<cv::BriskDescriptorExtractor>& descriptorExtractor, std::string& BRISKpattern , int threshold, int octaves, bool rotationInvariant=true, bool scaleInvariant=true){
	detector.reset(new cv::BriskFeatureDetector(threshold, octaves));
	descriptorExtractor.reset(new cv::BriskDescriptorExtractor(BRISKpattern, rotationInvariant, scaleInvariant));
}

template<typename detector_T, typename extractor_T>
__inline__ void extract_features(std::vector<cv::KeyPoint>& keypoints,
		cv::Mat& descriptors, const cv::Mat& imgRGB1,
		detector_T& detector,
		const extractor_T& descriptorExtractor, size_t minimumfeatures = 50, bool choosebestcornerscore = false, size_t requestedfeatures = 2500){
	// convert to grayscale
	cv::Mat imgGray1;
	if(imgRGB1.depth() != CV_8U){
		cv::cvtColor(imgRGB1, imgGray1, CV_BGR2GRAY);
	}else{
		imgGray1 = imgRGB1;
	}

	int oldthres = detector->threshold;

	// run AGAST detect in image with adaptive thres
	//	while(keypoints.size() < minimumfeatures && detector->threshold > 0){
	//		keypoints.clear();
	detector->detect(imgGray1,keypoints);
	//		detector->threshold -= 5; //adapt threshold
	//	}
	detector->threshold = oldthres; //reset

	if(choosebestcornerscore){
		sort(keypoints.begin(), keypoints.end(), boost::bind(&cv::KeyPoint::response, _1)>boost::bind(&cv::KeyPoint::response, _2));
		if(keypoints.size()>(size_t)requestedfeatures)
			keypoints.resize(requestedfeatures);
	}

	// get the descriptors
	descriptorExtractor->compute(imgGray1,keypoints,descriptors);
}

template<typename detector_T, typename extractor_T>
__inline__ int get_features(std::vector<cv::KeyPoint>& keypoints,
		cv::Mat& descriptors, const std::string fname1,
		detector_T& detector,
		const extractor_T& descriptorExtractor, size_t minimumfeatures = 50, bool choosebestcornerscore = false, int requestefeatures = 2000){

	cv::Mat imgRGB1 = cv::imread(fname1, 0);

	if(imgRGB1.empty() ){
		std::cout<<"Image load error!"<<std::endl;
		return -1;
	}

	extract_features(keypoints, descriptors, imgRGB1, detector, descriptorExtractor, minimumfeatures, choosebestcornerscore, requestefeatures);

	return 0;
}

#endif /* FEATURE_PROVIDER_H_ */

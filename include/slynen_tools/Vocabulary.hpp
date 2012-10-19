/*
 * Vocabulary.hpp
 *
 *  Created on: 4 Oct 2011
 *      Author: slynen
 */

#ifndef VOCABULARY_HPP_
#define VOCABULARY_HPP_

#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;

template<typename descriptor_T, class Cluster_T, int NIDENT, int N128S>
class Vocabulary {
private:
	Cluster_T  mroot;
	std::vector<Cluster_T*> vNodes;
	int minWords;
	int mmaxlevel;
	descriptor_T* mask;
public:
	Vocabulary(string& path, bool useVarianceMasking);
	virtual ~Vocabulary();
	int size(){return minWords;}
	template<typename Observation_T, typename Detector_T, typename Extractor_T>
	void quantize(const string& path, Observation_T& obs, const Detector_T& detector,
			const Extractor_T& descriptorExtractor);
	template<typename Observation_T, typename Detector_T, typename Extractor_T>
	void quantize(const cv::Mat& imgRGB, Observation_T& obs, const Detector_T& detector,
			const Extractor_T& descriptorExtractor);
	template<typename Observation_T>
	void quantize(const std::vector<descriptor_T*>& vPoints, Observation_T& obs);
	int quantize(const descriptor_T* point, uint32_t* error = NULL){
		//TODO this alloc is aweful
		typename Cluster_T::distance_type* bestdistances =  new typename Cluster_T::distance_type[mmaxlevel];
		for(int i = 0;i<mmaxlevel;++i) bestdistances[i] = std::numeric_limits<typename Cluster_T::distance_type>::max();
		int word = mroot.find_leaf(point,bestdistances, error);
		cout<<point<<" got quantized to "<<word<<endl;
		delete[] bestdistances;
		return word;
	}

	Cluster_T* getNode(int ID){return vNodes.at(ID);}

	int load_Node(Cluster_T& cl, std::ifstream& loader, bool useVarianceMasking, int& maxlevel);

	void compress(int thres);
};

#include "Vocabulary.cc"
#endif /* VOCABULARY_HPP_ */

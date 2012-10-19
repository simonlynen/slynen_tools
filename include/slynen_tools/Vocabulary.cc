/*
 * Vocabulary.cc
 *
 *  Created on: 4 Oct 2011
 *      Author: slynen
 */

#include <boost/foreach.hpp>
#include <sstream>
#include <fstream>
#include <slynen_tools/feature_provider.h>

template<typename descriptor_T, class Cluster_T, int NIDENT, int N128S>
Vocabulary<descriptor_T, Cluster_T, NIDENT, N128S>::Vocabulary(string& path, bool useVarianceMasking) {

	mask = (descriptor_T*)_mm_malloc(8*sizeof(descriptor_T),16);
	for(int i = 0;i<8;++i) mask[i] = _mm_set_epi8((1 << i), (1 << i), (1 << i), (1 << i),(1 << i), (1 << i), (1 << i), (1 << i),
			(1 << i), (1 << i), (1 << i), (1 << i),(1 << i), (1 << i), (1 << i), (1 << i));

	std::ifstream loader(path.c_str());
	mroot.setLocation(1);
	mroot.setParent(&mroot);
	mmaxlevel = 0;
	load_Node(mroot,loader, useVarianceMasking, mmaxlevel);
	int nextID = 0;
	minWords = mroot.assign_IDs(nextID, vNodes, &mroot); //parent of root is self
	cout<<"Loaded vocabulary from file with "<<minWords<<" leaves. highest ID assigned "<<nextID-1<<endl;
}

template<typename descriptor_T, class Cluster_T, int NIDENT, int N128S>
int Vocabulary<descriptor_T, Cluster_T, NIDENT, N128S>::load_Node(Cluster_T& cl, std::ifstream& loader, bool useVarianceMasking, int& maxlevel){

	assert(!loader.eof());
	int childcount = 0;
	loader.read((char*)(&childcount), sizeof(int));
	int numidentifiers = 0;
	assert(!loader.eof());
	loader.read((char*)(&numidentifiers), sizeof(int));
	if(numidentifiers!=NIDENT && numidentifiers!=0){ //root has zero idents
		cout<<"[ERROR] numidentifiers: "<<numidentifiers<<" NIDENT "<<NIDENT<<endl;
	}
	assert(numidentifiers==NIDENT || numidentifiers==0);

	cl.allocidentifiers(numidentifiers);

	int nodecount = 1;

	//load identifiers
	for(int i = 0;i<numidentifiers;++i){
		typename Cluster_T::template_T*& tmp = cl.identifier(i);
		load_descriptor(tmp, loader, N128S);
	}
	//load number of masked bits from variancemasking
	int nmaskedbits = 0;
	assert(!loader.eof());
	loader.read((char*)(&nmaskedbits), sizeof(int));
	//load mask for variance based masking
	if(useVarianceMasking){
		cl.setmaskall(); //allocates memory
		typename Cluster_T::template_T*& tmp = cl.getmask();
		load_descriptor(tmp, loader, N128S);
		cl.setnumberofbitsmasked(nmaskedbits); //load mask bits
	}else{
		cl.setmaskall(); //allocates memory
		typename Cluster_T::template_T*& tmp = cl.getmask(); //need to do this to move the reader forwards
		load_descriptor(tmp, loader, N128S);
		cl.setmaskall(); //and set mask back to all "on"
		cl.setnumberofbitsmasked(0);
	}

	for(int i = 0;i<childcount;++i){
		Cluster_T tmp(cl.level() + 1, cl.location() * 100 + i);
		Cluster_T& child = cl.addchild(tmp);
		if(tmp.level() > maxlevel) maxlevel = tmp.level();
		child.setParent(&cl);
		nodecount += load_Node(child, loader, useVarianceMasking, maxlevel); //recursively load other nodes
	}
	return nodecount;
}

template<typename descriptor_T, class Cluster_T, int NIDENT, int N128S>
Vocabulary<descriptor_T, Cluster_T, NIDENT, N128S>::~Vocabulary() {
	// TODO Auto-generated destructor stub
}

template<typename descriptor_T, class Cluster_T, int NIDENT, int N128S>
template<typename Observation_T, typename Detector_T, typename Extractor_T>
void Vocabulary<descriptor_T, Cluster_T, NIDENT, N128S>::quantize(const string& path, Observation_T& obs, const Detector_T& detector,
		const Extractor_T& descriptorExtractor){
	cv::Mat imgRGB1;

	// open the images
	imgRGB1 = cv::imread(path);

	if(imgRGB1.empty() ){
		std::cout<<"Image load error! file "<<path<<std::endl;
		return;
	}
	quantize(imgRGB1, obs, detector, descriptorExtractor);
}

template<typename descriptor_T, class Cluster_T, int NIDENT, int N128S>
template<typename Observation_T, typename Detector_T, typename Extractor_T>
void Vocabulary<descriptor_T, Cluster_T, NIDENT, N128S>::quantize(const cv::Mat& imgRGB, Observation_T& obs, const Detector_T& detector,
		const Extractor_T& descriptorExtractor){
	std::vector<cv::KeyPoint> keypoints;
	cv::Mat descriptors;
	extract_features(keypoints, descriptors, imgRGB, detector, descriptorExtractor);

	std::vector<descriptor_T*> vPoints;
	int ni128s = descriptors.cols/16;
	for(int i = 0;i<descriptors.rows;++i){
		__m128i* pt = (__m128i*)_mm_malloc (ni128s*sizeof(__m128i), 16);
		memcpy(pt,reinterpret_cast<__m128i*>(descriptors.row(i).data),ni128s*sizeof(__m128i));
		vPoints.push_back(pt);
	}

	quantize(vPoints, obs);

	for(size_t i = 0;i<vPoints.size();++i){
		_mm_free(vPoints.at(i));
		vPoints.at(i) = NULL;
	}
}

template<typename descriptor_T, class Cluster_T, int NIDENT, int N128S>
template<typename Observation_T>
void Vocabulary<descriptor_T, Cluster_T, NIDENT, N128S>::quantize(const std::vector<descriptor_T*>& vPoints, Observation_T& obs){
	if(static_cast<int>(obs.size())!=minWords){
		obs = Observation_T(minWords);
	}
	obs.clear(false);
	for(size_t i = 0;i<vPoints.size();++i){
		const descriptor_T* descriptor = vPoints.at(i);
		obs.set_bit(quantize(descriptor)); //const_cast<const descriptor_T*>
	}
}

template<typename descriptor_T, class Cluster_T, int NIDENT, int N128S>
void Vocabulary<descriptor_T, Cluster_T, NIDENT, N128S>::compress(int thres){
	cout<<"Vocabulary size before compression: "<<mroot.countleaves()<<endl;
	mroot.compress_leaves(thres);
	//TODO recompute all cluster idents
	cout<<"Vocabulary size after compression: "<<mroot.countleaves()<<endl;
}


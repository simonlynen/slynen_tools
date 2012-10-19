/*
 * BRISK_Feature.h
 *
 *  Created on: 20.01.2011
 *      Author: slynen
 */

#ifndef BRISK_FEATURE_H_
#define BRISK_FEATURE_H_
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include <stdio.h>
#include <iostream>
#include <slynen_tools/popcnt.h>

using namespace std;
namespace vt{
//slynen{
struct BRISKFeature{
	__m128i* _signature;
	u_int8_t _size; //how many 128i blocks form the signature
	typedef __m128i value_type;
	u_int8_t size() {
		return _size;
	}
	void zero(){
		for(int i = 0;i<_size;i++){
			*(_signature + i) = _mm_setzero_si128(); //zero
		}
	}
	void freesig(){
		free(_signature);
		_size=0;
	}
	BRISKFeature(){
		_size=0;
		_signature = NULL;
	}
	BRISKFeature(int size)
	{
		_size = size;
		_signature = NULL;
		if( _size )
		{
			int ret = posix_memalign((void **) &(_signature), 16,_size * sizeof(__m128i));
			if(ret!=0){
				std::cout<<"_memalign failed with:"<<ret<<" mysize: "<<_size<<" their size: "<<size<<std::endl;
				assert(ret==0);
			}
			zero();
		}
	};
	~BRISKFeature(){
		if(_size > 0 && _signature!=NULL)
			free(_signature);
	}
	BRISKFeature(const cv::Mat& data)
	{
		int iDescrlength = data.cols*sizeof(uchar)*8;
		int ii128packs=iDescrlength/128;
		_size = ii128packs;
		if( _size )
		{
			int ret = posix_memalign((void **) &(_signature), 16,_size * sizeof(__m128i));
			if(ret!=0){
				cout<<"_memalign failed with:"<<ret<<" mysize: "<<_size<<" their size: "<<ii128packs<<endl;
				assert(ret==0);
			}
			memcpy( _signature, reinterpret_cast<const __m128i*>(data.data), sizeof(__m128i)*ii128packs );
			//_mm_stream_si128(__m128i *p, __m128i a) //loads a to addr p. Does not pollute the caches so using this might be better.
			// __m128i _mm_loadu_si128 (__m128i *p);
		}
	};
	BRISKFeature(const BRISKFeature& feat){
		_size = feat._size;
		if(_size)
		{
			int ret = posix_memalign((void **) &(_signature), 16,_size * sizeof(__m128i));
			if(ret!=0){
				cout<<"_memalign failed with:"<<ret<<" mysize: "<<_size<<" their size: "<<feat._size<<endl;
				assert(ret==0);
			}
			memcpy( _signature, feat._signature, _size * sizeof(__m128i) );
		}
	}
	BRISKFeature& operator=(const BRISKFeature& feat){
		if(_size != feat._size){ //the sizes differ, this results from creating the target
			//of the assignment using the std. constructor which initialises it empty
			_size = feat._size;
			_signature = NULL;
			if( _size )
			{
				int ret = posix_memalign((void **) &(_signature), 16,_size * sizeof(__m128i));
				if(ret!=0){
					std::cout<<"_memalign failed with:"<<ret<<" mysize: "<<_size<<" their size: "<<feat._size<<std::endl;
					assert(ret==0); //throw exc
				}
			}
		}
		if( _size )
		{
			memcpy( _signature, feat._signature, _size * sizeof(__m128i) );
		}
		return *this;
	}
	BRISKFeature& operator=(const cv::Mat& data){
		int iDescrlength = data.cols*sizeof(uchar)*8;
		int ii128packs=iDescrlength/128;
		assert(_size == ii128packs);
		memcpy( _signature, reinterpret_cast<const __m128i*>(data.data), sizeof(__m128i)*ii128packs );
		return *this;
	}

	bool operator==(const BRISKFeature& m){
		assert(m._size == _size);
		return !(popcnt::popcntOfXored(_signature,m._signature,_size)); //any bits different?!
	};

};
}
#endif /* BRISK_FEATURE_H_ */

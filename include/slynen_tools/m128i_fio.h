/*
 * m128i_fio.h
 *
 *  Created on: 23 Aug 2011
 *      Author: slynen
 */

#ifndef M128I_FIO_H_
#define M128I_FIO_H_

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <slynen_tools/popcnt.h>
#include <slynen_tools/filetypes.h>
#include <slynen_tools/m128i_tools.h>

using namespace std;

template <typename descriptor_T>
void save_descriptor (std::ofstream& saver,	descriptor_T* it, int nWords){
	descriptor_T* ptr = it;
	for(int i = 0; i < nWords; ++i)	{
		saver.write((char*)(ptr), sizeof(descriptor_T));
		ptr++;
	}
}

template <template<typename, typename> class descriptor_C_T, template<typename> class descriptor_A_T, typename descriptor_T>
void save_descriptors (std::ofstream& saver,	descriptor_C_T<descriptor_T, descriptor_A_T<descriptor_T> >& cDescriptors, int nWords){
	for(typename descriptor_C_T<descriptor_T, descriptor_A_T<descriptor_T> >::iterator it = cDescriptors.begin(); it != cDescriptors.end(); ++it) {
		save_descriptor(saver, (*it), nWords);
	}
	saver.close();
}


template <template<typename, typename> class descriptor_C_T, template<typename> class descriptor_A_T, typename descriptor_T>
void save_descriptors (string location,	descriptor_C_T<descriptor_T, descriptor_A_T<descriptor_T> >& cDescriptors, int nWords){
	std::ofstream saver(location.c_str());
	save_descriptors(saver, cDescriptors, nWords);
}

template <template<typename, typename> class cluster_C_T, template<typename> class cluster_A_T, typename cluster_T>
void save_clusters (string location,	cluster_C_T<cluster_T, cluster_A_T<cluster_T> >& cClusters, int nWords){

	std::ofstream saver(location.c_str());
	for(typename cluster_C_T<cluster_T, cluster_A_T<cluster_T> >::iterator it = cClusters.begin(); it != cClusters.end(); ++it) {
		for(int i = 0; i < nWords; ++i)
		{
			const popcnt::UCHAR_ALIAS* ptrcenter = reinterpret_cast<const popcnt::UCHAR_ALIAS*>(&((it->center)[i]));
			for(int j = 0;j<16;++j){
				saver.put((popcnt::UCHAR_ALIAS)ptrcenter[j]);
			}
		}
		for(int i = 0; i < nWords; ++i)
		{
			const popcnt::UCHAR_ALIAS* ptrmask = reinterpret_cast<const popcnt::UCHAR_ALIAS*>(&((it->mask)[i]));
			for(int j = 0;j<16;++j){
				saver.put((popcnt::UCHAR_ALIAS)ptrmask[j]);
			}
		}
	}
	saver.close();
}

template <typename descriptor_T>
void load_descriptor(descriptor_T*& temp, ifstream& loader, int nWords){
	if(temp){
		_mm_free(temp);
	}
	temp = (descriptor_T*)_mm_malloc(nWords*sizeof(descriptor_T),16);
	descriptor_T* ptr = temp;
	for(int i = 0; i < nWords; i++) {
		loader.read((char*)(ptr), sizeof(descriptor_T));
		ptr++;
	}
}

template <template<typename, typename> class descriptor_C_T, template<typename> class descriptor_A_T, typename descriptor_T>
bool load_descriptors (std::string location,	descriptor_C_T<descriptor_T, descriptor_A_T<descriptor_T> >& cDescriptors, int nWords)
{
	ifstream loader(location.c_str());
	if(!loader.is_open()) return false;

	cDescriptors.clear();

	while(!loader.eof()) {
		descriptor_T temp = NULL;
		load_descriptor(temp, loader, nWords);
		cDescriptors.push_back(temp);
	}
	cDescriptors.pop_back();
	loader.close();
	return true;
}

template <template<typename, typename> class cluster_C_T, template<typename> class cluster_A_T, typename cluster_T>
bool load_clusters (std::string location,	cluster_C_T<cluster_T, cluster_A_T<cluster_T> >& cClusters, int nWords)
{
	ifstream loader(location.c_str());
	if(!loader.is_open()) return false;

	//prevents the destructor from freeing memory that belongs to another cluster
	cClusters.resize(4000);

	char value;

	loader.get(value);
	int idx = 0;
	while(!loader.eof()) {
		cluster_T& cl = cClusters.at(idx);
		cl.center = NULL;
		cl.mask = NULL;
		cl.variance = NULL;
		cl.center = (__m128i*)_mm_malloc(nWords*sizeof(__m128i),16);
		cl.mask = (__m128i*)_mm_malloc(nWords*sizeof(__m128i),16);
		popcnt::UCHAR_ALIAS* ptrcenter = reinterpret_cast<popcnt::UCHAR_ALIAS*>(cl.center);
		popcnt::UCHAR_ALIAS* ptrmask = reinterpret_cast<popcnt::UCHAR_ALIAS*>(cl.mask);
		for(int i = 0; i < nWords; i++) {
			for(int j = 0;j<16;++j){
				*ptrcenter++ = (popcnt::UCHAR_ALIAS)value;
				loader.get(value);
			}
		}
		for(int i = 0; i < nWords; i++) {
			for(int j = 0;j<16;++j){
				*ptrmask++ = (popcnt::UCHAR_ALIAS)value;
				loader.get(value);
			}
		}
		loader.get(value);//throw endl away
		++idx;
	}
	loader.close();
	cClusters.resize(idx-1);
	return true;
}


#endif /* M128I_FIO_H_ */

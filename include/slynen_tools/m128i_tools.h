/*
 * m128i_tools.h
 *
 *  Created on: 22 Aug 2011
 *      Author: slynen
 */


#ifndef M128I_TOOLS_H_
#define M128I_TOOLS_H_

#include <emmintrin.h>
#include <tmmintrin.h>
#include <vector>
#include <stdint.h>
#include <slynen_tools/filetypes.h>
#include <slynen_tools/popcnt.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iomanip>
#include <cmath>
#include <slynen_tools/sse_mathfun.h>

using namespace std;

namespace m128i_tools{

__inline__ bool bitset(const __m128i* const data, int bit){
	return reinterpret_cast<const popcnt::UINT32_ALIAS*>(data)[bit >> 5]&(1 << (bit&31));
}

__inline__ void plotDescriptor(const __m128i* const centroid, int numWords, int spacedist = -1){
	int iDescrlength = numWords*128;
	for(int i = 0;i<iDescrlength;++i){
		if(spacedist!=-1)
			if((i%spacedist)==0)
				cout<<" ";

		cout<<bitset(centroid,i);
	}
	cout<<endl;
}

__inline__ void print128(__m128i* data, int numWords, bool lf = true){
	if(!data) return;
	for(int i = 0;i<numWords*128;++i){
		cout<<(int)bitset(data, i)<<" ";
	}
	if(lf)
		cout<<endl;
}

__inline__ void print128(__m128* data, int numWords, bool lf = true){
	cout<<setprecision(4);
	if(!data) return;
	for(int i = 0;i<numWords;++i){
		const popcnt::FLOAT_ALIAS* ptr = reinterpret_cast<const popcnt::FLOAT_ALIAS*>(&data[i]);
		for(int j = 0;j<4;++j){
			cout<<(float)ptr[j]<<" ";
		}
	}
	if(lf)
		cout<<endl;
}

__inline__ void bitcountssse(const std::vector<const __m128i*>& vPoints, size_t ndescriptorbytes, int* bitcount/*, int startblock = 0*/){

	int nblocks = ndescriptorbytes / 16; //number of 128 bit blocks

	memset(bitcount, 0, ndescriptorbytes * 8 * sizeof(int));

	int nbits = 1; 	//number of bits to represent the max count
	int npts = vPoints.size();
	int npower2 = 1;
	while(npower2 < npts){
		npower2*=2;
		++nbits;
	}

	__m128i** current = new __m128i*[nbits];
	__m128i** overflow = new __m128i*[nbits+1];
	for(int i = 0;i<nbits;++i){
		current[i] = (__m128i*)_mm_malloc(nblocks*sizeof(__m128i), 16);
		memset(current[i], 0, nblocks*sizeof(__m128i));
		overflow[i] =(__m128i*) _mm_malloc(nblocks*sizeof(__m128i), 16);
		memset(overflow[i], 0, nblocks*sizeof(__m128i));
	}
	overflow[nbits] =(__m128i*) _mm_malloc(nblocks*sizeof(__m128i), 16);
	memset(overflow[nbits], 0, nblocks*sizeof(__m128i));

	int current_power2 = 0;
	int power2res = 1;
	for(int i = 0;i< npts;++i){
		if(i+1 >= power2res){
			++current_power2;
			power2res *= 2;
		}
		memcpy(overflow[0], vPoints.at(i), ndescriptorbytes);//start: cpy descriptor to overflow 0
		for(int p2 = 0;p2<current_power2;++p2){
			for(int block = 0/*startblock*/;block < nblocks;++block){
				overflow[p2+1][block] = _mm_and_si128(current[p2][block], overflow[p2][block]);
				current[p2][block] = _mm_xor_si128(current[p2][block], overflow[p2][block]);
			}
		}
	}

	int ndescriptorbits = nblocks * 128;
	for(int i = 0/*startblock*128*/;i<ndescriptorbits;++i){
		int p2res = 1;
		int& btcnt = bitcount[i];
		for(int p2 = 0;p2<nbits;++p2){
			if(bitset(current[p2], i))
				btcnt += p2res;
			p2res*=2;
		}
	}

	for(int i = 0;i<nbits;++i){
		_mm_free(current[i]);
		_mm_free(overflow[i]);
	}
	_mm_free(overflow[nbits]);
	delete [] current;
	delete [] overflow;
}


void bitcounts(const std::vector<const __m128i*>& vPoints, size_t ndescriptorbytes, int* bitcount){

	int iDescrlength = ndescriptorbytes * 8;

	__m128i mask[8];
	//create masks, could be actually done just once somewhere...but then cache misses decrease performance
	for(int i = 0;i<8;++i) mask[i] = _mm_set_epi8((1 << i), (1 << i), (1 << i), (1 << i),(1 << i), (1 << i), (1 << i), (1 << i),
			(1 << i), (1 << i), (1 << i), (1 << i),(1 << i), (1 << i), (1 << i), (1 << i));

	memset(bitcount,0,iDescrlength*sizeof(int));

	int nDescr = vPoints.size(); //number of descriptors
	//the following loops are of order: O(ndescr*npacks*32)
	for(int row = 0;row<nDescr;++row) { //goes through all descriptors
		for(size_t pack = 0;pack<ndescriptorbytes/16;++pack){ //goes through all packs of i128's
			const __m128i& _m1 = vPoints.at(row)[pack]; //get a reference to the i'th signature of the feature
			const register int pack128 = pack<<7;
			for(int i = 0;i<8;i++){ //checks 16 bits at once with sse
				__m128i xmm1 = _mm_and_si128(_m1, mask[i]); //masks the i'th bit of the 16 uint8s
				const popcnt::U_INT8T_ALIAS* ptr=reinterpret_cast<const popcnt::U_INT8T_ALIAS*>(&xmm1);
				if (ptr[0])		++bitcount[pack128+i+0]; 	//if bit i of the first i8 is set
				if (ptr[1])		++bitcount[pack128+i+8];
				if (ptr[2])		++bitcount[pack128+i+16];
				if (ptr[3])		++bitcount[pack128+i+24];
				if (ptr[4])		++bitcount[pack128+i+32];
				if (ptr[5])		++bitcount[pack128+i+40];
				if (ptr[6])		++bitcount[pack128+i+48];
				if (ptr[7])		++bitcount[pack128+i+56];
				if (ptr[8])		++bitcount[pack128+i+64];
				if (ptr[9])		++bitcount[pack128+i+72];
				if (ptr[10])	++bitcount[pack128+i+80];
				if (ptr[11])	++bitcount[pack128+i+88];
				if (ptr[12])	++bitcount[pack128+i+96];
				if (ptr[13])	++bitcount[pack128+i+104];
				if (ptr[14])	++bitcount[pack128+i+112];
				if (ptr[15])	++bitcount[pack128+i+120];
			}
		}
	}

}


void bitMarginal(const std::vector<const __m128i*>& vPoints, int descriptorbytes, std::vector<float>& marginals, bool smooth = true){


	int* bitcount = new int[descriptorbytes*8]; //that variable holds the number of bits set

	bitcountssse(vPoints, descriptorbytes, bitcount);

	marginals.clear();
	marginals.reserve(descriptorbytes*8);
	float fnDescr = (float)vPoints.size();
	int totalcnt = 0;
	for(int i = 0;i<descriptorbytes*8;++i){
		if(smooth){
			marginals.push_back((0.9998 * bitcount[i])/fnDescr + 0.0001);
		}else{
			marginals.push_back(bitcount[i]/fnDescr);
		}
		totalcnt+=bitcount[i];
	}
	delete[] bitcount;
//	std::cout<<"Total bits set "<<totalcnt<<std::endl;
}

void bitMarginal(const std::vector<const __m128i*>& vPoints, int descriptorbytes, std::vector<double>& marginals, bool smooth = true){


	int* bitcount = new int[descriptorbytes*8]; //that variable holds the number of bits set

	bitcountssse(vPoints, descriptorbytes, bitcount);

	marginals.clear();
	marginals.reserve(descriptorbytes*8);
	double fnDescr = (double)vPoints.size();
	int totalcnt = 0;
	for(int i = 0;i<descriptorbytes*8;++i){
		if(smooth){
			marginals.push_back((0.9998 * bitcount[i])/fnDescr + 0.0001);
		}else{
			marginals.push_back(bitcount[i]/fnDescr);
		}
		totalcnt+=bitcount[i];
	}
	delete[] bitcount;
//	std::cout<<"Total bits set "<<totalcnt<<std::endl;
}

void plot_mm128(__m128 value){
	const popcnt::FLOAT_ALIAS* asd = reinterpret_cast<popcnt::FLOAT_ALIAS*>(&value);

	cout<<": ";
	for(int i = 0;i<4;++i){
		cout<<asd[i]<<" ";
	}
	cout<<endl;
}



float totalEntropy(const std::vector<const __m128i*>& vPoints, size_t ndescriptorbytes, float* fmarginals = NULL , float* fentropies = NULL){

	register int nDescr = vPoints.size(); //number of descriptors

	if(nDescr==0){
		return -1; //this split doesnt make sense
	}

	if(nDescr==1){
		if(fmarginals){
			for(int i = 0;i < (int)ndescriptorbytes*8;++i){
				fmarginals[i] = bitset(vPoints.at(0), i)? 1.0f:0.0f;
			}
		}
		if(fentropies){
			for(int i = 0;i < (int)ndescriptorbytes*8;++i){
				fentropies[i] = 0.0f;
			}
		}
		return 0;
	}

	size_t iDescrlength = ndescriptorbytes * 8;

	int* bitcount = new int[iDescrlength]; //that variable holds the number of bits set

	bitcountssse(vPoints, ndescriptorbytes, bitcount);

	__m128 fnDescr = _mm_set1_ps((float)nDescr);
	const static __m128 one = _mm_set1_ps((float)1);
	const static __m128 log_2 = _mm_set1_ps((float)log(2.0));
	//	plot_mm128(fnDescr);
	//	plot_mm128(log_2);

	__m128 entr_sum = _mm_setzero_ps();

	float* marginalptr = fmarginals;
	float* fentropieptr = fentropies;
	for(size_t i = 0;i<iDescrlength/4;++i){

		__m128 marginal = _mm_div_ps(_mm_set_ps(bitcount[i*4+3], bitcount[i*4+2], bitcount[i*4+1], bitcount[i*4+0]), fnDescr); //p
		__m128 marginal_inv = _mm_sub_ps(one, marginal); 							//1-p

		__m128 marginal_mask = _mm_cmpneq_ps(_mm_setzero_ps(), marginal); 			//mask for zero
		__m128 invmarginal_mask = _mm_cmpneq_ps(_mm_setzero_ps(), marginal_inv); 	//mask for one

		__m128 entropy = -_mm_mul_ps(marginal, _mm_div_ps(log_ps(marginal), log_2)) - _mm_mul_ps(marginal_inv, _mm_div_ps(log_ps(marginal_inv), log_2));

		//mask nans
		entropy = _mm_and_ps(marginal_mask, entropy); //apply mask for zero
		entropy = _mm_and_ps(invmarginal_mask, entropy); //apply mask for one

		//sum
		entr_sum = _mm_add_ps(entropy, entr_sum);

		//		cout<<"marginalsinner=[";
		if(marginalptr){
			const popcnt::FLOAT_ALIAS* vals = reinterpret_cast<popcnt::FLOAT_ALIAS*>(&marginal);
			for(int j = 0;j<4;++j){
				*marginalptr = vals[j];
				//				cout<<vals[j]<<" "; cout.flush();
				++marginalptr;
			}
		}
		if(fentropieptr){
			const popcnt::FLOAT_ALIAS* vals = reinterpret_cast<popcnt::FLOAT_ALIAS*>(&entropy);
			for(int j = 0;j<4;++j){
				*fentropieptr = vals[j];
				++fentropieptr;
			}
		}
		//		cout<<"];"<<endl;
		//		plot_mm128(marginal * _mm_div_ps(log_ps(marginal), log_2));
	}


	const popcnt::FLOAT_ALIAS* values = reinterpret_cast<popcnt::FLOAT_ALIAS*>(&entr_sum);
	float sum = 0;
	for(int i = 0;i<4;++i){
		sum += values[i];
	}
	delete[] bitcount;

	return sum;
}

__inline__ void update_bitcount(__m128i* descriptor, float* pbitcount, __m128i* const & mask, int nWords){
	for(int pack = 0;pack<nWords;++pack){ 				//goes through all i128's
		__m128i& _m1 = descriptor[pack]; 		//get a reference to the i'th signature of the feature
		const register int pack128 = pack<<7;
		for(int i = 0;i<8;i++){ //checks 16 bits at once with sse
			__m128i xmm1 = _mm_and_si128(_m1, mask[i]); //masks the i'th bit of the 16 uint8s
			const popcnt::U_INT8T_ALIAS* ptr=reinterpret_cast<const popcnt::U_INT8T_ALIAS*>(&xmm1);
			if (ptr[0])		++pbitcount[pack128+i+0]; 	//if bit i of the first i8 is set
			if (ptr[1])		++pbitcount[pack128+i+8];
			if (ptr[2])		++pbitcount[pack128+i+16];
			if (ptr[3])		++pbitcount[pack128+i+24];
			if (ptr[4])		++pbitcount[pack128+i+32];
			if (ptr[5])		++pbitcount[pack128+i+40];
			if (ptr[6])		++pbitcount[pack128+i+48];
			if (ptr[7])		++pbitcount[pack128+i+56];
			if (ptr[8])		++pbitcount[pack128+i+64];
			if (ptr[9])		++pbitcount[pack128+i+72];
			if (ptr[10])	++pbitcount[pack128+i+80];
			if (ptr[11])	++pbitcount[pack128+i+88];
			if (ptr[12])	++pbitcount[pack128+i+96];
			if (ptr[13])	++pbitcount[pack128+i+104];
			if (ptr[14])	++pbitcount[pack128+i+112];
			if (ptr[15])	++pbitcount[pack128+i+120];
		}
	}
}

template <typename descriptor_T>
void descriptor_variance(std::vector<descriptor_T>& vdescriptor, std::vector<double>& vVariance, int nWords){

	double* pbitcount = new double[nWords * 128];
	memset(pbitcount, 0, nWords*128*sizeof(double));

	__m128i mask[8];
	for(int i = 0;i<8;++i) mask[i] = _mm_set_epi8((1 << i), (1 << i), (1 << i), (1 << i),(1 << i), (1 << i), (1 << i), (1 << i),
			(1 << i), (1 << i), (1 << i), (1 << i),(1 << i), (1 << i), (1 << i), (1 << i));

	for(size_t idxd = 0;idxd<vdescriptor.size();++idxd){
		for(int pack = 0;pack<nWords;++pack){ 				//goes through all i128's
			__m128i& _m1 = vdescriptor.at(idxd)[pack]; 		//get a reference to the i'th signature of the feature
			const register int pack128 = pack<<7;
			for(int i = 0;i<8;i++){ //checks 16 bits at once with sse
				__m128i xmm1 = _mm_and_si128(_m1, mask[i]); //masks the i'th bit of the 16 uint8s
				const popcnt::U_INT8T_ALIAS* ptr=reinterpret_cast<const popcnt::U_INT8T_ALIAS*>(&xmm1);
				if (ptr[0]) ++(pbitcount[pack128+i+0]); //if bit i of the ith i8 is set
				if (ptr[1]) ++(pbitcount[pack128+i+8]);
				if (ptr[2]) ++(pbitcount[pack128+i+16]);
				if (ptr[3]) ++(pbitcount[pack128+i+24]);
				if (ptr[4]) ++(pbitcount[pack128+i+32]);
				if (ptr[5]) ++(pbitcount[pack128+i+40]);
				if (ptr[6])	++(pbitcount[pack128+i+48]);
				if (ptr[7])	++(pbitcount[pack128+i+56]);
				if (ptr[8])	++(pbitcount[pack128+i+64]);
				if (ptr[9])	++(pbitcount[pack128+i+72]);
				if (ptr[10])++(pbitcount[pack128+i+80]);
				if (ptr[11])++(pbitcount[pack128+i+88]);
				if (ptr[12])++(pbitcount[pack128+i+96]);
				if (ptr[13])++(pbitcount[pack128+i+104]);
				if (ptr[14])++(pbitcount[pack128+i+112]);
				if (ptr[15])++(pbitcount[pack128+i+120]);
			}
		}
	}

	vVariance.clear();
	vVariance.resize(nWords * 128, 0.0);
	double denom;
	if (vdescriptor.size()>1)
		denom = (double)(vdescriptor.size()-1); //unbiased estimator
	else
		denom = (double)(vdescriptor.size()); //biased estimator

	double prob = 1.0/denom;

	for(int i = 0;i<nWords * 128;++i){
		double expected_val = pbitcount[i] / (double)vdescriptor.size();
		for(size_t idxd = 0;idxd<vdescriptor.size();++idxd){
			double dev = ((m128i_tools::bitset(vdescriptor.at(idxd), i)?1:0) - expected_val);
			vVariance[i] += dev*dev*prob;
		}
		//		cout<<"expected_val "<<expected_val<<" variance[i] "<<vVariance.at(i)<<endl;
	}
	delete[] pbitcount;
}

/**
 * \calcs the median of the binary descriptors
 */
__inline__ void bitVectorMedian(std::vector<__m128i*>& centroid, const std::vector<__m128i*>& descriptors, const std::vector<int>& clusterindices, int nWords)
{

	//plotDescriptor(centroid, numWords);

	int iDescrlength = nWords * 128;

	__m128i mask[8];
	//create masks, could be actually done just once somewhere...but then cache misses decrease performance
	for(int i = 0;i<8;++i) mask[i] = _mm_set_epi8((1 << i), (1 << i), (1 << i), (1 << i),(1 << i), (1 << i), (1 << i), (1 << i),
			(1 << i), (1 << i), (1 << i), (1 << i),(1 << i), (1 << i), (1 << i), (1 << i));

	int* aiCountAt = new int[iDescrlength]; //that variable holds the number of bits set

	//for (int i = 0;i<iDescrlength;i++){ aiCountAt[i] = 0; } //zero out
	memset(aiCountAt,0,iDescrlength*sizeof(int));

	int nDescr = clusterindices.size(); //number of descriptors
	//the following loops are of order: O(ndescr*npacks*32)
	for(int row = 0;row<nDescr;++row) { //goes through all descriptors
		for(int pack = 0;pack<nWords;++pack){ //goes through all packs of i128's
			__m128i& _m1 = descriptors.at(clusterindices.at(row))[pack]; //get a reference to the i'th signature of the feature
			const register int pack128 = pack<<7;
			for(int i = 0;i<8;i++){ //checks 16 bits at once with sse
				__m128i xmm1 = _mm_and_si128(_m1, mask[i]); //masks the i'th bit of the 16 uint8s
				const popcnt::U_INT8T_ALIAS* ptr=reinterpret_cast<const popcnt::U_INT8T_ALIAS*>(&xmm1);
				if (ptr[0])		++aiCountAt[pack128+i+0]; 	//if bit i of the first i8 is set
				if (ptr[1])		++aiCountAt[pack128+i+8];
				if (ptr[2])		++aiCountAt[pack128+i+16];
				if (ptr[3])		++aiCountAt[pack128+i+24];
				if (ptr[4])		++aiCountAt[pack128+i+32];
				if (ptr[5])		++aiCountAt[pack128+i+40];
				if (ptr[6])		++aiCountAt[pack128+i+48];
				if (ptr[7])		++aiCountAt[pack128+i+56];
				if (ptr[8])		++aiCountAt[pack128+i+64];
				if (ptr[9])		++aiCountAt[pack128+i+72];
				if (ptr[10])	++aiCountAt[pack128+i+80];
				if (ptr[11])	++aiCountAt[pack128+i+88];
				if (ptr[12])	++aiCountAt[pack128+i+96];
				if (ptr[13])	++aiCountAt[pack128+i+104];
				if (ptr[14])	++aiCountAt[pack128+i+112];
				if (ptr[15])	++aiCountAt[pack128+i+120];
			}
		}
	}

	//	for(size_t j = 0;j<centroid.size();++j){
	//		_mm_free (centroid.at(j));
	//	}
	//	centroid.clear();
	//	//zero out center
	//	for(int j = 0;j<nClusterSamples;++j){
	//		__m128i* ptr = (__m128i*)_mm_malloc(nWords*sizeof(__m128i),16);
	//		for(int i = 0;i<nWords;++i){
	//			ptr[i] = _mm_setzero_si128();
	//		}
	//		centroid.push_back(ptr);
	//	}
	//
	//	__m128i mask_bitset[nClusterSamples];
	//
	//	for (int i = 0;i<nWords;i++){ //assemble the new centroid
	//		for(int bit = 0;bit<8;++bit){
	//			for(int k = 0;k<nClusterSamples;++k)
	//				mask_bitset[k] = _mm_setzero_si128();
	//			for(int j = 0;j<16;++j){
	//				//approximate by setting bits in the descriptors of the centroid
	//				int percentage = 0.5 + nClusterSamples * (double)aiCountAt[i*128+j*8+bit]/nDescr; //0.5 rounds
	//				for(int k = 0;k<percentage;++k){
	//					popcnt::U_INT8T_ALIAS* ptr_mask=reinterpret_cast<popcnt::U_INT8T_ALIAS*>(&mask_bitset[k]);
	//					ptr_mask[15-j] = 1<<bit;
	//				}
	//			}
	//			for(int k = 0;k<nClusterSamples;++k)
	//				centroid.at(k)[i] = _mm_or_si128(centroid.at(k)[i],mask_bitset[k]);
	//		}
	//	}
	delete aiCountAt;
	//plotDescriptor(centroid, numWords);
}

double jaccard(__m128i* a, __m128i* b, int nWords){

	__m128i* tmp = (__m128i*)_mm_malloc(nWords*sizeof(__m128i),16);

	uint32_t M01 = 0;
	for(int i = 0;i<nWords;++i) tmp[i] = _mm_andnot_si128(a[i],b[i]);
	M01 = popcnt::ssse3_popcount(a,nWords);

	uint32_t M10 = 0;
	for(int i = 0;i<nWords;++i) tmp[i] = _mm_andnot_si128(b[i],a[i]);
	M10 = popcnt::ssse3_popcount(tmp,nWords);

	//	uint32_t M10_01 = 0;
	//	M10_01 = popcnt::ssse3_popcntofXORed(a,b,nWords);

	uint32_t M11 = 0;
	M11 = popcnt::ssse3_popcntofAND(a,b,nWords);

	for(int i = 0;i<nWords;++i) tmp[i] = _mm_or_si128(b[i],a[i]);

	//	uint32_t M00;
	//	M00 = nWords*128-popcnt::ssse3_popcount(tmp,nWords);

	_mm_free (tmp);
	return (double)(M10 + M01)/(double)(M10+M01+M11);
}

}
#endif /* M128I_TOOLS_H_ */

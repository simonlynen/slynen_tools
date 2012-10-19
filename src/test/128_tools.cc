/*
 * 128_tools.cc
 *
 *  Created on: Jan 6, 2012
 *      Author: slynen
 */


#include <gtest/gtest.h>
#include <slynen_tools/m128i_tools.h>
#include <emmintrin.h>
#include <tmmintrin.h>
#include <boost/foreach.hpp>

void createsomeData(std::vector<__m128i*>& vPoints, int howmany, int blocks){
	for(int i = 0;i<howmany;++i){
		__m128i* pt = new __m128i[blocks];
		int* ptr = reinterpret_cast<int*>(pt);
		for(int j = 0;j<4*blocks;++j){
			ptr[j] = rand() % std::numeric_limits<int>::max();
		}
		vPoints.push_back(pt);
	}
}


TEST(m128Tools, Marginals){
	int meas = 1000; //how many points should be tested
	int blocks = 4; //number of 128i blocks

	std::vector<__m128i*> vPoints;
	createsomeData(vPoints, meas, blocks);

	int nbits = blocks*sizeof(__m128i)*8;

	float* mfmarginals;
	mfmarginals = new float[nbits];
	float entropyall = m128i_tools::totalEntropy(vPoints, blocks*sizeof(__m128i), mfmarginals);

	for(int bit = 0;bit<nbits;++bit){
		int count = 0;
		BOOST_FOREACH(__m128i* pt, vPoints){
			if(m128i_tools::bitset(pt, bit)) ++count;
		}
		EXPECT_GE(count,0);
		EXPECT_LE(count,meas);
		EXPECT_LT(count/static_cast<float>(meas) - mfmarginals[bit],0.000001);
	}
}




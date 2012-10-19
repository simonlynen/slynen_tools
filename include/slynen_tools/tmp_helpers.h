/*
 * tmp_helpers.h
 *
 *  Created on: 19 Aug 2011
 *      Author: asl
 */

#ifndef TMP_HELPERS_H_
#define TMP_HELPERS_H_


/*
 * Faktorial
 */
template<unsigned n>
struct tmp_factorial{
	enum{value = n*tmp_factorial<n-1>::value};
};

template<>
struct tmp_factorial<0>{
	enum{value = 1};
};

#endif /* TMP_HELPERS_H_ */

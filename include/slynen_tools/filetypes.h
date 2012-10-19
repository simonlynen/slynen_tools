/*
 * filetypes.h
 *
 *  Created on: 23 Aug 2011
 *      Author: slynen
 */

#ifndef FILETYPES_H_
#define FILETYPES_H_
#include <emmintrin.h>
#include <tmmintrin.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>


typedef unsigned int __attribute__ ((__may_alias__)) UINT32_ALIAS;

typedef std::pair<std::vector<__m128i*>, std::vector<int> > m128i_cluster;


#endif /* FILETYPES_H_ */

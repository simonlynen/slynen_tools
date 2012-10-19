/*
 * tictoc.h
 *
 *  Created on: 5 Oct 2011
 *      Author: slynen
 */

#ifndef TICTOC_H_
#define TICTOC_H_

#include <opencv2/opencv.hpp>
#include <iostream>

static double tictoc_timestamp;

static void tic(){
	tictoc_timestamp = cvGetTickCount();
}

static void toc(){
	tictoc_timestamp = cvGetTickCount() - tictoc_timestamp;
	std::cout<<"took: "<<tictoc_timestamp/((double)cvGetTickFrequency()*1000.)<<" ms."<<std::endl;
}

#endif /* TICTOC_H_ */

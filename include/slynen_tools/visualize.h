/*
 * visualize.h
 *
 *  Created on: 8 Sep 2011
 *      Author: asl
 */

#ifndef VISUALIZE_H_
#define VISUALIZE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <opencv2/opencv.hpp>
#include <brisk/brisk.h>
#include <math.h>
#include <cmath>
#include <string.h>

#include <slynen_tools/m128i_tools.h>

#include <fstream>
#include <iostream>
#include <utility>
#include <algorithm>

#ifndef PI
#define PI 3.141592653589793238462643383279502884197169399
#endif

using namespace std;

__inline__ void visdescriptor(__m128i* descriptor, int nWords, IplImage*& img_in ){
	int sze_x = 23;
	int sze_y = 23;
	int scale = 2;
	IplImage* img  = cvCreateImage(cvSize(sze_x*scale,sze_y*scale),IPL_DEPTH_8U,1);
	int step       = img->widthStep/sizeof(uchar);
	uchar* data    = (uchar *)img->imageData;
	for(int i = 0;i<nWords*128;++i){
		int row = scale*(i/sze_y);
		int col = scale*(i%sze_x);
		for(int k = 0;k<scale;++k){
			for(int j = 0;j<scale;++j){
				data[(k+row)*step+col+j] = m128i_tools::bitset(descriptor,i)*250;
			}
		}
	}
	img_in = cvCreateImage(cvSize(sze_x*scale,sze_y*scale),IPL_DEPTH_8U,3);
	cvCvtColor( img, img_in, CV_GRAY2RGB );
	cvReleaseImage(&img);
}


__inline__ void visfeature(IplImage* ipl_img, cv::KeyPoint& kp, unsigned int* sizeList_, IplImage*& img2){
	int scale_img = 2;

	cv::KeyPoint point;

	float basicSize_    =12.0;
	static const float log2 = 0.693147180559945;
	static const float basicSize06=basicSize_*0.6;
	int scales_=64;
	float scalerange_   =30;
	static const float lb_scalerange = log(scalerange_)/(log2);
	int scale=std::max((int)(scales_/lb_scalerange*(log(kp.size/(basicSize06))/log2)+0.5),0);

	const int border = sizeList_[scale];


	cvSetImageROI(ipl_img, cvRect(kp.pt.x-border,kp.pt.y-border,border*2,border*2));

	img2 = cvCreateImage(cvSize(25*scale_img,25*scale_img),
			ipl_img->depth,
			ipl_img->nChannels);
	cvResize(ipl_img, img2);
}


__inline__ void showfeature(cv::Mat* img, cv::KeyPoint& kp, unsigned int* sizeList_, std::string name){
	IplImage *img2;
	IplImage ipl_img = *img;
	visfeature(&ipl_img, kp, sizeList_, img2);
	cvShowImage(name.c_str(),img2);
}

__inline__ void showdescriptor(__m128i* descriptor, int nWords, std::string name){
	IplImage* img;
	visdescriptor(descriptor, nWords,  img);
	cvShowImage(name.c_str(),img);

}

template <template<typename, typename> class descriptor_C_T, template<typename> class descriptor_A_T, typename descriptor_T,
template<typename, typename> class keypoint_C_T, template<typename> class keypoint_A_T, typename keypoint_T,
template<typename, typename> class edge_C_T, template<typename> class edge_A_T, typename edge_T>
__inline__ void showCluster(cv::Mat* img, descriptor_C_T<descriptor_T, descriptor_A_T<descriptor_T> >& cDescriptors,
		keypoint_C_T<keypoint_T, keypoint_A_T<keypoint_T> >& cKeypoints,
		edge_C_T<edge_T, edge_A_T<edge_T> >& cEdges,
		std::vector<int>& cCluster, int nWords, unsigned int* sizeList_)
{
	typedef typename std::vector<int>::iterator c_iterator;
	typedef typename descriptor_C_T<descriptor_T, descriptor_A_T<descriptor_T> >::iterator d_iterator;
	typedef typename keypoint_C_T<keypoint_T, keypoint_A_T<keypoint_T> >::iterator k_iterator;
	typedef typename edge_C_T<edge_T, edge_A_T<edge_T> >::iterator e_iterator;

	size_t nClusters = cCluster.size();

	float anglestep = 360/(float)nClusters;

	if(nClusters == 1)
		anglestep = 90;

	int dist = 120;

	int radius = (int)(((float)dist/2)/(tan(360./(2*nClusters)*PI/180)));

	int imgsze = (radius+100)*2;

	IplImage ipl_img;
	if(img!=NULL)
		ipl_img = *img;
	IplImage* img2 = cvCreateImage(cvSize(imgsze, imgsze),
			ipl_img.depth,
			ipl_img.nChannels);
	cvSet(img2, CV_RGB(255,255,255));

	CvScalar linecol = CV_RGB(255,225,7);
	CvScalar textcol = CV_RGB(0,0,0);
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.3, 0.3);

	IplImage* img_descriptor;
	IplImage* img_patch;

	int clidx = 0;
	for(c_iterator cl_it = cCluster.begin();cl_it!=cCluster.end();++cl_it, ++clidx){
		int x = imgsze/2 - radius * sin(clidx*anglestep*PI/180);
		int y = imgsze/2 - radius * cos(clidx*anglestep*PI/180);
		cv::Point pt1(x,y);
		int otherptcnt = 0;
		for(e_iterator edge_it = cEdges.begin();edge_it!=cEdges.end();++edge_it){
			int thickness = log2(edge_it->second)-7;
			bool isOtherPoint = false;
			if(edge_it->first.first == *cl_it){
				int clidx2 = 0;
				for(c_iterator cl_it_other = cCluster.begin();cl_it_other!=cCluster.end();++cl_it_other,++clidx2){
					if(edge_it->first.second == *cl_it_other){
						cv::Point pt2(imgsze/2 - radius * sin(clidx2*anglestep*PI/180),imgsze/2 - radius * cos(clidx2*anglestep*PI/180));
						cvLine(img2, pt1, pt2, linecol, thickness, CV_AA);
						isOtherPoint=true;
						break;
					}
				}
				if(!isOtherPoint){
					otherptcnt++;
					cv::Point pt2(imgsze/2 - radius * 2 * sin(clidx*anglestep*PI/180) - 10 * sin(otherptcnt*anglestep*2*PI/180),imgsze/2 - radius * 2 * cos(clidx*anglestep*PI/180) - 10 * sin(otherptcnt*anglestep*2*PI/180));
					cvLine(img2, pt1, pt2, linecol, thickness, CV_AA);
					pt2.x += 4;
					cvPutText(img2, convertInt(edge_it->first.second).c_str(), pt2, &font, textcol);
				}
			}
			if(edge_it->first.second == *cl_it){
				int clidx2 = 0;
				for(c_iterator cl_it_other = cCluster.begin();cl_it_other!=cCluster.end();++cl_it_other,++clidx2){
					if(edge_it->first.first == *cl_it_other){
						cv::Point pt2(imgsze/2 - radius * sin(clidx2*anglestep*PI/180),imgsze/2 - radius * cos(clidx2*anglestep*PI/180));
						cvLine(img2, pt1, pt2, linecol, thickness, CV_AA);
						isOtherPoint=true;
						break;
					}
				}
				if(!isOtherPoint){
					otherptcnt++;
					cv::Point pt2(imgsze/2 - radius * 2 * sin(clidx*anglestep*PI/180) - 10 * sin(otherptcnt*anglestep*2*PI/180),imgsze/2 - radius * 2 * cos(clidx*anglestep*PI/180) - 10 * sin(otherptcnt*anglestep*2*PI/180));
					cvLine(img2, pt1, pt2, linecol, thickness, CV_AA);
					pt2.x += 4;
					cvPutText(img2, convertInt(edge_it->first.first).c_str(), pt2, &font, textcol);
				}
			}
		}

	}

	clidx = 0;
	for(c_iterator cl_it = cCluster.begin();cl_it!=cCluster.end();++cl_it, ++clidx){
		int x = imgsze/2 - radius * sin(clidx*anglestep*PI/180);
		int y = imgsze/2 - radius * cos(clidx*anglestep*PI/180);
		visdescriptor(cDescriptors.at(*cl_it), nWords, img_descriptor);
		if(img!=NULL)
			visfeature(&ipl_img,cKeypoints.at(*cl_it),sizeList_, img_patch);

		cvSetImageROI(img2, cvRect(x-10-25,y-10-25,img_patch->width,img_patch->height));
		cvCopy(img_patch,img2);
		cvResetImageROI(img2);
		cvSetImageROI(img2, cvRect(x-8-25,y+15,img_descriptor->width,img_descriptor->height));
		cvCopy(img_descriptor,img2);
		cvResetImageROI(img2);
	}

	cvShowImage("Cluster",img2);
	cvWaitKey();

}

#endif /* VISUALIZE_H_ */







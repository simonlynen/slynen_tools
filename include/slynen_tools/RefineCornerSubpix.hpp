/*
 * RefineCornerSubpix.hpp
 *
 *  Created on: Oct 18, 2011
 *      Author: slynen
 */

#ifndef REFINECORNERSUBPIX_HPP_
#define REFINECORNERSUBPIX_HPP_

#include <opencv2/opencv.hpp>

using namespace cv;

void refineCornersSubpix(const Mat& prevImg, const Mat& nextImg, const vector<Point2f>& prevPts, vector<Point2f>& nextPts, vector<uchar>& status){

}

double IterateSubPix(const Mat& prevImg, const Mat& nextImg, Point2f& pt_first, Point2f& pt_second)
{
	if(!im.in_image_with_border(ir_rounded(v2Center), mnPatchSize / 2 + 1))
    return -1.0;       // Negative return value indicates off edge of image

  // Position of top-left corner of patch in search level
  Vector<2> v2Base = v2Center - vec(mirCenter);

  // I.C. JT*d accumulator
  Vector<3> v3Accum = Zeros;

  ImageRef ir;

  CVD::byte* pTopLeftPixel;

  // Each template pixel will be compared to an interpolated target pixel
  // The target value is made using bilinear interpolation as the weighted sum
  // of four target image pixels. Calculate mixing fractions:
  double dX = v2Base[0]-floor(v2Base[0]); // Distances from pixel center of TL pixel
  double dY = v2Base[1]-floor(v2Base[1]);
  float fMixTL = (1.0 - dX) * (1.0 - dY);
  float fMixTR = (dX)       * (1.0 - dY);
  float fMixBL = (1.0 - dX) * (dY);
  float fMixBR = (dX)       * (dY);

  // Loop over template image
  unsigned long nRowOffset = &kf.aLevels[mnSearchLevel].im[ImageRef(0,1)] - &kf.aLevels[mnSearchLevel].im[ImageRef(0,0)];
  for(ir.y = 1; ir.y < mnPatchSize - 1; ir.y++)
    {
      pTopLeftPixel = &im[::ir(v2Base) + ImageRef(1,ir.y)]; // n.b. the x=1 offset, as with y
      for(ir.x = 1; ir.x < mnPatchSize - 1; ir.x++)
	{
	  float fPixel =   // Calc target interpolated pixel
	    fMixTL * pTopLeftPixel[0]          + fMixTR * pTopLeftPixel[1] +
	    fMixBL * pTopLeftPixel[nRowOffset] + fMixBR * pTopLeftPixel[nRowOffset + 1];
	  pTopLeftPixel++;
	  double dDiff = fPixel - mimTemplate[ir] + mdMeanDiff;
	  v3Accum[0] += dDiff * mimJacs[ir - ImageRef(1,1)].first;
	  v3Accum[1] += dDiff * mimJacs[ir - ImageRef(1,1)].second;
	  v3Accum[2] += dDiff;  // Update JT*d
	};
    }

  // All done looping over image - find JTJ^-1 * JTd:
  Vector<3> v3Update = mm3HInv * v3Accum;
  mv2SubPixPos -= v3Update.slice<0,2>() * LevelScale(mnSearchLevel);
  mdMeanDiff -= v3Update[2];

  double dPixelUpdateSquared = v3Update.slice<0,2>() * v3Update.slice<0,2>();
  return dPixelUpdateSquared;
}


// Iterate inverse composition until convergence. Since it should never have
// to travel more than a pixel's distance, set a max number of iterations;
// if this is exceeded, consider the IC to have failed.
bool PatchFinder::IterateSubPixToConvergence(KeyFrame &kf, int nMaxIts)
{
  const double dConvLimit = 0.03;
  bool bConverged = false;
  int nIts;
  for(nIts = 0; nIts < nMaxIts && !bConverged; nIts++)
    {
      double dUpdateSquared = IterateSubPix(kf);
      if(dUpdateSquared < 0) // went off edge of image
	return false;
      if(dUpdateSquared < dConvLimit*dConvLimit)
	return true;
    }
  return false;
}


#endif /* REFINECORNERSUBPIX_HPP_ */

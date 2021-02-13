/*    This file is a part of Calyp project
 *    Copyright (C) 2014-2021  by Joao Carreira   (jfmcarreira@gmail.com)
 *                                Luis Lucas      (luisfrlucas@gmail.com)
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * \file     SaliencyDetection.h
 * \brief    Modules to detect saliency in image and videos
 */

#ifndef __SALIENCYDETECTION_H__
#define __SALIENCYDETECTION_H__

// CalypLib
#include "lib/CalypOpenCVModuleIf.h"

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/saliency.hpp>

using cv::Mat;

/**
 * \ingroup  Calyp_Modules
 * @defgroup Calyp_Modules_Saliency Saliency
 * @{
 * Frame processing modules for saliency detection
 * @}
 */

class SaliencyDetectionModule : public CalypOpenCVModuleIf
{
protected:
  Mat* m_pcvSaliency;
  cv::Ptr<cv::saliency::Saliency> m_ptrSaliencyAlgorithm;

public:
  SaliencyDetectionModule();
  virtual ~SaliencyDetectionModule() {}
  void destroy_using_opencv();
};

/**
 * \ingroup  Calyp_Modules_Saliency
 * \class    SaliencyDetectionSpectral
 * \brief    Starting from the principle of natural image statistics,
 *  this method simulate the behavior of pre-attentive visual search.
 * The algorithm analyze the log spectrum of each image and obtain the
 * spectral residual. Then transform the spectral residual to spatial
 * domain to obtain the saliency map, which suggests the positions of proto-objects.
 * Xiaodi Hou and Liqing Zhang. Saliency detection: A spectral residual
 * approach. In Computer Vision and Pattern Recognition, 2007.
 * CVPR'07. IEEE Conference on, pages 1–8. IEEE, 2007.
 */
class SaliencyDetectionSpectral : public SaliencyDetectionModule
{
  REGISTER_CLASS_FACTORY( SaliencyDetectionSpectral )
private:
  bool m_bBinaryMap;

public:
  SaliencyDetectionSpectral();
  virtual ~SaliencyDetectionSpectral() {}
  Mat* create_using_opencv( std::vector<Mat*> apcMatList );
  Mat* process_using_opencv( std::vector<Mat*> apcMatList );
};

/**
 * \ingroup  Calyp_Modules_Saliency
 * \class    SaliencyDetectionFineGrained
 * \brief    This method calculates saliency based on center-surround
 * differences. High resolution saliency maps are generated in real
 * time by using integral images.
 * Sebastian Montabone and Alvaro Soto. Human detection using a mobile
 * platform and novel features derived from a visual saliency mechanism.
 * In Image and Vision Computing, Vol. 28 Issue 3, pages 391–402. Elsevier, 2010.
 */
class SaliencyDetectionFineGrained : public SaliencyDetectionModule
{
  REGISTER_CLASS_FACTORY( SaliencyDetectionFineGrained )
public:
  SaliencyDetectionFineGrained();
  virtual ~SaliencyDetectionFineGrained() {}
  Mat* create_using_opencv( std::vector<Mat*> apcMatList );
  Mat* process_using_opencv( std::vector<Mat*> apcMatList );
};

/**
 * \ingroup  Calyp_Modules_Saliency
 * \class    SaliencyDetectionBinWangApr2014
 * \brief    Fast Self-tuning Background Subtraction Algorithm based on the work
 *					 Wang and P. Dudek “A Fast Self-tuning Background Subtraction Algorithm”,
 *					 in proc of IEEE Workshop on Change Detection, 2014
 */
class SaliencyDetectionBinWangApr2014 : public SaliencyDetectionModule
{
  REGISTER_CLASS_FACTORY( SaliencyDetectionBinWangApr2014 )
public:
  SaliencyDetectionBinWangApr2014();
  virtual ~SaliencyDetectionBinWangApr2014() {}
  Mat* create_using_opencv( std::vector<Mat*> apcMatList );
  Mat* process_using_opencv( std::vector<Mat*> apcMatList );
};

#endif  // __SALIENCYDETECTION_H__

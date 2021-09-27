/*    This file is a part of Calyp project
 *    Copyright (C) 2014-2019  by Joao Carreira   (jfmcarreira@gmail.com)
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
 * \file     ThreeSixtyDownsampling.h
 * \brief    Adaptive Latitude Downsampling
 */

#ifndef __THREESIXTYDOWNSAMPLING_H__
#define __THREESIXTYDOWNSAMPLING_H__

// CalypLib
#include "lib/CalypModuleIf.h"
#include "opencv2/core/mat.hpp"
#include "opencv2/core/types.hpp"

using cv::Mat;
using cv::Mat_;
using cv::Point;

class ThreeSixtyDownsampling : public CalypModuleIf
{
  REGISTER_CLASS_FACTORY( ThreeSixtyDownsampling )
private:
  unsigned m_uiDownsampling;
  unsigned m_uiX0;
  unsigned m_uiY0;
  int m_iInterpolation;
  int m_iInpaintMethod;
  unsigned m_bForceIntSlope;
  unsigned m_iRearrange;

  bool m_isOdd;
  int m_iWidth;
  int m_iHeight;

  double m_dPixelRatio;

  CalypFrame* m_pcResultedFrame;
  CalypFrame* m_pcDownsampled;

  // Values one per channel
  std::vector<Mat*> m_cvDownsamplingMask;
  std::vector<Mat_<Point>*> m_cvReshapePoints;
  bool createDownsamplingMask( CalypFrame* pcFrame );

  void downsamplingOperation( CalypFrame* pcInputFrame );
  void upsamplingOperation( CalypFrame* pcInputFrame );

public:
  ThreeSixtyDownsampling();
  virtual ~ThreeSixtyDownsampling() {}
  bool create( std::vector<CalypFrame*> apcFrameList );
  CalypFrame* process( std::vector<CalypFrame*> apcFrameList );
  ClpString moduleInfo();
  bool keyPressed( enum Module_Key_Supported value );
  void destroy();
};

#endif  // __THREESIXTYDOWNSAMPLING_H__

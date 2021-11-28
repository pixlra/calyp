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
 * \file     ShapeDetection.cpp
 */

#include "ShapeDetection.h"

#include <cstdlib>
#include <opencv2/imgproc.hpp>

using cv::Mat;
using cv::Ptr;

ShapeDetection::ShapeDetection()
    : m_rng( 12345 )
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = ClpModuleType::FrameProcessing;
  m_pchModuleCategory = "Utilities";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = ClpModuleFeature::NewWindow;
  m_bConvertToGray = false;

  /* Module Definition */
  m_pchModuleName = "ShapeDetection";
  m_pchModuleLongName = "Shape detection";
  m_pchModuleTooltip = "Detect shapes in images";
}

cv::Mat* ShapeDetection::create_using_opencv( const std::vector<Mat>& apcMatList )
{
  m_resultImage = std::make_unique<Mat>( apcMatList[0].rows, apcMatList[0].cols, CV_8UC1 );
  return m_resultImage.get();
}

Mat* ShapeDetection::process_using_opencv( const std::vector<Mat>& apcMatList )
{
  Mat canny_output;
  cv::Canny( apcMatList[0], canny_output, 30, 100 );

  std::vector<std::vector<cv::Point> > contours;
  findContours( canny_output, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );

  std::vector<std::vector<cv::Point> > hull( contours.size() );
  for( size_t i = 0; i < contours.size(); i++ )
  {
    cv::convexHull( contours[i], hull[i] );
  }
  for( size_t i = 0; i < contours.size(); i++ )
  {
    cv::Scalar color = cv::Scalar( m_rng.uniform( 0, 256 ), m_rng.uniform( 0, 256 ), m_rng.uniform( 0, 256 ) );
    cv::drawContours( *m_resultImage, contours, (int)i, color );
    cv::drawContours( *m_resultImage, hull, (int)i, color );
  }

  return m_resultImage.get();
}

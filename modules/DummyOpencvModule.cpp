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
 * \file     DummyOpencvModule.cpp
 */

#include "DummyOpencvModule.h"

#include <cstdlib>

//#include <opencv2/video/video.hpp>

using cv::Mat;
using cv::Ptr;
using namespace cv::saliency;

DummyOpencvModule::DummyOpencvModule()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = ClpModuleType::FrameProcessing;
  m_pchModuleCategory = "Dummy";
  m_pchModuleName = "DummyOpencvModule";
  m_pchModuleLongName = "Shape detection";
  m_pchModuleTooltip = "Detect shapes in images";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = ClpModuleFeature::NewWindow | ClpModuleFeature::Options;
  m_bConvertToGray = false;

  m_cModuleOptions.addOptions()( "Convert to Gray", m_bConvertToGray, "Measure a binary saliency map [false]" );
}

cv::Mat* DummyOpencvModule::create_using_opencv( const std::vector<Mat>& apcMatList )
{
  m_resultImage = std::make_unique<Mat>( apcMatList[0].rows, apcMatList[0].cols, CV_8UC1 );
  return m_resultImage.get();
}

Mat* DummyOpencvModule::process_using_opencv( const std::vector<Mat>& apcMatList )
{
  apcMatList[0].copyTo( *m_resultImage );
  return m_resultImage.get();
}

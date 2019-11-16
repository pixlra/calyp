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
 * \file     SaliencyDetection.cpp
 * \brief    Modules to measure optical flow
 */

#include "SaliencyDetection.h"

#include <cstdlib>

//#include <opencv2/video/video.hpp>

using cv::Mat;
using cv::Ptr;
using namespace cv::saliency;

SaliencyDetectionModule::SaliencyDetectionModule()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "Saliency";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_NEW_WINDOW;
  m_bConvertToGray = true;
}

void SaliencyDetectionModule::destroy_using_opencv()
{
  delete m_pcvSaliency;
}

SaliencyDetectionSpectral::SaliencyDetectionSpectral()
{
  /* Module Definition */
  m_pchModuleName = "SaliencySpectralResidual";
  m_pchModuleLongName = "Spectral residual";
  m_pchModuleTooltip = "Measure saliency using spectral residual method";

  m_uiModuleRequirements = m_uiModuleRequirements | CLP_MODULE_REQUIRES_OPTIONS;

  m_cModuleOptions.addOptions() /**/
      ( "Binary map", m_bBinaryMap, "Measure a binary saliency map [false]" );

  m_bBinaryMap = false;
}

bool SaliencyDetectionSpectral::create_using_opencv( std::vector<Mat*> apcMatList )
{
  m_pcvSaliency = new Mat;
  m_ptrSaliencyAlgorithm = StaticSaliencySpectralResidual::create();
  return true;
}

Mat* SaliencyDetectionSpectral::process_using_opencv( std::vector<Mat*> apcMatList )
{
  Mat cvSaliency;
  m_ptrSaliencyAlgorithm->computeSaliency( *apcMatList[0], cvSaliency );
  Mat cvBinaryMap;
  StaticSaliencySpectralResidual spec;
  spec.computeBinaryMap( cvSaliency, *m_pcvSaliency );
  if( !m_bBinaryMap )
  {
    cvSaliency *= 255;
    cvSaliency.convertTo( *m_pcvSaliency, CV_8UC1 );
  }
  return m_pcvSaliency;
}

SaliencyDetectionFineGrained::SaliencyDetectionFineGrained()
{
  /* Module Definition */
  m_pchModuleName = "SaliencyDetectionFineGrained";
  m_pchModuleLongName = "Fine grained";
  m_pchModuleTooltip = "Measure saliency using fine grained method";
}

bool SaliencyDetectionFineGrained::create_using_opencv( std::vector<Mat*> apcMatList )
{
  m_pcvSaliency = new Mat;
  m_ptrSaliencyAlgorithm = cv::saliency::StaticSaliencyFineGrained::create();
  return true;
}

Mat* SaliencyDetectionFineGrained::process_using_opencv( std::vector<Mat*> apcMatList )
{
  m_ptrSaliencyAlgorithm->computeSaliency( *apcMatList[0], *m_pcvSaliency );
  return m_pcvSaliency;
}

SaliencyDetectionBinWangApr2014::SaliencyDetectionBinWangApr2014()
{
  /* Module Definition */
  m_pchModuleName = "SaliencyDetectionBinWangApr2014";
  m_pchModuleLongName = "Fast Self-tuning Background Subtraction Algorithm";
  m_pchModuleTooltip = "Measure saliency using a fast self-tuning background subtraction algorithm";
}

bool SaliencyDetectionBinWangApr2014::create_using_opencv( std::vector<Mat*> apcMatList )
{
  m_pcvSaliency = new Mat;
  m_ptrSaliencyAlgorithm = MotionSaliencyBinWangApr2014::create();
  m_ptrSaliencyAlgorithm.dynamicCast<MotionSaliencyBinWangApr2014>()->setImagesize( apcMatList[0]->cols, apcMatList[0]->rows );
  m_ptrSaliencyAlgorithm.dynamicCast<MotionSaliencyBinWangApr2014>()->init();
  return true;
}

Mat* SaliencyDetectionBinWangApr2014::process_using_opencv( std::vector<Mat*> apcMatList )
{
  if( m_ptrSaliencyAlgorithm->computeSaliency( *apcMatList[0], *m_pcvSaliency ) )
  {
    *m_pcvSaliency *= 255;
    m_pcvSaliency->convertTo( *m_pcvSaliency, CV_8UC1 );
    //cv::cvtColor(m_matSaliency, m_matSaliency, cv::COLOR_BGR2GRAY);
  }
  return m_pcvSaliency;
}

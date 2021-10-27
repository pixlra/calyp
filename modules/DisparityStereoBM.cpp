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
 * \file     DisparityStereoBM.cpp
 * \brief    Measure the disparity between two images using the Stereo Var
 * method (OpenCV)
 */

#include "DisparityStereoBM.h"

#include "lib/CalypFrame.h"

DisparityStereoBM::DisparityStereoBM()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = ClpModuleType::FrameProcessing;
  m_pchModuleCategory = "Stereo";
  m_pchModuleName = "Disparity-BM";
  m_pchModuleLongName = "Block macth based disparity";
  m_pchModuleTooltip = "Measure the disparity between two images using the "
                       "Stereo BM method (OpenCV)";
  m_uiNumberOfFrames = 2;
  m_uiModuleRequirements = ClpModuleFeature::SkipWhilePlaying | ClpModuleFeature::NewWindow | ClpModuleFeature::Options;

  m_cModuleOptions.addOptions() /**/
      ( "block_size", m_uiBlockSize, "Block Size (positive odd number) [9]" );

  m_pcDisparityFrame = NULL;
  m_uiBlockSize = 9;
}

bool DisparityStereoBM::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_
  m_pcDisparityFrame =
      new CalypFrame( apcFrameList[0]->getWidth(), apcFrameList[0]->getHeight(), ClpPixelFormats::Gray );
  if( ( m_uiBlockSize % 2 ) == 0 )
  {
    m_uiBlockSize++;
  }
  m_uiNumberOfDisparities = ( ( apcFrameList[0]->getWidth() / 8 ) + 15 ) & -16;
#if( CV_MAJOR_VERSION == 2 )
  m_cStereoBM.state->preFilterCap = 31;
  m_cStereoBM.state->SADWindowSize = 9;
  m_cStereoBM.state->minDisparity = 0;
  m_cStereoBM.state->textureThreshold = 10;
  m_cStereoBM.state->uniquenessRatio = 15;
  m_cStereoBM.state->speckleWindowSize = 100;
  m_cStereoBM.state->speckleRange = 32;
  m_cStereoBM.state->disp12MaxDiff = 1;
  m_cStereoBM.state->SADWindowSize = m_uiBlockSize;
  m_cStereoBM.state->numberOfDisparities = m_uiNumberOfDisparities;
#else
  m_cStereoBM = cv::StereoBM::create( 16, m_uiBlockSize );
  m_cStereoBM->setPreFilterCap( 31 );
  m_cStereoBM->setBlockSize( m_uiBlockSize );
  m_cStereoBM->setMinDisparity( 0 );
  m_cStereoBM->setNumDisparities( m_uiNumberOfDisparities );
  m_cStereoBM->setTextureThreshold( 10 );
  m_cStereoBM->setUniquenessRatio( 15 );
  m_cStereoBM->setSpeckleWindowSize( 100 );
  m_cStereoBM->setSpeckleRange( 32 );
  m_cStereoBM->setDisp12MaxDiff( 1 );
#endif
  return true;
}
CalypFrame* DisparityStereoBM::process( std::vector<CalypFrame*> apcFrameList )
{
  CalypFrame* InputLeft = apcFrameList[0];
  CalypFrame* InputRight = apcFrameList[1];

  cv::Mat leftImage, rightImage;
  if( !InputLeft->toMat( leftImage, true ) || !InputRight->toMat( rightImage, true ) )
  {
    return m_pcDisparityFrame;
  }

  cv::Mat disparityImage, disparityImage8;

#if( CV_MAJOR_VERSION == 2 )
  m_cStereoBM( leftImage, rightImage, disparityImage );
#else
  m_cStereoBM->compute( leftImage, rightImage, disparityImage );
#endif

  disparityImage.convertTo( disparityImage8, CV_8U );
  m_pcDisparityFrame->fromMat( disparityImage8 );
  return m_pcDisparityFrame;
}

void DisparityStereoBM::destroy()
{
  if( m_pcDisparityFrame )
    delete m_pcDisparityFrame;
  m_pcDisparityFrame = NULL;
}

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
 * \file     DisparityStereoSGBM.cpp
 * \brief    Measure the disparity between two images using the SGBM method
 * (OpenCV)
 */

#include "DisparityStereoSGBM.h"

DisparityStereoSGBM::DisparityStereoSGBM()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "Stereo";
  m_pchModuleName = "Disparity-SGBM";
  m_pchModuleName = "SGBM based disparity";
  m_pchModuleTooltip = "Measure the disparity between two images using the "
                       "Stereo SGBM method (OpenCV)";
  m_uiNumberOfFrames = 2;
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_SKIP_WHILE_PLAY | CLP_MODULE_REQUIRES_NEW_WINDOW | CLP_MODULE_REQUIRES_OPTIONS;

  m_cModuleOptions.addOptions() /**/
      ( "block_size", m_uiBlockSize, "Block Size (positive odd number) [3]" )( "HHAlgorithm", m_bUseHH,
                                                                               "Use HH algorithm [false]" );

  m_pcDisparityFrame = NULL;
  m_uiBlockSize = 3;
  m_uiNumberOfDisparities = 0;
}

bool DisparityStereoSGBM::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_
  m_pcDisparityFrame =
      new CalypFrame( apcFrameList[0]->getWidth(), apcFrameList[0]->getHeight(), CLP_GRAY );
  if( ( m_uiBlockSize % 2 ) == 0 )
  {
    m_uiBlockSize++;
  }
  m_uiNumberOfDisparities = ( ( apcFrameList[0]->getWidth() / 8 ) + 15 ) & -16;
  int cn = apcFrameList[0]->getNumberChannels();
#if( CV_MAJOR_VERSION >= 3 )
  m_cStereoMatch = cv::StereoSGBM::create( 0, 16, m_uiBlockSize );
  m_cStereoMatch->setPreFilterCap( 63 );

  m_cStereoMatch->setP1( 8 * cn * m_uiBlockSize * m_uiBlockSize );
  m_cStereoMatch->setP2( 32 * cn * m_uiBlockSize * m_uiBlockSize );
  m_cStereoMatch->setMinDisparity( 0 );
  m_cStereoMatch->setNumDisparities( m_uiNumberOfDisparities );
  m_cStereoMatch->setUniquenessRatio( 10 );
  m_cStereoMatch->setSpeckleWindowSize( 100 );
  m_cStereoMatch->setSpeckleRange( 32 );
  m_cStereoMatch->setDisp12MaxDiff( 1 );
  // m_cStereoMatch->setMode( alg == STEREO_HH ? StereoSGBM::MODE_HH :
  // StereoSGBM::MODE_SGBM );
  m_cStereoMatch->setMode( m_bUseHH ? cv::StereoSGBM::MODE_HH : cv::StereoSGBM::MODE_SGBM );
#else
  m_cStereoMatch.preFilterCap = 63;
  m_cStereoMatch.SADWindowSize = m_uiBlockSize;
  m_cStereoMatch.P1 = 8 * cn * m_uiBlockSize * m_uiBlockSize;
  m_cStereoMatch.P2 = 32 * cn * m_uiBlockSize * m_uiBlockSize;
  m_cStereoMatch.minDisparity = 0;
  m_cStereoMatch.numberOfDisparities = m_uiNumberOfDisparities;
  m_cStereoMatch.uniquenessRatio = 10;
  m_cStereoMatch.speckleWindowSize = 100;
  m_cStereoMatch.speckleRange = 32;
  m_cStereoMatch.disp12MaxDiff = 1;
  m_cStereoMatch.fullDP = m_bUseHH ? 1 : 0;
#endif
  return true;
}

CalypFrame* DisparityStereoSGBM::process( std::vector<CalypFrame*> apcFrameList )
{
  CalypFrame* InputLeft = apcFrameList[0];
  CalypFrame* InputRight = apcFrameList[1];
  cv::Mat leftImage, rightImage;
  if( !InputLeft->toMat( leftImage, true ) || !InputRight->toMat( rightImage, true ) )
  {
    return m_pcDisparityFrame;
  }
  cv::Mat disparityImage, disparityImage8;
#if( CV_MAJOR_VERSION >= 3 )
  m_cStereoMatch->compute( leftImage, rightImage, disparityImage );
#else
  m_cStereoMatch( leftImage, rightImage, disparityImage );
#endif
  disparityImage.convertTo( disparityImage8, CV_8U );
  m_pcDisparityFrame->fromMat( disparityImage8 );
  return m_pcDisparityFrame;
}

void DisparityStereoSGBM::destroy()
{
  if( m_pcDisparityFrame )
    delete m_pcDisparityFrame;
  m_pcDisparityFrame = NULL;
}

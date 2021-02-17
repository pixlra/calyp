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
 * \file     FrameResampling.cpp
 * \brief    Modules to resampling frames
 */

#include "FrameResampling.h"

#include <cstdlib>

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>

using cv::Mat;
using cv::Size;

FrameResampling::FrameResampling()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "Utilities";
  m_pchModuleName = "FrameResampling";
  m_pchModuleLongName = "Frame Resampling (Spatial)";
  m_pchModuleTooltip = "Resampling frame to an abritary resolution";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_OPTIONS;

  m_cModuleOptions.addOptions()                                 /**/
      ( "width", m_iWidth, "Width of the crop region [-1]" )    /**/
      ( "height", m_iHeight, "Height of the crop region [-1]" ) /**/
      ( "Interpolation", m_iInterpolation, "Interpolation method (1-4) [4]" );

  m_iInterpolation = 4;
  m_iWidth = -1;
  m_iHeight = -1;
}

bool FrameResampling::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_
  switch( m_iInterpolation )
  {
  case 2:
    m_iInterpolation = cv::INTER_LINEAR;
    break;
  case 3:
    m_iInterpolation = cv::INTER_CUBIC;
    break;
  case 4:
    m_iInterpolation = cv::INTER_LANCZOS4;
    break;
  default:
    m_iInterpolation = cv::INTER_NEAREST;
  }
  if( m_iWidth == -1 )
  {
    m_iWidth = apcFrameList[0]->getWidth();
  }
  if( m_iHeight == -1 )
  {
    m_iHeight = apcFrameList[0]->getHeight() * double( m_iWidth ) / double( apcFrameList[0]->getWidth() );
  }
  m_pcOutputFrame = new CalypFrame( m_iWidth, m_iHeight, apcFrameList[0]->getPelFormat(), apcFrameList[0]->getBitsPel() );
  return m_pcOutputFrame;
}

CalypFrame* FrameResampling::process( std::vector<CalypFrame*> apcFrameList )
{
  m_pcOutputFrame->reset();
  for( unsigned ch = 0; ch < apcFrameList[0]->getNumberChannels(); ch++ )
  {
    Mat inputFrame, outputFrame;
    cv::Size outSize( m_pcOutputFrame->getWidth( ch ), m_pcOutputFrame->getHeight( ch ) );
    apcFrameList[0]->toMat( inputFrame, true, false, ch );
    cv::resize( inputFrame, outputFrame, outSize, 0, 0, m_iInterpolation );
    m_pcOutputFrame->fromMat( outputFrame, ch );
  }
  return m_pcOutputFrame;
}

void FrameResampling::destroy()
{
  if( m_pcOutputFrame )
    delete m_pcOutputFrame;
}

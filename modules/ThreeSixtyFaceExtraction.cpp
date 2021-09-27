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
 * \file     ThreeSixtyFaceExtraction.cpp
 */

#include "ThreeSixtyFaceExtraction.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>

using cv::InputArray;
using cv::Mat;
using cv::OutputArray;
using cv::Point;
using cv::Rect;
using cv::Size;

ThreeSixtyFaceExtraction::ThreeSixtyFaceExtraction()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "360Video";
  m_pchModuleLongName = "Face Extration";
  m_pchModuleName = "ThreeSixtyFaceExtration";
  m_pchModuleTooltip = "Extract a 360 video face";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_SKIP_WHILE_PLAY;

  m_cModuleOptions.addOptions()                                                            /**/
      ( "faceNum", m_uiFaceNum, "360 proejction face to be outputed [0]" )                 /**/
      ( "projection", m_uiProjectionType, "Projection [1] \n 1: Cubemap" )                 /**/
      ( "partitions", m_uiNumberOfPartitionsPerFace, "Number of partitions per face [1]" ) /**/
      ;

  m_uiFaceNum = 2;
  m_uiProjectionType = 1;
  m_uiNumberOfPartitionsPerFace = 1;
  m_uiFacesX = 0;
  m_uiFacesY = 0;

  m_pcTmpFrame = NULL;
}

bool ThreeSixtyFaceExtraction::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_

  switch( m_uiProjectionType )
  {
  case 1:
    m_uiFacesX = 3 * m_uiNumberOfPartitionsPerFace;
    m_uiFacesY = 2 * m_uiNumberOfPartitionsPerFace;
    break;
  default:
    assert( 0 );
  }

  unsigned int width = apcFrameList[0]->getWidth() / m_uiFacesX;
  unsigned int height = apcFrameList[0]->getHeight() / m_uiFacesY;

  m_pcFrameExtractionFrame = std::make_unique<CalypFrame>( width, height, apcFrameList[0]->getPelFormat(), apcFrameList[0]->getBitsPel() );

  return true;
}

CalypFrame* ThreeSixtyFaceExtraction::process( std::vector<CalypFrame*> apcFrameList )
{
  m_pcFrameExtractionFrame->reset();

  unsigned x = -1;
  unsigned y = -1;

  switch( m_uiProjectionType )
  {
  case 1:  // Cube-map
    x = m_uiFaceNum % m_uiFacesX * m_pcFrameExtractionFrame->getWidth();
    y = ( m_uiFaceNum / m_uiFacesX ) * m_pcFrameExtractionFrame->getHeight();
    break;
  default:
    assert( 0 );
  }

  m_pcFrameExtractionFrame->copyFrom( apcFrameList[0], x, y );
  return m_pcFrameExtractionFrame.get();
}

void ThreeSixtyFaceExtraction::destroy()
{
}

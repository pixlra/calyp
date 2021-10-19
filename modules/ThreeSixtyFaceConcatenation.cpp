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
 * \file     ThreeSixtyFaceConcatenation.cpp
 */

#include "ThreeSixtyFaceConcatenation.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>

using cv::InputArray;
using cv::Mat;
using cv::OutputArray;
using cv::Point;
using cv::Rect;
using cv::Size;

ThreeSixtyFaceConcatenation::ThreeSixtyFaceConcatenation()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = ClpModuleType::FrameProcessing;
  m_pchModuleCategory = "360Video";
  m_pchModuleLongName = "Face Concatenation";
  m_pchModuleName = "ThreeSixtyFaceConcatenation";
  m_pchModuleTooltip = "Concatenate 360 video faces";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = ClpModuleFeature::SkipWhilePlaying |
                           ClpModuleFeature::Options |
                           ClpModuleFeature::VariableNumOfFrames;

  m_cModuleOptions.addOptions()                                                            /**/
      ( "projection", m_uiProjectionType, "Projection [1] \n 1: Cubemap (6 faces)" )       /**/
      ( "partitions", m_uiNumberOfPartitionsPerFace, "Number of partitions per face [1]" ) /**/
      ;

  m_uiProjectionType = 1;
  m_uiNumberOfPartitionsPerFace = 1;
  m_uiFacesX = 0;
  m_uiFacesY = 0;
}

bool ThreeSixtyFaceConcatenation::create( std::vector<CalypFrame*> apcFrameList )
{
  if( apcFrameList.size() < 1 )
    return false;
  for( unsigned int i = 0; i < apcFrameList.size(); i++ )
    if( !apcFrameList[i] )
      return false;

  for( unsigned int i = 1; i < apcFrameList.size(); i++ )
    if( !apcFrameList[i]->haveSameFmt( apcFrameList[0], CalypFrame::MATCH_COLOR_SPACE |
                                                            CalypFrame::MATCH_RESOLUTION |
                                                            CalypFrame::MATCH_BITS ) )
      return false;

  switch( m_uiProjectionType )
  {
  case 1:
    m_uiFacesX = 3 * m_uiNumberOfPartitionsPerFace;
    m_uiFacesY = 2 * m_uiNumberOfPartitionsPerFace;
    break;
  default:
    return false;
  }

  if( apcFrameList.size() != m_uiFacesX * m_uiFacesY )
    return false;

  unsigned int width = apcFrameList[0]->getWidth() * m_uiFacesX;
  unsigned int height = apcFrameList[0]->getHeight() * m_uiFacesY;

  m_pcConcatenatedFaces = std::make_unique<CalypFrame>( width, height, apcFrameList[0]->getPelFormat(), apcFrameList[0]->getBitsPel() );

  return true;
}

CalypFrame* ThreeSixtyFaceConcatenation::process( std::vector<CalypFrame*> apcFrameList )
{
  m_pcConcatenatedFaces->reset();

  switch( m_uiProjectionType )
  {
  case 1:  // Cube-map
    for( unsigned i = 0; i < m_uiFacesX * m_uiFacesY; i++ )
    {
      CalypFrame* pcFrame = apcFrameList[i];
      unsigned x = i % m_uiFacesX * pcFrame->getWidth();
      unsigned y = i / m_uiFacesX * pcFrame->getHeight();
      m_pcConcatenatedFaces->copyTo( pcFrame, x, y );
    }
    break;
  default:
    assert( 0 );
  }

  return m_pcConcatenatedFaces.get();
}

void ThreeSixtyFaceConcatenation::destroy()
{
}

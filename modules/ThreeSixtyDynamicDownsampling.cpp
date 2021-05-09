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
 * \file     ThreeSixtyDynamicDownsampling.cpp
 */

#include "ThreeSixtyDynamicDownsampling.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>

using cv::InputArray;
using cv::Mat;
using cv::OutputArray;
using cv::Point;
using cv::Rect;
using cv::Size;

ThreeSixtyDynamicDownsampling::ThreeSixtyDynamicDownsampling()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "360Video";
  m_pchModuleLongName = "Dynamic Downsampling";
  m_pchModuleName = "ThreeSixtyDynamicDownsampling";
  m_pchModuleTooltip = "Dynamic downsampling of 360 videos";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_SKIP_WHILE_PLAY | CLP_MODULE_REQUIRES_OPTIONS;

  m_cModuleOptions.addOptions()                                                       /**/
      ( "CodingFaceWidth", m_uiFaceWidth, "Coding Face Width [same as input]" )       /**/
      ( "CodingFaceHeight", m_uiFaceHeight, "Coding Face Height [same as input]" )    /**/
      ( "SourceFPStructure", m_arrayInputDefinitionString, "Input frame packing" )    /**/
      ( "CodingFPStructure", m_arrayOutputDefinitionString, "Coding frame packing" ); /**/

  m_arrayInputDefinitionString = "6 2 3  0 0 0 0 100   1 0 1 0 100   2 0 2 0 100  0 1 3 0 100   1 1 4 0 100   2 1 5 0 100";

  m_arrayOutputDefinitionString = "6 3 3  0 0 0 0 100   1 0 1 0 200   0 1 2 0 100  0 2 3 0 100   1 2 4 0 100   2 2 5 0 100 ";

  m_uiFaceWidth = 0;
  m_uiFaceHeight = 0;
}

void processFPSString( const ClpString& fps, ClpVideoFPStruct* pcFPSStruct )
{
  std::istringstream ss( fps );
  ss >> pcFPSStruct->numFaces;
  ss >> pcFPSStruct->rows;
  ss >> pcFPSStruct->cols;
  for( int i = 0; i < pcFPSStruct->numFaces; i++ )
  {
    ss >> pcFPSStruct->faces[i].x;
    ss >> pcFPSStruct->faces[i].y;
    ss >> pcFPSStruct->faces[i].id;
    ss >> pcFPSStruct->faces[i].rot;
    ss >> pcFPSStruct->faces[i].ratio;
  }
}

bool ThreeSixtyDynamicDownsampling::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_

  processFPSString( m_arrayInputDefinitionString, &m_cInputFPSStruct );
  processFPSString( m_arrayOutputDefinitionString, &m_cOutputFPSStruct );

  if( !m_uiFaceWidth && !m_uiFaceHeight )
  {
    m_uiFaceWidth = apcFrameList[0]->getWidth() / m_cInputFPSStruct.cols;
    m_uiFaceHeight = apcFrameList[0]->getHeight() / m_cInputFPSStruct.rows;
  }
  else if( !m_uiFaceHeight )
  {
    m_uiFaceHeight = m_uiFaceWidth;
  }

  m_pcOutputFrame = new CalypFrame( m_uiFaceWidth * m_cOutputFPSStruct.cols,
                                    m_uiFaceHeight * m_cOutputFPSStruct.rows,
                                    apcFrameList[0]->getPelFormat(), apcFrameList[0]->getBitsPel() );

  return true;
}

CalypFrame* ThreeSixtyDynamicDownsampling::process( std::vector<CalypFrame*> apcFrameList )
{
  m_pcOutputFrame->reset();

  for( unsigned ch = 0; ch < apcFrameList[0]->getNumberChannels(); ch++ )
  {
    Mat inputFrame;
    apcFrameList[0]->toMat( inputFrame, true, false, ch );

    unsigned inGridW = apcFrameList[0]->getWidth( ch ) / m_cInputFPSStruct.cols;
    unsigned inGridH = apcFrameList[0]->getHeight( ch ) / m_cInputFPSStruct.rows;

    unsigned outGridW = CHROMASHIFT( m_uiFaceWidth, ch > 0 ? m_pcOutputFrame->getChromaWidthRatio() : 0 );
    unsigned outGridH = CHROMASHIFT( m_uiFaceHeight, ch > 0 ? m_pcOutputFrame->getChromaHeightRatio() : 0 );

    Mat outputFrame;
    m_pcOutputFrame->toMat( outputFrame, true, false, ch );

    for( int face = 0; face < m_cOutputFPSStruct.numFaces; face++ )
    {
      ClpFaceProperty outFp = m_cOutputFPSStruct.faces[face];
      Mat outputFace( outGridW * outFp.ratio / 100, outGridH * outFp.ratio / 100, CV_8UC1 );
      Rect outRect( Point( outFp.x * outGridW, outFp.y * outGridH ), outputFace.size() );

      int inFace = face;
      for( int i = 0; i < m_cInputFPSStruct.numFaces; i++ )
      {
        if( m_cInputFPSStruct.faces[i].id == outFp.id )
        {
          inFace = i;
          break;
        }
      }
      ClpFaceProperty inFp = m_cInputFPSStruct.faces[inFace];
      Mat inputFace( inGridW * inFp.ratio / 100, inGridH * inFp.ratio / 100, CV_8UC1 );
      Rect inRect( Point( inFp.x * inGridW, inFp.y * inGridH ), inputFace.size() );
      inputFrame( inRect ).copyTo( inputFace );

      cv::resize( inputFace, outputFace, outputFace.size(), 0, 0, cv::INTER_LANCZOS4 );

      outputFace.copyTo( outputFrame( outRect ) );
    }
    m_pcOutputFrame->fromMat( outputFrame, ch );
  }

  return m_pcOutputFrame;
}

void ThreeSixtyDynamicDownsampling::destroy()
{
  if( m_pcOutputFrame )
    delete m_pcOutputFrame;
  m_pcOutputFrame = NULL;
}

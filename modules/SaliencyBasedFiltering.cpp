/*    This file is a part of plaYUVer project
 *    Copyright (C) 2014-2015  by Luis Lucas      (luisfrlucas@gmail.com)
 *                                Joao Carreira   (jfmcarreira@gmail.com)
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
 * \file     SaliencyBasedFiltering.cpp
 */

#include "SaliencyBasedFiltering.h"

#include <algorithm>
#include <cmath>
#include <opencv2/opencv.hpp>

#define CLAMP_RANGE( X, MIN, MAX ) ( ( X ) < MIN ? MIN : ( ( X ) > MAX ? MAX : X ) )

SaliencyBasedFiltering::SaliencyBasedFiltering()
{
  /* Module Definition */
  m_iModuleAPI = MODULE_API_2;
  m_iModuleType = FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "Quality";
  m_pchModuleName = "SaliencyBasedFiltering";
  m_pchModuleTooltip = "Image filter based on a map";
  m_uiModuleRequirements = MODULE_REQUIRES_NOTHING;
  m_uiNumberOfFrames = MODULE_REQUIRES_TWO_FRAMES;

  m_pcProcessedFrame = NULL;
}

Bool SaliencyBasedFiltering::create( std::vector<PlaYUVerFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_

  for( UInt i = 0; i < apcFrameList.size(); i++ )
  {
    if( !apcFrameList[i]->haveSameFmt( apcFrameList[0], PlaYUVerFrame::MATCH_RESOLUTION ) )
      return false;
  }
  if( apcFrameList[1]->getNumberChannels() != 1 )
  {
    return false;
  }
  m_pcProcessedFrame = new PlaYUVerFrame( apcFrameList[0] );
  return true;
}

static cv::Mat* getCvFrame( Pel* pInputPel, UInt uiWidth, UInt uiHeight )
{
  cv::Mat* cvInput = new cv::Mat( uiHeight, uiWidth, CV_MAKETYPE( CV_8U, 1 ) );
  UChar* pCvPel = cvInput->data;
  for( UInt i = 0; i < uiWidth * uiHeight; i++ )
  {
    *pCvPel++ = *pInputPel++;
  }
  return cvInput;
}

static cv::Mat* filterframe( cv::Mat* cvFrame, UInt kernelSize )
{
  cv::Mat* cvFiltered = new cv::Mat( cvFrame->rows, cvFrame->cols, CV_MAKETYPE( CV_8U, 1 ) );
  /// Initialize arguments for the filter
  cv::Point anchor = cv::Point( -1, -1 );
  Double delta = 0;
  Int ddepth = -1;

  /// Update kernel size for a normalized box filter
  cv::Mat kernel = cv::Mat::ones( kernelSize, kernelSize, CV_32F ) / (Double)( kernelSize * kernelSize );

  /// Apply filter
  cv::filter2D( *cvFrame, *cvFiltered, ddepth, kernel, anchor, delta, cv::BORDER_DEFAULT );

  return cvFiltered;
}

static void filterComponent( PlaYUVerFrame* pInput, PlaYUVerFrame* pOutput, PlaYUVerFrame* Map, UInt uiComp )
{
  Pel* pMapPel = Map->getPelBufferYUV()[0][0];
  Int iMapStep = 1;
  Pel* pInputPel = pInput->getPelBufferYUV()[uiComp][0];
  Pel* pOutputPel = pOutput->getPelBufferYUV()[uiComp][0];
  Int iHeight = pInput->getHeight();
  Int iWidth = pInput->getWidth();

  if( uiComp > 0 )
  {
    iHeight = pInput->getChromaHeight();
    iWidth = pInput->getChromaWidth();
    iMapStep = 2;
  }

  cv::Mat* cvInput = getCvFrame( pInputPel, iWidth, iHeight );

  UInt numberFilters = 1;
  UChar* pCvPel[3];
  cv::Mat* cvOutput[3];
  for( UInt i = 0; i < numberFilters; i++ )
  {
    cvOutput[i] = filterframe( cvInput, ( 1 << ( 2 * i + 7 ) ) + 1 );
    pCvPel[i] = cvOutput[i]->data;
  }

  Int iCheckNeighbour = 8;
  UInt uiRelevance = 0;
  for( Int y = 0; y < iHeight; y++ )
  {
    for( Int x = 0; x < iWidth; x++ )
    {
      uiRelevance = 0;
      //      uiRelevance = *pMapPel;
      for( Int j = CLAMP_RANGE( y - iCheckNeighbour, 0, iHeight ); j < CLAMP_RANGE( y + iCheckNeighbour + 1, 0, iHeight ); j++ )
      {
        for( Int i = CLAMP_RANGE( x - iCheckNeighbour, 0, iWidth ); i < CLAMP_RANGE( x + iCheckNeighbour + 1, 0, iWidth ); i++ )
        {
          uiRelevance += *( pMapPel + ( ( j - y ) * iWidth ) + ( i - x ) );
        }
      }

      if( uiRelevance > 0 )
      {
        *pOutputPel = *pInputPel;
      }
      else
      {
        *pOutputPel = *pCvPel[0];
        //*pOutputPel = 0;
      }

      for( UInt i = 0; i < numberFilters; i++ )
        pCvPel[i]++;
      pInputPel++;
      pOutputPel++;
      pMapPel += iMapStep;
    }
    pMapPel += ( iMapStep - 1 ) * iWidth;
  }
}

PlaYUVerFrame* SaliencyBasedFiltering::process( std::vector<PlaYUVerFrame*> apcFrameList )
{
  PlaYUVerFrame* pcInputFrame = apcFrameList[0];

  for( UInt c = 0; c < pcInputFrame->getNumberChannels(); c++ )
    filterComponent( pcInputFrame, m_pcProcessedFrame, apcFrameList[1], c );

  return m_pcProcessedFrame;
}

Void SaliencyBasedFiltering::destroy()
{
}

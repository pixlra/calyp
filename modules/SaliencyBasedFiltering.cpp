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
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "Quality";
  m_pchModuleName = "SaliencyBasedFiltering";
  m_pchModuleLongName = "Saliency Based Filtering";
  m_pchModuleTooltip = "Image filter based on a map";
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_NOTHING;
  m_uiNumberOfFrames = 2;

  m_pcProcessedFrame = NULL;
}

bool SaliencyBasedFiltering::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_

  for( unsigned i = 0; i < apcFrameList.size(); i++ )
  {
    if( !apcFrameList[i]->haveSameFmt( apcFrameList[0], CalypFrame::MATCH_RESOLUTION ) )
      return false;
  }
  if( apcFrameList[1]->getNumberChannels() != 1 )
  {
    return false;
  }

  m_pcProcessedFrame = std::make_unique<CalypFrame>( apcFrameList[0]->getWidth(),
                                                     apcFrameList[0]->getHeight(),
                                                     apcFrameList[0]->getPelFormat(),
                                                     apcFrameList[0]->getBitsPel(),
                                                     apcFrameList[0]->getHasNegativeValues() );

  return true;
}

static cv::Mat* getCvFrame( ClpPel* pInputPel, unsigned uiWidth, unsigned uiHeight )
{
  cv::Mat* cvInput = new cv::Mat( uiHeight, uiWidth, CV_MAKETYPE( CV_8U, 1 ) );
  unsigned char* pCvPel = cvInput->data;
  for( unsigned i = 0; i < uiWidth * uiHeight; i++ )
  {
    *pCvPel++ = *pInputPel++;
  }
  return cvInput;
}

static cv::Mat* filterframe( cv::Mat* cvFrame, unsigned kernelSize )
{
  cv::Mat* cvFiltered = new cv::Mat( cvFrame->rows, cvFrame->cols, CV_MAKETYPE( CV_8U, 1 ) );
  /// Initialize arguments for the filter
  cv::Point anchor = cv::Point( -1, -1 );
  double delta = 0;
  int ddepth = -1;

  /// Update kernel size for a normalized box filter
  cv::Mat kernel = cv::Mat::ones( kernelSize, kernelSize, CV_32F ) / static_cast<double>( kernelSize * kernelSize );

  /// Apply filter
  cv::filter2D( *cvFrame, *cvFiltered, ddepth, kernel, anchor, delta, cv::BORDER_DEFAULT );

  return cvFiltered;
}

static void filterComponent( CalypFrame* pInput, CalypFrame* pOutput, CalypFrame* Map, unsigned uiComp )
{
  ClpPel* pMapPel = Map->getPelBufferYUV()[0][0];
  int iMapStep = 1;
  ClpPel* pInputPel = pInput->getPelBufferYUV()[uiComp][0];
  ClpPel* pOutputPel = pOutput->getPelBufferYUV()[uiComp][0];
  std::int64_t iHeight = pInput->getHeight( uiComp );
  std::int64_t iWidth = pInput->getWidth( uiComp );

  if( uiComp > 0 )
  {
    iMapStep = 2;
  }

  cv::Mat* cvInput = getCvFrame( pInputPel, iWidth, iHeight );

  unsigned numberFilters = 1;
  unsigned char* pCvPel[3];
  cv::Mat* cvOutput[3];
  for( unsigned i = 0; i < numberFilters; i++ )
  {
    cvOutput[i] = filterframe( cvInput, ( 1 << ( 2 * i + 7 ) ) + 1 );
    pCvPel[i] = cvOutput[i]->data;
  }

  int iCheckNeighbour = 8;
  unsigned uiRelevance = 0;
  for( int y = 0; y < iHeight; y++ )
  {
    for( int x = 0; x < iWidth; x++ )
    {
      uiRelevance = 0;
      //      uiRelevance = *pMapPel;
      for( std::int64_t j = CLAMP_RANGE( y - iCheckNeighbour, 0, iHeight ); j < CLAMP_RANGE( y + iCheckNeighbour + 1, 0, iHeight ); j++ )
      {
        for( std::int64_t i = CLAMP_RANGE( x - iCheckNeighbour, 0, iWidth ); i < CLAMP_RANGE( x + iCheckNeighbour + 1, 0, iWidth ); i++ )
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

      for( unsigned i = 0; i < numberFilters; i++ )
        pCvPel[i]++;
      pInputPel++;
      pOutputPel++;
      pMapPel += iMapStep;
    }
    pMapPel += ( iMapStep - 1 ) * iWidth;
  }
}

CalypFrame* SaliencyBasedFiltering::process( std::vector<CalypFrame*> apcFrameList )
{
  CalypFrame* pcInputFrame = apcFrameList[0];

  for( unsigned c = 0; c < pcInputFrame->getNumberChannels(); c++ )
    filterComponent( pcInputFrame, m_pcProcessedFrame.get(), apcFrameList[1], c );

  return m_pcProcessedFrame.get();
}

void SaliencyBasedFiltering::destroy()
{
}

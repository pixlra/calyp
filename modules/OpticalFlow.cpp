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
 * \file     OpticalFlow.cpp
 * \brief    Modules to measure optical flow
 */

#include "OpticalFlow.h"

#include <cstdlib>
#include <opencv2/optflow.hpp>

using cv::Mat;
using cv::Mat_;
using cv::Point;
using cv::Point2f;
using cv::Ptr;
using cv::Rect;
using cv::Scalar;

OpticalFlowModule::OpticalFlowModule()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = ClpModuleType::FrameProcessing;
  m_pchModuleCategory = "OpticalFlow";
  m_uiNumberOfFrames = 2;
  m_uiModuleRequirements = ClpModuleFeature::SkipWhilePlaying | ClpModuleFeature::NewWindow | ClpModuleFeature::Options;

  m_cModuleOptions.addOptions() /**/
      ( "Show reconstruction", m_bShowReconstruction, "Show reconstructed frame instead of MVs [false]" );

  m_bShowReconstruction = false;
  m_pcOutputFrame = NULL;
}

bool OpticalFlowModule::commonCreate( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_

  for( unsigned int i = 1; i < apcFrameList.size(); i++ )
    if( !apcFrameList[i]->haveSameFmt( apcFrameList[0], CalypFrame::MATCH_COLOR_SPACE |
                                                            CalypFrame::MATCH_RESOLUTION |
                                                            CalypFrame::MATCH_BITS ) )
      return false;

  m_iStep = 16;
  m_pcOutputFrame = new CalypFrame( apcFrameList[0]->getWidth(), apcFrameList[0]->getHeight(), ClpPixelFormats::CLP_GRAY );

  return true;
}

static inline bool isFlowCorrect( Point2f u )
{
  return !cvIsNaN( u.x ) && !cvIsNaN( u.y ) && fabs( u.x ) < 1e9 && fabs( u.y ) < 1e9;
}

void OpticalFlowModule::drawFlow()
{
  Mat cvMatAfter;
  m_pcFrameAfter->toMat( cvMatAfter, true );
  Scalar vectorColor( 255, 0, 0, 0 );
  for( int y = m_iStep / 2; y < m_cvFlow.rows; y += m_iStep )
  {
    for( int x = m_iStep / 2; x < m_cvFlow.cols; x += m_iStep )
    {
      Point2f u( 0, 0 );
      double count = 0;
      for( int i = ( -m_iStep / 2 ); i < ( m_iStep / 2 ); i++ )
      {
        for( int j = ( -m_iStep / 2 ); j < ( m_iStep / 2 ); j++ )
        {
          if( isFlowCorrect( u ) )
          {
            u = u + m_cvFlow( y, x );
            count++;
          }
        }
      }
      if( count > 0 )
      {
        Point p = Point( x, y );
        Point ui( u.x / count, u.y / count );
        arrowedLine( cvMatAfter, p, p + ui, vectorColor );
      }
    }
  }
  m_pcOutputFrame->fromMat( cvMatAfter );
}

void OpticalFlowModule::compensateFlow()
{
  Rect frameShape( 0, 0, m_pcOutputFrame->getWidth(), m_pcOutputFrame->getHeight() );
  m_pcOutputFrame->reset();
  for( unsigned int c = 0; c < m_pcOutputFrame->getNumberChannels(); c++ )
  {
    ClpPel** pPelPrev = m_pcFramePrev->getPelBufferYUV()[c];
    ClpPel* pPelOut = m_pcOutputFrame->getPelBufferYUV()[c][0];

    for( unsigned int y = 0; y < m_pcOutputFrame->getHeight( c ); y++ )
    {
      for( unsigned int x = 0; x < m_pcOutputFrame->getWidth( c ); x++ )
      {
        Point2f u = m_cvFlow( y, x );
        Point p( x + u.x, y + u.y );
        if( p.inside( frameShape ) )
          *pPelOut = pPelPrev[p.y][p.x];
        pPelOut++;
      }
    }
  }
}

CalypFrame* OpticalFlowModule::process( std::vector<CalypFrame*> apcFrameList )
{
  m_pcFrameAfter = apcFrameList[0];
  m_pcFramePrev = apcFrameList[1];

  Mat cvMatPrev, cvMatAfter;
  if( !m_pcFramePrev->toMat( cvMatPrev, true ) || !m_pcFrameAfter->toMat( cvMatAfter, true ) )
  {
    return m_pcOutputFrame;
  }

  m_cOpticalFlow->calc( cvMatAfter, cvMatPrev, m_cvFlow );
  if( m_bShowReconstruction )
    compensateFlow();
  else
    drawFlow();

  return m_pcOutputFrame;
}

void OpticalFlowModule::destroy()
{
  if( m_pcOutputFrame )
    delete m_pcOutputFrame;
  m_pcOutputFrame = NULL;
  //   if( m_cOpticalFlow )
  //     delete m_cOpticalFlow;
}

OpticalFlowDualTVL1::OpticalFlowDualTVL1()
{
  /* Module Definition */
  m_pchModuleName = "OpticalFlowDualTVL1";
  m_pchModuleLongName = "Dual TVL1";
  m_pchModuleTooltip = "Measure optical flow using DualTVL1 method";
}

bool OpticalFlowDualTVL1::create( std::vector<CalypFrame*> apcFrameList )
{
  bool bRet = commonCreate( apcFrameList );
  if( !bRet )
    return bRet;
#if CV_MAJOR_VERSION >= 4
  m_cOpticalFlow = cv::optflow::createOptFlow_DualTVL1();
#else
  m_cOpticalFlow = cv::createOptFlow_DualTVL1();
#endif
  return bRet;
}

OpticalFlowSparseToDense::OpticalFlowSparseToDense()
{
  /* Module Definition */
  m_pchModuleName = "OpticalFlowSparseToDense";
  m_pchModuleLongName = "Sparse to dense";
  m_pchModuleTooltip = "Measure optical flow using SparseToDense method";
}

bool OpticalFlowSparseToDense::create( std::vector<CalypFrame*> apcFrameList )
{
  bool bRet = commonCreate( apcFrameList );
  if( !bRet )
    return bRet;
  m_cOpticalFlow = cv::optflow::createOptFlow_SparseToDense();
  return bRet;
}

OpticalFlowFarneback::OpticalFlowFarneback()
{
  /* Module Definition */
  m_pchModuleName = "OpticalFlowFarneback";
  m_pchModuleLongName = "Farneback";
  m_pchModuleTooltip = "Measure optical flow using Farneback method";
}

bool OpticalFlowFarneback::create( std::vector<CalypFrame*> apcFrameList )
{
  bool bRet = commonCreate( apcFrameList );
  if( !bRet )
    return bRet;
  m_cOpticalFlow = cv::optflow::createOptFlow_Farneback();
  return bRet;
}

OpticalDeepFlow::OpticalDeepFlow()
{
  /* Module Definition */
  m_pchModuleName = "OpticalDeepFlow";
  m_pchModuleLongName = "Deep flow";
  m_pchModuleTooltip = "Measure optical flow using Farneback method";
}

bool OpticalDeepFlow::create( std::vector<CalypFrame*> apcFrameList )
{
  bool bRet = commonCreate( apcFrameList );
  if( !bRet )
    return bRet;
  m_cOpticalFlow = cv::optflow::createOptFlow_DeepFlow();
  return bRet;
}

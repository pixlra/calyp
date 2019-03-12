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
 * \file     ThreeSixtyDownsampling.cpp
 * \brief    Downsampling ERP to NxN
 */

#include "ThreeSixtyDownsampling.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>

using cv::InputArray;
using cv::Mat;
using cv::OutputArray;
using cv::Size;

ThreeSixtyDownsampling::ThreeSixtyDownsampling()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "360Video";
  m_pchModuleName = "ThreeSixtyDownsampling";
  m_pchModuleLongName = "ERP to NxN";
  m_pchModuleTooltip = "Downsampling ERP to NxN";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_SKIP_WHILE_PLAY | CLP_MODULE_REQUIRES_OPTIONS;

  m_cModuleOptions.addOptions() /**/
      ( "Downsampling", m_bDownsampling, "ERP to NxN [true]" );

  m_pcOutputFrame = NULL;
  m_bDownsampling = true;
}

bool ThreeSixtyDownsampling::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_
  m_pcOutputFrame = new CalypFrame( apcFrameList[0]->getWidth(), apcFrameList[0]->getHeight(), CLP_GRAY );
  m_isOdd = !( m_pcOutputFrame->getHeight() % 2 );
  return true;
}

CalypFrame* ThreeSixtyDownsampling::process( std::vector<CalypFrame*> apcFrameList )
{
  Mat cvIn;
  apcFrameList[0]->toMat( cvIn, true );
  m_pcOutputFrame->reset();

  memcpy( &m_pcOutputFrame->getPelBufferYUV()[0][m_pcOutputFrame->getHeight() / 2][0],
          &apcFrameList[0]->getPelBufferYUV()[0][m_pcOutputFrame->getHeight() / 2][0],
          m_pcOutputFrame->getWidth() * sizeof( ClpPel ) );
  if( m_isOdd )
  {
    memcpy( &m_pcOutputFrame->getPelBufferYUV()[0][m_pcOutputFrame->getHeight() / 2 - 1][0],
            &apcFrameList[0]->getPelBufferYUV()[0][m_pcOutputFrame->getHeight() / 2 - 1][0],
            m_pcOutputFrame->getWidth() * sizeof( ClpPel ) );
  }

  for( int line = 1; line < m_pcOutputFrame->getHeight() / 2; line++ )
  {
    for( int hemisfer = -1; hemisfer < 2; hemisfer += 2 )
    {
      Mat cvOutput;

      // calculate destination sizes
      unsigned int ypos = m_pcOutputFrame->getHeight() / 2 + line * hemisfer + ( hemisfer == -1 ? ( m_isOdd ? -1 : 0 ) : 0 );
      unsigned int lineWidth = ( m_pcOutputFrame->getHeight() / 2 - line ) * 4 + 2;
      unsigned int xpos = ( m_pcOutputFrame->getWidth() - lineWidth ) / 2;

      Mat cvDownsample( 1, lineWidth, CV_8UC1 );
      Mat cvUpsample( 1, m_pcOutputFrame->getWidth(), CV_8UC1 );

      // resize
      if( m_bDownsampling )
      {
        cv::resize( cvIn.row( ypos ), cvDownsample, cvDownsample.size() );
        cvOutput = cvDownsample;
      }
      else
      {
        Mat cvauxin = cvIn.row( ypos ).colRange( xpos, xpos + lineWidth );
        cvauxin.copyTo( cvDownsample );
        cv::resize( cvDownsample, cvUpsample, cvUpsample.size() );
        cvOutput = cvUpsample;
      }

      // copy to output
      ClpPel* outpel = &m_pcOutputFrame->getPelBufferYUV()[0][ypos][xpos * m_bDownsampling];
      unsigned char* pchar = cvOutput.data;
      for( unsigned int i = 0; i < cvOutput.cols; i++ )
      {
        *outpel++ = *pchar++;
      }
    }
  }
  return m_pcOutputFrame;
}

void ThreeSixtyDownsampling::destroy()
{
  if( m_pcOutputFrame )
    delete m_pcOutputFrame;
  m_pcOutputFrame = NULL;
}

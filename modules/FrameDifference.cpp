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
 * \file     FrameDifference.cpp
 * \brief    Frame Difference module
 */

#include "FrameDifference.h"

FrameDifference::FrameDifference()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "Measurements";
  m_pchModuleName = "FrameDifference";
  m_pchModuleName = "Difference";
  m_pchModuleTooltip = "Measure the difference between two images (Y plane),  "
                       "Y1 - Y2, with max absolute diff of 128";
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_NEW_WINDOW | CLP_MODULE_REQUIRES_OPTIONS;
  m_uiNumberOfFrames = 2;

  m_cModuleOptions.addOptions()                                                           /**/
      ( "BitsPerpixel", m_uiBitsPixel, "Bits per pixel (use zero to avoid scaling) [0]" ) /**/
      ( "SumOperation", m_uiSumOperation, "Sum instead of difference [0]" );

  m_uiBitsPixel = 0;
  m_uiSumOperation = 0;
  m_iOperation = 1;

  m_pcFrameDifference = NULL;
}

bool FrameDifference::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_

  m_uiSumOperation = m_uiSumOperation > 1 ? 0 : m_uiSumOperation;

  unsigned int uiMaxBitsPixel = 0;
  for( unsigned int i = 0; i < apcFrameList.size(); i++ )
  {
    if( !apcFrameList[i]->haveSameFmt( apcFrameList[0], CalypFrame::MATCH_PEL_FMT | CalypFrame::MATCH_COLOR_SPACE | CalypFrame::MATCH_RESOLUTION ) )
      return false;
    if( apcFrameList[i]->getBitsPel() > uiMaxBitsPixel )
    {
      uiMaxBitsPixel = apcFrameList[i]->getBitsPel();
      if( apcFrameList[i]->getHasNegativeValues() )
        uiMaxBitsPixel -= 1;
    }
  }

  uiMaxBitsPixel = m_uiSumOperation ? uiMaxBitsPixel : uiMaxBitsPixel + 1;
  m_uiBitsPixel = !m_uiBitsPixel ? uiMaxBitsPixel & 0x0F : m_uiBitsPixel;

  m_iDiffBitShift = uiMaxBitsPixel - m_uiBitsPixel;
  m_iMaxDiffValue = ( 1 << ( m_uiSumOperation ? m_uiBitsPixel : m_uiBitsPixel - 1 ) );

  m_pcFrameDifference = new CalypFrame( apcFrameList[0]->getWidth(), apcFrameList[0]->getHeight(),
                                        apcFrameList[0]->getPelFormat(), m_uiBitsPixel, m_uiSumOperation == 1 ? false : true );

  m_iOperation = m_uiSumOperation == 0 ? -1 : 1;
  return true;
}

CalypFrame* FrameDifference::process( std::vector<CalypFrame*> apcFrameList )
{
  const CalypFrame& frame1 = *apcFrameList[0];
  const CalypFrame& frame2 = *apcFrameList[1];
  ClpPel* pOutputPelYUV = m_pcFrameDifference->getPelBufferYUV()[0][0];
  short aux_pel_1, aux_pel_2;
  short diff = 0;

  for( unsigned int ch = 0; ch < m_pcFrameDifference->getNumberChannels(); ch++ )
    for( unsigned int y = 0; y < m_pcFrameDifference->getHeight( ch ); y++ )
      for( unsigned int x = 0; x < m_pcFrameDifference->getWidth( ch ); x++ )
      {
        aux_pel_1 = frame1( ch, x, y, false );
        aux_pel_2 = frame2( ch, x, y, false );
        diff = aux_pel_1 + m_iOperation * aux_pel_2;
        diff = m_iDiffBitShift > 0 ? diff >> m_iDiffBitShift : diff << -m_iDiffBitShift;
        diff = std::min<short>( diff, m_iMaxDiffValue - 1 );
        if( m_uiSumOperation == 0 )
        {
          diff = std::max<short>( diff, -m_iMaxDiffValue );
          diff += m_iMaxDiffValue;
        }
        *pOutputPelYUV++ = diff;
      }
  return m_pcFrameDifference;
}

void FrameDifference::destroy()
{
  if( m_pcFrameDifference )
    delete m_pcFrameDifference;
  m_pcFrameDifference = NULL;
}

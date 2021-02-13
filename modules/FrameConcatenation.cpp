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
 * \file     FrameConcatenation.cpp
 * \brief    Concatenate two frames side-by-side
 */

#include "FrameConcatenation.h"

FrameConcatenation::FrameConcatenation()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "Utilities";
  m_pchModuleLongName = "Frame Concatenation";
  m_pchModuleName = "FrameConc";
  m_pchModuleTooltip = "Concatenate frames side-by-side";
  m_uiNumberOfFrames = 2;
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_OPTIONS | CLP_MODULE_REQUIRES_NEW_WINDOW;

  m_cModuleOptions.addOptions() /**/
      ( "ShiftHorizontal", m_iShiftHor, "Amount of pixels to shift in horizontal direction" );

  m_pcProcessedFrame = NULL;
  m_iShiftHor = 64;
}

bool FrameConcatenation::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_

  for( unsigned int i = 1; i < apcFrameList.size(); i++ )
  {
    if( !apcFrameList[i]->haveSameFmt( apcFrameList[0], CalypFrame::MATCH_COLOR_SPACE |
                                                            CalypFrame::MATCH_RESOLUTION |
                                                            CalypFrame::MATCH_BITS ) )
    {
      return false;
    }
  }

  unsigned int newWidth = apcFrameList[0]->getWidth() * 2 + m_iShiftHor;
  m_pcProcessedFrame = new CalypFrame( newWidth, apcFrameList[0]->getHeight(), apcFrameList[0]->getPelFormat(), apcFrameList[0]->getBitsPel() );
  return true;
}

CalypFrame* FrameConcatenation::process( std::vector<CalypFrame*> apcFrameList )
{
  ClpPel* pelLeft;
  ClpPel* pelRight;
  ClpPel* pelout;

  m_pcProcessedFrame->reset();

  for( unsigned int ch = 0; ch < m_pcProcessedFrame->getNumberChannels(); ch++ )
  {
    unsigned int uiHeight = apcFrameList[0]->getHeight( ch );
    unsigned int uiWidth = apcFrameList[0]->getWidth( ch );
    unsigned int shift = ch == 0 ? m_iShiftHor : m_iShiftHor >> m_pcProcessedFrame->getChromaWidthRatio();

    for( unsigned int y = 0; y < uiHeight; y++ )
    {
      pelLeft = apcFrameList[0]->getPelBufferYUV()[ch][y];
      pelRight = apcFrameList[1]->getPelBufferYUV()[ch][y];
      pelout = m_pcProcessedFrame->getPelBufferYUV()[ch][y];

      for( unsigned int x = 0; x < uiWidth; x++ )
      {
        *pelout = *pelLeft++;
        *( pelout + uiWidth + shift ) = *pelRight++;
        pelout++;
      }
    }
  }
  return m_pcProcessedFrame;
}

void FrameConcatenation::destroy()
{
  if( m_pcProcessedFrame )
    delete m_pcProcessedFrame;
  m_pcProcessedFrame = NULL;
}

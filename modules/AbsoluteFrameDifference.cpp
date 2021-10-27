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
 * \file     AbsoluteFrameDifference.cpp
 * \brief    Absolute Frame Difference module
 */

#include "AbsoluteFrameDifference.h"

AbsoluteFrameDifference::AbsoluteFrameDifference()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;  // Use API version 2 (recommended).
  // See this example for details on the functions prototype
  m_iModuleType = ClpModuleType::FrameProcessing;          // Apply module to the frames or to
                                                           // the whole sequence.
  m_pchModuleCategory = "Measurements";                    // Category (sub-menu)
  m_pchModuleName = "AbsoluteFrameDifference";             // Name (no spaces)
  m_pchModuleLongName = "Absolute Difference";             // Long Name
  m_pchModuleTooltip = "Measure the absolute difference "  // Description
                       "between two images (Y plane), e. g., abs( Y1 - Y2 )";
  m_uiNumberOfFrames = 2;                                // Number of frames required
  m_uiModuleRequirements = ClpModuleFeature::NewWindow;  // Module requirements
                                                         // (check
                                                         // CalypModulesIf.h).
  // Several requirements should be "or" between each others.
  m_pcFrameDifference = nullptr;
}

bool AbsoluteFrameDifference::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_

  for( unsigned int i = 1; i < apcFrameList.size(); i++ )
    if( !apcFrameList[i]->haveSameFmt( apcFrameList[0], CalypFrame::MATCH_COLOR_SPACE_IGNORE_GRAY | CalypFrame::MATCH_COLOR_SPACE | CalypFrame::MATCH_RESOLUTION | CalypFrame::MATCH_BITS ) )
      return false;

  m_pcFrameDifference = std::make_unique<CalypFrame>( apcFrameList[0]->getWidth(), apcFrameList[0]->getHeight(), ClpPixelFormats::Gray, apcFrameList[0]->getBitsPel() );
  return true;
}

CalypFrame* AbsoluteFrameDifference::process( std::vector<CalypFrame*> apcFrameList )
{
  CalypFrame* Input1 = apcFrameList[0];
  CalypFrame* Input2 = apcFrameList[1];
  ClpPel* pInput1PelYUV = Input1->getPelBufferYUV()[0][0];
  ClpPel* pInput2PelYUV = Input2->getPelBufferYUV()[0][0];
  ClpPel* pOutputPelYUV = m_pcFrameDifference->getPelBufferYUV()[0][0];
  int aux_pel_1, aux_pel_2;

  for( unsigned int ch = 0; ch < m_pcFrameDifference->getNumberChannels(); ch++ )
  {
    for( unsigned int y = 0; y < m_pcFrameDifference->getHeight(); y++ )
    {
      for( unsigned int x = 0; x < m_pcFrameDifference->getWidth(); x++ )
      {
        aux_pel_1 = *pInput1PelYUV++;
        aux_pel_2 = *pInput2PelYUV++;
        *pOutputPelYUV++ = abs( aux_pel_1 - aux_pel_2 );
      }
    }
  }
  return m_pcFrameDifference.get();
}

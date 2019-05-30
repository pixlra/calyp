/*    This file is a part of Calyp project
 *    Copyright (C) 2014-2019  by Joao Carreira   (jfmcarreira@gmail.com)
 *                                Joao Santos     (joaompssantos@gmail.com)
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
 * \file     OptimiseDisplay.cpp
 * \brief    Optimises the display of images with sparse histograms
 */

#include "OptimiseDisplay.h"

#include <cmath>

OptimiseDisplay::OptimiseDisplay()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;  // Use API version 2 (recommended).
  // See this example for details on the functions prototype
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;          // Apply module to the frames or to
                                                        // the whole sequence.
  m_pchModuleCategory = "Conversions";                  // Category (sub-menu)
  m_pchModuleName = "OptimiseDisplay";                  // Name (no spaces)
  m_pchModuleLongName = "Optimise Display";             // Long Name
  m_pchModuleTooltip = "Scales the display of images "  // Description
                       "for better visualization of small differences";
  m_uiNumberOfFrames = 1;                                   // Number of frames required
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_NEW_WINDOW;  // Module requirements
                                                            // (check
                                                            // CalypModulesIf.h).
                                                            // Several requirements should be "or" between each others.
  m_pcOptimisedFrame = NULL;
}

bool OptimiseDisplay::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_

  for( unsigned int i = 1; i < apcFrameList.size(); i++ )
    if( !apcFrameList[i]->haveSameFmt( apcFrameList[0], CalypFrame::MATCH_COLOR_SPACE_IGNORE_GRAY | CalypFrame::MATCH_COLOR_SPACE | CalypFrame::MATCH_RESOLUTION | CalypFrame::MATCH_BITS ) )
      return false;

  m_pcOptimisedFrame = new CalypFrame( apcFrameList[0]->getWidth(), apcFrameList[0]->getHeight(), CLP_LUMA, 16 );
  m_pcOptimisedFrame->reset();

  return true;
}

CalypFrame* OptimiseDisplay::process( std::vector<CalypFrame*> apcFrameList )
{
  unsigned int numValues = (int)pow( 2, apcFrameList[0]->getBitsPel() ), scale, usedValues = 0;
  int* lookUpTable = (int*)malloc( sizeof( int ) * numValues );
  const ClpPel* pInput1PelYUV = apcFrameList[0]->getPelBufferYUV()[0][0];
  ClpPel* pOutputPelYUV = m_pcOptimisedFrame->getPelBufferYUV()[0][0];

  apcFrameList[0]->calcHistogram();
  m_pcOptimisedFrame->reset();

  for( unsigned int ch = 0; ch < apcFrameList[0]->getNumberChannels(); ch++ )
  {
    for( unsigned int b = 0; b < 1u << apcFrameList[0]->getBitsPel(); b++ )
    {
      if( apcFrameList[0]->getHistogramValue( ch, b ) != 0 )
      {
        lookUpTable[b] = usedValues;
        usedValues++;
      }
      else
      {
        lookUpTable[b] = -1;
      }
    }

    scale = 65536 / usedValues;
    usedValues = 0;

    for( unsigned int y = 0; y < m_pcOptimisedFrame->getHeight( ch ); y++ )
    {
      for( unsigned int x = 0; x < m_pcOptimisedFrame->getWidth( ch ); x++ )
      {
        *pOutputPelYUV++ = lookUpTable[*pInput1PelYUV++] * scale;
      }
    }
  }

  free( lookUpTable );

  return m_pcOptimisedFrame;
}

void OptimiseDisplay::destroy()
{
  if( m_pcOptimisedFrame )
    delete m_pcOptimisedFrame;
  m_pcOptimisedFrame = NULL;
}

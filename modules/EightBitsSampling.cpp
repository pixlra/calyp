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
 * \file     EightBitsSampling.cpp
 * \brief    Binarize frame module
 */

#include "EightBitsSampling.h"

EightBitsSampling::EightBitsSampling()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "Conversions";
  m_pchModuleName = "BitsResampling";
  m_pchModuleLongName = "Re-sampling frame (bpp)";
  m_pchModuleTooltip = "Re-sampling frame to a different value of bits per pixel";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_OPTIONS;

  m_cModuleOptions.addOptions() /**/
      ( "num_bits", m_iNumberOfBits, "Number of bits/pixel (8-16) [8]" );

  m_iNumberOfBits = 8;
  m_pcResampledFrame = NULL;
}

bool EightBitsSampling::create( std::vector<CalypFrame*> apcFrameList )
{
  m_iBitSifting = int( apcFrameList[0]->getBitsPel() ) - m_iNumberOfBits;
  if( !m_iBitSifting )
    return false;

  m_pcResampledFrame = new CalypFrame( apcFrameList[0]->getWidth(), apcFrameList[0]->getHeight(),
                                       apcFrameList[0]->getPelFormat(), m_iNumberOfBits );
  return true;
}

CalypFrame* EightBitsSampling::process( std::vector<CalypFrame*> apcFrameList )
{
  CalypFrame* pcFrame = apcFrameList[0];
  ClpPel* pPelInput = pcFrame->getPelBufferYUV()[0][0];
  ClpPel* pPelResampled = m_pcResampledFrame->getPelBufferYUV()[0][0];
  int bitShifting = m_iBitSifting > 0 ? m_iBitSifting : -m_iBitSifting;

  if( m_iBitSifting > 0 )
    for( unsigned i = 0; i < pcFrame->getTotalNumberOfPixels(); i++ )
      *pPelResampled++ = *pPelInput++ >> bitShifting;
  else
    for( unsigned i = 0; i < pcFrame->getTotalNumberOfPixels(); i++ )
      *pPelResampled++ = *pPelInput++ << bitShifting;
  return m_pcResampledFrame;
}

void EightBitsSampling::destroy()
{
  if( m_pcResampledFrame )
    delete m_pcResampledFrame;
  m_pcResampledFrame = NULL;
}

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
 * \file     ThreeSixtySpatialtoTemporal.cpp
 * \brief    Convert 360 videos from spatial to temporal
 */

#include "ThreeSixtySpatialtoTemporal.h"

ThreeSixtySpatialtoTemporal::ThreeSixtySpatialtoTemporal()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_3;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "360Video";
  m_pchModuleLongName = "Spatial to Temporal";
  m_pchModuleName = "SpatialtoTemporal";
  m_pchModuleTooltip = "Convert 360 video from Cube Map Projection to temporal frames";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_SKIP_WHILE_PLAY;

  m_uiFacesX = 3;
  m_uiFacesY = 2;

  flush();
}

bool ThreeSixtySpatialtoTemporal::flush()
{
  CalypModuleIf::flush();
  m_uiCopyX = 0;
  m_uiCopyY = 0;
  return true;
};

bool ThreeSixtySpatialtoTemporal::needFrame()
{
  if( m_iFrameBufferCount <= 0 )
  {
    return true;
  }
  return false;
};

bool ThreeSixtySpatialtoTemporal::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_

  unsigned int iWidth;
  unsigned int iHeight;

  iWidth = apcFrameList[0]->getWidth() / m_uiFacesX;
  iHeight = apcFrameList[0]->getHeight() / m_uiFacesY;

  m_pcOutputFrame = new CalypFrame( iWidth, iHeight, apcFrameList[0]->getPelFormat(),
                                    apcFrameList[0]->getBitsPel() );

  m_pcTmpFrame = new CalypFrame( apcFrameList[0]->getWidth(), apcFrameList[0]->getHeight(), apcFrameList[0]->getPelFormat(),
                                 apcFrameList[0]->getBitsPel() );
  return true;
}

CalypFrame* ThreeSixtySpatialtoTemporal::process( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_3_BREAK_CONDITION_

  if( m_iFrameBufferCount == 0 )
  {
    m_pcTmpFrame->copyFrom( apcFrameList[0], 0, 0 );
    m_uiCopyX = 0;
    m_uiCopyY = 0;

    m_iFrameBufferCount = m_uiFacesX * m_uiFacesY;
  }

  m_pcOutputFrame->copyFrom( m_pcTmpFrame, m_uiCopyX, m_uiCopyY );
  m_iFrameBufferCount--;

  m_uiCopyX += m_pcOutputFrame->getWidth();
  if( m_uiCopyX >= m_pcTmpFrame->getWidth() )
  {
    m_uiCopyX = 0;
    m_uiCopyY += m_pcOutputFrame->getHeight();
  }

  return m_pcOutputFrame;
}

void ThreeSixtySpatialtoTemporal::destroy()
{
  if( m_pcOutputFrame )
    delete m_pcOutputFrame;
  m_pcOutputFrame = NULL;
  if( m_pcTmpFrame )
    delete m_pcTmpFrame;
  m_pcTmpFrame = NULL;
}

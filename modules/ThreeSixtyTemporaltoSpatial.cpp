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
 * \file     ThreeSixtyTemporaltoSpatial.cpp
 * \brief    Convert 360 videos from temporal to spatial
 */

#include "ThreeSixtyTemporaltoSpatial.h"

ThreeSixtyTemporaltoSpatial::ThreeSixtyTemporaltoSpatial()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_3;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "360Video";
  m_pchModuleLongName = "Temporal to Spatial";
  m_pchModuleName = "TemporaltoSpatial";
  m_pchModuleTooltip = "Convert 360 video from temporal frames to Cube Map Projection";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_SKIP_WHILE_PLAY;

  m_uiFacesX = 3;
  m_uiFacesY = 2;

  flush();
}

bool ThreeSixtyTemporaltoSpatial::flush()
{
  CalypModuleIf::flush();
  m_uiCopyX = 0;
  m_uiCopyY = 0;
  m_uiFacesCount = 0;
  return true;
};

bool ThreeSixtyTemporaltoSpatial::needFrame()
{
  return CalypModuleIf::needFrame();
};

bool ThreeSixtyTemporaltoSpatial::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_

  unsigned int iWidth;
  unsigned int iHeight;

  iWidth = apcFrameList[0]->getWidth() * m_uiFacesX;
  iHeight = apcFrameList[0]->getHeight() * m_uiFacesY;

  m_pcOutputFrame = new CalypFrame( iWidth, iHeight, apcFrameList[0]->getPelFormat(),
                                       apcFrameList[0]->getBitsPel() );
  return true;
}

CalypFrame* ThreeSixtyTemporaltoSpatial::process( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_3_BREAK_CONDITION_

  m_pcOutputFrame->copyTo( apcFrameList[0], m_uiCopyX, m_uiCopyY );

  m_uiCopyX += apcFrameList[0]->getWidth();
  if( m_uiCopyX >= m_pcOutputFrame->getWidth() )
  {
    m_uiCopyX = 0;
    m_uiCopyY += apcFrameList[0]->getHeight();
  }
  if( ++m_uiFacesCount < (m_uiFacesX * m_uiFacesY) )
  {
    return NULL;
  }
  m_uiCopyX = 0;
  m_uiCopyY = 0;
  m_uiFacesCount = 0;
  return m_pcOutputFrame;
}

void ThreeSixtyTemporaltoSpatial::destroy()
{
  if( m_pcOutputFrame )
    delete m_pcOutputFrame;
  m_pcOutputFrame = NULL;
}

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
  m_pchModuleName = "ThreeSixtySpatialtoTemporal";
  m_pchModuleTooltip = "Convert 360 video from Cube Map Projection to temporal frames";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_SKIP_WHILE_PLAY | CLP_MODULE_REQUIRES_OPTIONS;

  m_cModuleOptions.addOptions()                                                    /**/
      ( "spa2temp", m_uiSpatial2Temporal, "Convert from spatial to temporal [1]" ) /**/
      ( "facesX", m_uiFacesX, "Number of horizontal faces[3]" )                    /**/
      ( "facesY", m_uiFacesY, "Number of vertical faces[2]" )                      /**/
      ( "facesFrame", m_uiFacesPerFrame, "Faces per frame [1]" );

  m_uiSpatial2Temporal = true;
  m_uiFacesX = 3;
  m_uiFacesY = 2;
  m_uiFacesPerFrame = 1;

  flush();
  m_pcTmpInputFrame = NULL;
  m_pcTmpFace = NULL;
}

bool ThreeSixtySpatialtoTemporal::flush()
{
  CalypModuleIf::flush();
  m_uiCopyX = 0;
  m_uiCopyY = 0;
  m_uiFacesCount = 0;
  return true;
};

bool ThreeSixtySpatialtoTemporal::needFrame()
{
  if( m_uiSpatial2Temporal )
  {
    if( m_iFrameBufferCount <= 0 )
      return true;
  }
  else
  {
    return CalypModuleIf::needFrame();
  }
  return false;
}

bool ThreeSixtySpatialtoTemporal::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_

  unsigned int iWidth;
  unsigned int iHeight;

  if( m_uiSpatial2Temporal )
  {
    if( apcFrameList[0]->getWidth() % m_uiFacesX || apcFrameList[0]->getHeight() % m_uiFacesY )
      return false;

    if( m_uiFacesPerFrame > m_uiFacesX * m_uiFacesY )
      return false;

    iWidth = apcFrameList[0]->getWidth() / m_uiFacesX;
    iHeight = apcFrameList[0]->getHeight() / m_uiFacesY;

    m_pcTmpInputFrame = new CalypFrame( apcFrameList[0]->getWidth(), apcFrameList[0]->getHeight(), apcFrameList[0]->getPelFormat(),
                                        apcFrameList[0]->getBitsPel() );
    m_pcTmpFace = new CalypFrame( iWidth, iHeight, apcFrameList[0]->getPelFormat(),
                                  apcFrameList[0]->getBitsPel() );
    iWidth *= m_uiFacesPerFrame;
  }
  else
  {
    iWidth = apcFrameList[0]->getWidth() * m_uiFacesX / m_uiFacesPerFrame;
    iHeight = apcFrameList[0]->getHeight() * m_uiFacesY;
  }
  m_pcOutputFrame = new CalypFrame( iWidth, iHeight, apcFrameList[0]->getPelFormat(),
                                    apcFrameList[0]->getBitsPel() );
  return true;
}

CalypFrame* ThreeSixtySpatialtoTemporal::process( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_3_BREAK_CONDITION_

  if( m_uiSpatial2Temporal )
  {
    if( m_iFrameBufferCount == 0 )
    {
      m_pcTmpInputFrame->copyFrom( apcFrameList[0] );
      m_uiCopyX = 0;
      m_uiCopyY = 0;
      m_iFrameBufferCount = m_uiFacesX * m_uiFacesY;
    }

    for( unsigned i = 0; i < m_uiFacesPerFrame; i++ )
    {
      m_pcTmpFace->copyFrom( m_pcTmpInputFrame, m_uiCopyX, m_uiCopyY );
      m_pcOutputFrame->copyTo( m_pcTmpFace, m_pcTmpFace->getWidth() * i, 0 );
      m_iFrameBufferCount--;
      m_uiCopyX += m_pcTmpFace->getWidth();
      if( m_uiCopyX >= m_pcTmpInputFrame->getWidth() )
      {
        m_uiCopyX = 0;
        m_uiCopyY += m_pcTmpFace->getHeight();
      }
    }
  }
  else
  {
    m_pcOutputFrame->copyTo( apcFrameList[0], m_uiCopyX, m_uiCopyY );
    m_uiFacesCount += m_uiFacesPerFrame;

    m_uiCopyX += apcFrameList[0]->getWidth();
    if( m_uiCopyX >= m_pcOutputFrame->getWidth() )
    {
      m_uiCopyX = 0;
      m_uiCopyY += apcFrameList[0]->getHeight();
    }
    if( m_uiFacesCount < ( m_uiFacesX * m_uiFacesY ) )
    {
      return NULL;
    }
    m_iFrameBufferCount = 0;
    m_uiCopyX = 0;
    m_uiCopyY = 0;
    m_uiFacesCount = 0;
    return m_pcOutputFrame;
  }

  return m_pcOutputFrame;
}

void ThreeSixtySpatialtoTemporal::destroy()
{
  if( m_pcOutputFrame )
    delete m_pcOutputFrame;
  m_pcOutputFrame = NULL;

  if( m_pcTmpInputFrame )
    delete m_pcTmpInputFrame;
  m_pcTmpInputFrame = NULL;

  if( m_pcTmpFace )
    delete m_pcTmpFace;
  m_pcTmpFace = NULL;
}

/*    This file is a part of Calyp project
 *    Copyright (C) 2014-2018  by Joao Carreira   (jfmcarreira@gmail.com)
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
 * \file     HEVCIntraPrediction.cpp
 * \brief    Binarize frame module
 */

#include "HEVCIntraPrediction.h"

HEVCIntraPrediction::HEVCIntraPrediction()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "HEVC";
  m_pchModuleLongName = "Intra Prediction";
  m_pchModuleName = "HEVCIntraPrediction";
  m_pchModuleTooltip = "Apply intra-frame prediction";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_OPTIONS;

  m_cModuleOptions.addOptions() /**/
//   ( "mode", m_uiMode, "Intra mode [0-34]" )
//   ( "block_size", m_uiBlockSize, "Block size" )
  ( "x_pel", m_uiXpel, "X coordinate [0]" )
  ( "y_pel", m_uiXpel, "Y coordinate [0]" )
  ;

  m_pcPredBlock = NULL;
  m_uiMode = 31;
  m_uiBlockSize = 4;
  m_uiXpel = 0;
  m_uiYpel = 0;
}

bool HEVCIntraPrediction::create( std::vector<CalypFrame*> apcFrameList )
{
  m_pcPredBlock = NULL;
  m_pcPredBlock = new CalypFrame( m_uiBlockSize, m_uiBlockSize, CLP_GRAY, apcFrameList[0]->getBitsPel() );
  return true;
}

CalypFrame* HEVCIntraPrediction::process( std::vector<CalypFrame*> apcFrameList )
{
  // Vertical modes
  if( m_uiMode >= 18 && m_uiMode <=34 )
  {


  }
  return m_pcPredBlock;
}

void HEVCIntraPrediction::destroy()
{
  if( m_pcPredBlock )
    delete m_pcPredBlock;
  m_pcPredBlock = NULL;
}

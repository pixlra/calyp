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
 * \file     HEVCIntraPrediction.cpp
 * \brief    Binarize frame module
 */

#include "HEVCIntraPrediction.h"

#include <cassert>

HEVCIntraPrediction::HEVCIntraPrediction()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = ClpModuleType::FrameProcessing;
  m_pchModuleCategory = "HEVC";
  m_pchModuleLongName = "Intra Prediction";
  m_pchModuleName = "HEVCIntraPrediction";
  m_pchModuleTooltip = "Apply intra-frame prediction";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = ClpModuleFeature::NewWindow |
                           ClpModuleFeature::KeysShortcuts |
                           ClpModuleFeature::HasInfo |
                           ClpModuleFeature::Options;

  m_cModuleOptions.addOptions()                                      /**/
      ( "mode", m_uiMode, "Intra mode (26-34) [26]" )                /**/
      ( "block_size", m_iBlockSize, "Block size [4]" )               /**/
      ( "x_pel", m_uiXpel, "X coordinate [1]" )                      /**/
      ( "y_pel", m_uiYpel, "Y coordinate [1]" )                      /**/
      ( "recon", m_bShowResidue, "Show prediction residue [false]" ) /**/
      ;

  m_pcPredBlock = NULL;

  m_uiMode = 26;
  m_iBlockSize = 4;
  m_uiXpel = 1;
  m_uiYpel = 1;
}

bool HEVCIntraPrediction::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_
  m_pcPredBlock = NULL;
  m_pcPredBlock = new CalypFrame( m_iBlockSize * 2 + 1, m_iBlockSize * 2 + 1, ClpPixelFormats::CLP_GRAY, apcFrameList[0]->getBitsPel() );

  m_referenceMem.resize( m_iBlockSize * 4 );

  return true;
}

static const int g_AngularParamLookUp[] = { 32, 26, 21, 17, 13, 9, 5, 2, 0, -2, -5, -9, -13, -17, -21, -26,         // Horizontal
                                            -32, -26, -21, -17, -13, -9, -5, -2, 0, 2, 5, 9, 13, 17, 21, 26, 32 };  // Vertical

static const int g_InverAngularParamLookup[2][8] = { { -32, -26, -21, -17, -13, -9, -5, -2 },
                                                     { -256, -315, -390, -482, -630, -910, -1638, -4096 } };

int getAngularParam( int mode )
{
  return g_AngularParamLookUp[mode - 2];
}

int getInverseAngularParam( int angular_param )
{
  for( int i = 0; i < 8; i++ )
  {
    if( g_InverAngularParamLookup[0][i] == angular_param )
      return g_InverAngularParamLookup[1][i];
  }
  assert( 0 );
  return 0;
}

CalypFrame* HEVCIntraPrediction::process( std::vector<CalypFrame*> apcFrameList )
{
  if( m_uiMode < 2 )
  {
    m_uiMode = 2;
  }
  else if( m_uiMode > 34 )
  {
    m_uiMode = 34;
  }

  int y, x;
  int *main_coordinate, *sub_cordinate;
  ClpPel** predBlock = m_pcPredBlock->getPelBufferYUV()[0];
  ClpPel** refFrame = apcFrameList[0]->getPelBufferYUV()[0];

  // Reset block
  for( y = 0; y < m_iBlockSize * 2 + 1; y++ )
  {
    for( x = 0; x < m_iBlockSize * 2 + 1; x++ )
    {
      predBlock[y][x] = refFrame[m_uiYpel + y - 1][m_uiXpel + x - 1];
    }
  }

  ClpPel* refArray = &( m_referenceMem[m_iBlockSize * 2] );
  int angular_param = getAngularParam( m_uiMode );

  if( m_uiMode >= 2 && m_uiMode <= 17 )
  {  // Horizontal modes
    main_coordinate = &y;
    sub_cordinate = &x;
  }
  else
  {  // Vertical modes
    main_coordinate = &x;
    sub_cordinate = &y;
  }

  *main_coordinate = 0;
  *sub_cordinate = 0;
  for( int i = 0; i < m_iBlockSize * 2 + 1; i++ )
  {
    *main_coordinate = i;
    refArray[i] = predBlock[y][x];
  }
  if( angular_param < 0 )
  {
    int inverse_angular_param = getInverseAngularParam( angular_param );
    *main_coordinate = 0;
    *sub_cordinate = 0;
    for( int i = -1; i > -int( 2 * m_iBlockSize ); i-- )
    {
      *sub_cordinate = ( ( i * inverse_angular_param + 128 ) >> 8 );
      if( *sub_cordinate >= 2 * int( m_iBlockSize ) )
        break;
      refArray[i] = predBlock[y][x];
    }
  }

  for( y = 0; y < m_iBlockSize; y++ )
  {
    for( x = 0; x < m_iBlockSize; x++ )
    {
      int ref_pos = *main_coordinate + 1 + ( ( ( *sub_cordinate + 1 ) * angular_param ) >> 5 );
      int fract = ( ( *sub_cordinate + 1 ) * angular_param ) & 31;
      predBlock[y + 1][x + 1] = ( ( 32 - fract ) * refArray[ref_pos] + fract * refArray[ref_pos + 1] + 16 ) >> 5;
    }
  }

  if( m_bShowResidue )
  {
    for( y = 0; y < m_iBlockSize; y++ )
    {
      for( x = 0; x < m_iBlockSize; x++ )
      {
        predBlock[y + 1][x + 1] = abs( refFrame[m_uiYpel + y - 1][m_uiXpel + x - 1] - predBlock[y + 1][x + 1] );
      }
    }
  }

  return m_pcPredBlock;
}

bool HEVCIntraPrediction::keyPressed( enum Module_Key_Supported value )
{
  if( value == MODULE_KEY_LEFT )
  {
    m_uiMode -= 1;
    return true;
  }
  if( value == MODULE_KEY_RIGHT )
  {
    m_uiMode += 1;
    return true;
  }
  if( value == MODULE_KEY_UP )
  {
    m_uiMode += 1;
    return true;
  }
  if( value == MODULE_KEY_DOWN )
  {
    m_uiMode -= 1;
    return true;
  }
  return false;
}

std::string HEVCIntraPrediction::moduleInfo()
{
  return std::string( "Intra mode: " ) + std::to_string( m_uiMode );
}

void HEVCIntraPrediction::destroy()
{
  if( m_pcPredBlock )
    delete m_pcPredBlock;
  m_pcPredBlock = NULL;
}

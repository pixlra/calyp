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
 * \file     StreamHandlerPortableMap.cpp
 * \brief    Handling portable pixmap formats
 */

#include "StreamHandlerPortableMap.h"

#include <cmath>
#include <cstdio>

#include "CalypFrame.h"
#include "PixelFormats.h"

std::vector<CalypStreamFormat> StreamHandlerPortableMap::supportedReadFormats()
{
  INI_REGIST_CALYP_SUPPORTED_FMT;
  REGIST_CALYP_SUPPORTED_FMT( &StreamHandlerPortableMap::Create, "Portable BitMap ", "pbm" );
  REGIST_CALYP_SUPPORTED_FMT( &StreamHandlerPortableMap::Create, "Portable GrayMap ", "pgm" );
  REGIST_CALYP_SUPPORTED_FMT( &StreamHandlerPortableMap::Create, "Portable PixMap ", "ppm" );
  END_REGIST_CALYP_SUPPORTED_FMT;
}

std::vector<CalypStreamFormat> StreamHandlerPortableMap::supportedWriteFormats()
{
  INI_REGIST_CALYP_SUPPORTED_FMT;
  REGIST_CALYP_SUPPORTED_FMT( &StreamHandlerPortableMap::Create, "Portable BitMap ", "pbm" );
  REGIST_CALYP_SUPPORTED_FMT( &StreamHandlerPortableMap::Create, "Portable GrayMap ", "pgm" );
  REGIST_CALYP_SUPPORTED_FMT( &StreamHandlerPortableMap::Create, "Portable PixMap ", "ppm" );
  END_REGIST_CALYP_SUPPORTED_FMT;
}

bool StreamHandlerPortableMap::openHandler( std::string strFilename, bool bInput )
{
  m_bIsInput = bInput;
  m_pFile = NULL;
  m_strFormatName = "PGM";
  m_strCodecName = "Raw Video";
  if( m_bIsInput )
  {
    m_pFile = fopen( strFilename.c_str(), "rb" );
    char line[101];
    while( fgets( line, 100, m_pFile ) && line[0] == '#' )
      ;

    if( sscanf( line, "P%d %u %u %d", &m_iMagicNumber, &m_uiWidth, &m_uiHeight, &m_iMaxValue ) != 4 )
    {
      sscanf( line, "P%d", &m_iMagicNumber );
      while( fgets( line, 100, m_pFile ) && line[0] == '#' )
        ;
      sscanf( line, "%u %u", &m_uiWidth, &m_uiHeight );

      if( m_iMagicNumber == 1 )
      {
        m_iMaxValue = 1;
      }
      else
      {
        while( fgets( line, 100, m_pFile ) && line[0] == '#' )
          ;
        sscanf( line, "%d", &m_iMaxValue );
      }
    }
    m_uiBitsPerPixel = log( m_iMaxValue + 1 ) / log( 2 );
    m_iPixelFormat = m_iMagicNumber == 1 || m_iMagicNumber == 2 || m_iMagicNumber == 5 ? ClpPixelFormats::Gray : ClpPixelFormats::RGB24;
  }
  else
  {
    int colorSpace = CalypFrame::pelFormatColorSpace( m_iPixelFormat );
    m_iMaxValue = ( 1 << m_uiBitsPerPixel ) - 1;
    if( m_uiBitsPerPixel == 1 )
    {
      m_iMagicNumber = 1;
      m_iPixelFormat = ClpPixelFormats::Gray;
    }
    else if( colorSpace == CLP_COLOR_GRAY )
    {
      m_iMagicNumber = 5;
      m_iPixelFormat = ClpPixelFormats::Gray;
    }
    else if( colorSpace == CLP_COLOR_RGB )
    {
      m_iMagicNumber = 6;
      m_iPixelFormat = ClpPixelFormats::RGB24;
    }
    else
    {
      closeHandler();
      throw CalypFailure( "CalypStream", "Invalid format for portable map" );
      return false;
    }
    m_pFile = fopen( strFilename.c_str(), "wb" );
  }
  m_iEndianness = CLP_BIG_ENDIAN;
  m_dFrameRate = 1;
  m_uiTotalNumberFrames = 1;
  if( m_pFile == NULL )
  {
    closeHandler();
    return false;
  }
  return true;
}

void StreamHandlerPortableMap::closeHandler()
{
  if( m_pFile )
    fclose( m_pFile );
}

bool StreamHandlerPortableMap::configureBuffer( const CalypFrame& pcFrame )
{
  m_pStreamBuffer.resize( pcFrame.getBytesPerFrame() );
  return true;
}

bool StreamHandlerPortableMap::seek( std::uint64_t iFrameNum )
{
  // m_uiCurrFrameFileIdx =
  return true;
}

bool StreamHandlerPortableMap::read( CalypFrame& pcFrame )
{
  unsigned long long int processed_bytes = fread( m_pStreamBuffer.data(), sizeof( ClpByte ), m_uiNBytesPerFrame, m_pFile );
  if( processed_bytes != m_uiNBytesPerFrame )
    return false;
  pcFrame.frameFromBuffer( m_pStreamBuffer, CLP_BIG_ENDIAN );
  m_uiCurrFrameFileIdx++;
  return true;
}

bool StreamHandlerPortableMap::write( const CalypFrame& pcFrame )
{
  CalypFrame pcRGBFrame( pcFrame.getWidth(), pcFrame.getHeight(),
                         m_iPixelFormat, pcFrame.getBitsPel() );
  pcRGBFrame.copyFrom( pcFrame );
  fseek( m_pFile, 0, SEEK_SET );
  fprintf( m_pFile, "P%d\n%d %d\n", m_iMagicNumber, m_uiWidth, m_uiHeight );
  if( m_iMagicNumber > 4 )
  {
    fprintf( m_pFile, "%d\n", m_iMaxValue );
  }
  pcRGBFrame.frameToBuffer( m_pStreamBuffer, m_iEndianness );
  unsigned long long int processed_bytes = fwrite( m_pStreamBuffer.data(), sizeof( ClpByte ), m_uiNBytesPerFrame, m_pFile );
  if( processed_bytes != m_uiNBytesPerFrame )
    return false;
  return true;
}

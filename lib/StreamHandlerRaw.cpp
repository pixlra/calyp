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
 * \file     StreamHandlerRaw.cpp
 * \brief    interface for raw (yuv) streams
 */

#include "StreamHandlerRaw.h"

#include <cstdio>
#include <filesystem>

#include "CalypFrame.h"

std::vector<CalypStreamFormat> StreamHandlerRaw::supportedReadFormats()
{
  INI_REGIST_CALYP_SUPPORTED_FMT;
  REGIST_CALYP_SUPPORTED_FMT( &StreamHandlerRaw::Create, "Raw YUV Video", "yuv" );
  REGIST_CALYP_SUPPORTED_FMT( &StreamHandlerRaw::Create, "Raw Gray Video", "gray" );
  REGIST_CALYP_SUPPORTED_FMT( &StreamHandlerRaw::Create, "Raw RGB Video", "rgb" );
  REGIST_CALYP_SUPPORTED_FMT( &StreamHandlerRaw::Create, "Raw Video", "raw" );
  END_REGIST_CALYP_SUPPORTED_FMT;
}

std::vector<CalypStreamFormat> StreamHandlerRaw::supportedWriteFormats()
{
  INI_REGIST_CALYP_SUPPORTED_FMT;
  REGIST_CALYP_SUPPORTED_FMT( &StreamHandlerRaw::Create, "Raw Video", "yuv" );
  END_REGIST_CALYP_SUPPORTED_FMT;
}

StreamHandlerRaw::StreamHandlerRaw()
    : CalypStreamHandlerIf{ true }
{
  m_pchHandlerName = "RawVideo";
}

bool StreamHandlerRaw::openHandler( std::string strFilename, bool bInput )
{
  m_bIsInput = bInput;
  m_pFile = NULL;
  m_pFile = fopen( strFilename.c_str(), bInput ? "rb" : "wb" );
  if( m_pFile == NULL )
  {
    return false;
  }
  calculateFrameNumber();
  std::string fileExtension = std::filesystem::path{ strFilename }.extension();
  fileExtension.erase( fileExtension.begin() );
  m_strFormatName = clpUppercase( fileExtension );
  m_strCodecName = "Raw Video";
  return true;
}

void StreamHandlerRaw::closeHandler()
{
  if( m_pFile )
    fclose( m_pFile );
}

bool StreamHandlerRaw::configureBuffer( const CalypFrame& pcFrame )
{
  m_pStreamBuffer.resize( pcFrame.getBytesPerFrame() );
  return true;
}

void StreamHandlerRaw::calculateFrameNumber()
{
  if( m_pFile && m_uiNBytesPerFrame > 0 )
  {
    fseek( m_pFile, 0, SEEK_END );
    unsigned long long fileSize = ftell( m_pFile );
    fseek( m_pFile, 0, SEEK_SET );
    m_uiTotalNumberFrames = fileSize / m_uiNBytesPerFrame;
  }
}

bool StreamHandlerRaw::seek( std::uint64_t iFrameNum )
{
  if( m_bIsInput && m_pFile )
  {
    fseek( m_pFile, iFrameNum >= 0 ? iFrameNum * m_uiNBytesPerFrame : 0, SEEK_SET );
    m_uiCurrFrameFileIdx = iFrameNum;
    return true;
  }
  return false;
}

bool StreamHandlerRaw::read( CalypFrame& pcFrame )
{
  if( !m_pFile || m_pStreamBuffer.empty() || m_uiNBytesPerFrame == 0 )
    return false;
  unsigned long long int processed_bytes = fread( m_pStreamBuffer.data(), sizeof( ClpByte ), m_uiNBytesPerFrame, m_pFile );
  if( processed_bytes != m_uiNBytesPerFrame )
    return false;
  m_uiCurrFrameFileIdx++;
  pcFrame.frameFromBuffer( m_pStreamBuffer, m_iEndianness );
  return true;
}

bool StreamHandlerRaw::write( const CalypFrame& pcFrame )
{
  pcFrame.frameToBuffer( m_pStreamBuffer, m_iEndianness );
  unsigned long long int processed_bytes = fwrite( m_pStreamBuffer.data(), sizeof( ClpByte ), m_uiNBytesPerFrame, m_pFile );
  if( processed_bytes != m_uiNBytesPerFrame )
    return false;
  return true;
}

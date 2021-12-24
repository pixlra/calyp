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
 * \file     VideostreamResource.h
 * \brief    Class to control video playback
 */

#include "VideostreamResource.h"

#include <qfileinfo.h>

#include <QDataStream>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QRegularExpression>
#include <memory>

#include "lib/CalypFrame.h"

/**
 * \brief Functions to control data stream from stream information
 */

QDataStream& operator<<( QDataStream& out, const CalypFileInfoVector& array )
{
  CalypFileInfo d;
  out << array.size();
  for( int i = 0; i < array.size(); i++ )
  {
    d = array.at( i );
    out << d.m_cFilename << d.m_uiWidth << d.m_uiHeight << static_cast<int>( d.m_iPelFormat ) << d.m_uiBitsPelPixel
        << d.m_iEndianness << d.m_uiFrameRate << d.m_uiFileSize << d.m_bForceRaw;
  }
  return out;
}

QDataStream& operator>>( QDataStream& in, CalypFileInfoVector& array )
{
  int array_size{ 0 };
  in >> array_size;
  for( int i = 0; i < array_size; i++ )
  {
    CalypFileInfo d;
    int pelFormat;
    in >> d.m_cFilename;
    in >> d.m_uiWidth;
    in >> d.m_uiHeight;
    in >> pelFormat;
    in >> d.m_uiBitsPelPixel;
    in >> d.m_iEndianness;
    in >> d.m_uiFrameRate;
    in >> d.m_uiFileSize;
    in >> d.m_bForceRaw;
    d.m_iPelFormat = static_cast<ClpPixelFormats>( pelFormat );
    array.append( d );
  }
  return in;
}

auto findCalypStreamInfo( const CalypFileInfoVector& array, const QString& filename ) -> int
{
  for( int i = 0; i < array.size(); i++ )
    if( array.at( i ).m_cFilename == filename )
      return i;
  return -1;
}

VideostreamResource::VideostreamResource( QObject* parent ) : QObject{ parent } {}

VideostreamResource::~VideostreamResource() = default;

auto VideostreamResource::iteration() -> bool
{
  m_currStream.readNextFrameFillRGBBuffer();
  return true;
}

auto VideostreamResource::isReady() -> bool
{
  return m_currStream.hasWritingSlot();
}

bool VideostreamResource::loadFile( const QString& cFilename )
{
  CalypFileInfo streamInfo;

  m_currStream.getFormat( streamInfo.m_uiWidth, streamInfo.m_uiHeight, streamInfo.m_iPelFormat,
                          streamInfo.m_uiBitsPelPixel, streamInfo.m_iEndianness, streamInfo.m_uiFrameRate );

  [[maybe_unused]] auto guessed =
      video_resource_guess_format( cFilename, streamInfo.m_uiWidth, streamInfo.m_uiHeight, streamInfo.m_iPelFormat,
                                   streamInfo.m_uiBitsPelPixel, streamInfo.m_iEndianness, streamInfo.m_uiFrameRate );

  if( streamInfo.m_iPelFormat == ClpPixelFormats::Invalid )
  {
    streamInfo.m_iPelFormat = ClpPixelFormats::YUV420p;
  }

  streamInfo.m_cFilename = cFilename;

  return loadFile( streamInfo );
}

bool VideostreamResource::loadFile( const CalypFileInfo& streamInfo, bool optimistic )
{
  m_sStreamInfo = streamInfo;
  ClpPixelFormats inputFormat = static_cast<ClpPixelFormats>( m_sStreamInfo.m_iPelFormat );

  try
  {
    if( !m_currStream.open( streamInfo.m_cFilename.toStdString(), streamInfo.m_uiWidth, streamInfo.m_uiHeight,
                            inputFormat, streamInfo.m_uiBitsPelPixel, streamInfo.m_iEndianness,
                            streamInfo.m_uiFrameRate, streamInfo.m_bForceRaw, CalypStream::Type::Input ) )
    {
      return false;
    }
  }
  catch( CalypFailure& e )
  {
    if( !optimistic )
    {
      return false;
    }
    throw( e );
  }

  m_currStream.getFormat( m_sStreamInfo.m_uiWidth, m_sStreamInfo.m_uiHeight, m_sStreamInfo.m_iPelFormat,
                          m_sStreamInfo.m_uiBitsPelPixel, m_sStreamInfo.m_iEndianness, m_sStreamInfo.m_uiFrameRate );
  m_sStreamInfo.m_uiFileSize = QFileInfo{ streamInfo.m_cFilename }.size();

  return true;
}

auto VideostreamResource::getCurrFrame() -> std::shared_ptr<CalypFrame>
{
  return m_currStream.getCurrFrameAsset();
}
bool VideostreamResource::play()
{
  bool isPlaying = false;
  if( m_currStream.getFrameNum() > 1 )
  {
    isPlaying = true;
  }
  m_bIsPlaying = isPlaying;
  return m_bIsPlaying;
}

bool VideostreamResource::playEvent()
{
  bool bEndOfSeq = false;
  if( m_bIsPlaying )
  {
    bEndOfSeq = goToNextFrame( true );
  }
  return bEndOfSeq;
}

void VideostreamResource::pause()
{
  m_bIsPlaying = false;
  triggerRefresh();
}

void VideostreamResource::seekAbsoluteEvent( unsigned int new_frame_num )
{
  if( m_currStream.seekInput( new_frame_num ) )
    triggerRefresh();
}

void VideostreamResource::seekRelativeEvent( bool bIsForward )
{
  if( bIsForward )
  {
    goToNextFrame( true );
  }
  else
  {
    m_currStream.seekInputRelative( bIsForward );
    triggerRefresh();
  }
}

void VideostreamResource::stop()
{
  m_bIsPlaying = false;
  seekAbsoluteEvent( 0 );
  return;
}

auto VideostreamResource::goToNextFrame( bool bThreaded ) -> bool
{
  while( !m_currStream.hasNextFrame() && !m_currStream.isEof() ) {}
  bool bEndOfSeq = m_currStream.setNextFrame();
  if( !bEndOfSeq )
  {
    triggerRefresh();
  }
  return bEndOfSeq;
}

void VideostreamResource::triggerRefresh() {}

auto video_resource_guess_format( const QString& filename, unsigned int& rWidth, unsigned int& rHeight,
                                  ClpPixelFormats& rInputFormat, unsigned int& rBitsPerPixel, int& rEndianness,
                                  unsigned int& rFrameRate ) -> bool
{
  std::vector<CalypStandardResolution> stdResList = CalypStream::stdResolutionSizes();
  bool bGuessed = true;
  bool bGuessedByFilesize = false;
  QString FilenameShort = QFileInfo( filename ).fileName();
  QString fileExtension = QFileInfo( filename ).suffix();

  if( filename.startsWith( "/dev/" ) )
  {
    return true;
  }
  if( !fileExtension.compare( "yuv", Qt::CaseInsensitive ) || !fileExtension.compare( "rgb", Qt::CaseInsensitive ) ||
      !fileExtension.compare( "gray", Qt::CaseInsensitive ) )
  {
    bGuessed = false;
    // Guess pixel format
    const auto& formats_list = CalypFrame::supportedPixelFormatListNames();
    for( const auto& [key, name] : formats_list )
    {
      if( FilenameShort.contains( QString::fromStdString( std::string( name ) ), Qt::CaseInsensitive ) )
      {
        rInputFormat = key;
        break;
      }
    }

    if( rWidth == 0 || rHeight == 0 )
    {
      // Guess resolution - match  resolution name
      int iMatch = -1;
      for( unsigned int i = 0; i < stdResList.size(); i++ )
      {
        if( FilenameShort.contains( QString::fromStdString( stdResList[i].shortName ) ) )
        {
          iMatch = i;
        }
      }
      if( iMatch >= 0 )
      {
        rWidth = stdResList[iMatch].uiWidth;
        rHeight = stdResList[iMatch].uiHeight;
      }

      // Guess resolution - match %dx%d
      // QRegularExpressionMatch resolutionMatch = QRegularExpression( "_\\d*x\\d*" ).match( FilenameShort );
      QRegularExpressionMatch resolutionMatch = QRegularExpression( "_[0-9]+x[0-9]+" ).match( FilenameShort );
      if( resolutionMatch.hasMatch() )
      {
        QString resolutionString = resolutionMatch.captured( resolutionMatch.lastCapturedIndex() );
        if( resolutionString.startsWith( "_" ) || resolutionString.endsWith( "_" ) )
        {
          resolutionString.remove( "_" );
          QStringList resolutionArgs = resolutionString.split( "x" );
          if( resolutionArgs.size() == 2 )
          {
            rWidth = resolutionArgs.at( 0 ).toUInt();
            rHeight = resolutionArgs.at( 1 ).toUInt();
          }
        }
      }
    }

    // Guess resolution by file size
    if( rWidth == 0 && rHeight == 0 )
    {
      FILE* pF = fopen( filename.toStdString().c_str(), "rb" );
      if( pF )
      {
        fseek( pF, 0, SEEK_END );
        unsigned long long int uiFileSize = ftell( pF );
        fclose( pF );

        int count = 0;
        int match = -1;
        for( unsigned int i = 0; i < stdResList.size(); i++ )
        {
          auto frame_bytes =
              CalypFrame::getBytesPerFrame( stdResList[i].uiWidth, stdResList[i].uiHeight, rInputFormat, 8 );
          auto module = uiFileSize % frame_bytes;
          if( module == 0 )
          {
            match = i;
            count++;
          }
        }
        if( count == 1 )
        {
          rWidth = stdResList[match].uiWidth;
          rHeight = stdResList[match].uiHeight;
          bGuessedByFilesize = true;
        }
      }
    }

    // [.|_]
    // Guess bits per pixel - match %dbpp
    QRegularExpressionMatch BppMatch = QRegularExpression( "_[0-9]+bpp" ).match( FilenameShort );
    if( BppMatch.hasMatch() )
    {
      QString matchString = BppMatch.captured( BppMatch.lastCapturedIndex() );
      matchString.remove( "_" );
      matchString.remove( "bpp" );
      rBitsPerPixel = matchString.toUInt();
      if( !( rBitsPerPixel > 0 && rBitsPerPixel < 16 ) )
      {
        rBitsPerPixel = -1;
      }
    }
    QRegularExpressionMatch BppOnlyMatch = QRegularExpression( "_[0-9]+b" ).match( FilenameShort );
    if( BppOnlyMatch.hasMatch() )
    {
      QString matchString = BppOnlyMatch.captured( BppOnlyMatch.lastCapturedIndex() );
      matchString.remove( "_" );
      matchString.remove( "b" );
      rBitsPerPixel = matchString.toUInt();
      if( !( rBitsPerPixel > 0 && rBitsPerPixel < 16 ) )
      {
        rBitsPerPixel = -1;
      }
    }

    // Guess frame rate - match %dbpp
    QRegularExpressionMatch FpsMatch = QRegularExpression( "_[0-9]+fps" ).match( FilenameShort );
    if( FpsMatch.hasMatch() )
    {
      QString matchString = FpsMatch.captured( FpsMatch.lastCapturedIndex() );
      matchString.remove( "_" );
      matchString.remove( "fps" );
      rFrameRate = matchString.toUInt();
      if( rFrameRate < 0 )
      {
        rFrameRate = 30;
      }
    }

    // Guess Endianness
    if( FilenameShort.contains( QStringLiteral( "be" ), Qt::CaseInsensitive ) )
    {
      rEndianness = CLP_BIG_ENDIAN;
    }
    if( FilenameShort.contains( QStringLiteral( "le" ), Qt::CaseInsensitive ) )
    {
      rEndianness = CLP_LITTLE_ENDIAN;
    }

    if( rWidth > 0 && rHeight > 0 && rInputFormat != ClpPixelFormats::Invalid )
      bGuessed = true && !bGuessedByFilesize;
  }
  return bGuessed;
}
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
 * \file     VideoStreamSubWindow.cpp
 * \brief    Video Sub windows handling
 */

#include "VideoStreamSubWindow.h"

#include <QScrollArea>
#include <QSettings>
#include <QStaticText>
#include <cassert>

#include "ConfigureFormatDialog.h"
#include "ModulesHandle.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "ResourceHandle.h"
#include "SubWindowAbstract.h"

#define QT_NO_CONCURRENT

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
    out << d.m_cFilename << d.m_uiWidth << d.m_uiHeight << d.m_iPelFormat << d.m_uiBitsPelPixel << d.m_iEndianness
        << d.m_uiFrameRate << d.m_uiFileSize;
  }
  return out;
}

QDataStream& operator>>( QDataStream& in, CalypFileInfoVector& array )
{
  CalypFileInfo d;
  int array_size{ 0 };
  in >> array_size;
  for( int i = 0; i < array_size; i++ )
  {
    in >> d.m_cFilename;
    in >> d.m_uiWidth;
    in >> d.m_uiHeight;
    in >> d.m_iPelFormat;
    in >> d.m_uiBitsPelPixel;
    in >> d.m_iEndianness;
    in >> d.m_uiFrameRate;
    in >> d.m_uiFileSize;
    array.append( d );
  }
  return in;
}

int findCalypStreamInfo( CalypFileInfoVector array, QString filename )
{
  for( int i = 0; i < array.size(); i++ )
    if( array.at( i ).m_cFilename == filename )
      return i;
  return -1;
}

VideoStreamSubWindow::VideoStreamSubWindow( QWidget* parent )
    : VideoSubWindow( VideoSubWindow::VIDEO_STREAM_SUBWINDOW, parent )
    , m_pcResourceManager( nullptr )
    , m_pCurrStream( NULL )
    , m_bIsPlaying( false )

{
}

VideoStreamSubWindow::~VideoStreamSubWindow()
{
  if( m_pcResourceManager )
    m_pcResourceManager->removeResource( m_uiResourceId );
}

void VideoStreamSubWindow::resetWindowName()
{
  setWindowName( QFileInfo( m_cFilename ).fileName() );
}

void VideoStreamSubWindow::updateVideoWindowInfo()
{
  m_cStreamInformation = "";
  if( m_pcDisplayModule )
  {
    m_cStreamInformation = "Module";
    const auto& arraySubWindows = m_pcDisplayModule->getSubWindowList();
    QStringList windowInfoList;
    if( arraySubWindows.size() > 0 )
    {
      for( std::size_t i = 0; i < arraySubWindows.size(); i++ )
      {
        if( arraySubWindows[i]->getWindowName() != getWindowName() )
        {
          windowInfoList.append( QString( "Input %1 - " + arraySubWindows[i]->getWindowName() ).arg( i + 1 ) );
        }
      }
    }
    if( m_pcDisplayModule->getModuleRequirements() & CLP_MODULES_HAS_INFO )
    {
      QStringList list = QString::fromStdString( m_pcDisplayModule->moduleInfo() ).split( '\n' );
      windowInfoList.append( list );
    }
    if( windowInfoList.size() > 0 )
    {
      m_pcVideoInfo->setInformationTopLeft( windowInfoList );
    }
  }
  else if( m_pCurrStream )
  {
    QString m_cFormatName = QString::fromStdString( m_pCurrStream->getFormatName() );
    QString m_cCodedName = QString::fromStdString( m_pCurrStream->getCodecName() );
    m_cStreamInformation = m_cFormatName + " | " + m_cCodedName;
  }
  if( m_pcCurrFrameAsset )
  {
    QString m_cPelFmtName = QString::fromStdString( m_pcCurrFrameAsset->getPelFmtName() );
    if( m_pCurrStream )
      if( !m_pCurrStream->isNative() )
        m_cPelFmtName += "*";
    m_cStreamInformation += " | " + m_cPelFmtName;
  }
  if( m_cStreamInformation.isEmpty() )
  {
    m_cStreamInformation = "          ";
  }
}

void VideoStreamSubWindow::loadAll()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );
  m_pCurrStream->loadAll();
  refreshFrame();
  QApplication::restoreOverrideCursor();
}

bool VideoStreamSubWindow::loadFile( QString cFilename, bool bForceDialog )
{
  assert( m_pcResourceManager != nullptr );

  ConfigureFormatDialog formatDialog( this );
  unsigned int Width = 0, Height = 0, BitsPel = 8, FrameRate = 30;
  int Endianness = CLP_LITTLE_ENDIAN;
  int InputFormat = CLP_YUV420P;
  QSettings appSettings;

  if( m_pCurrStream )
    m_pCurrStream->getFormat( Width, Height, InputFormat, BitsPel, Endianness, FrameRate );
  else
    m_uiResourceId = m_pcResourceManager->getResource( nullptr );

  m_pCurrStream = m_pcResourceManager->getResourceAsset( m_uiResourceId );

  bool bConfig = true;
  if( !bForceDialog )
  {
    bConfig = guessFormat( cFilename, Width, Height, InputFormat, BitsPel, Endianness, FrameRate );
    if( bConfig )
    {
      // Pre-load values with last opened file
      Width = appSettings.value( "VideoStreamSubWindow/LastWidth" ).value<unsigned int>();
      Height = appSettings.value( "VideoStreamSubWindow/LastHeight" ).value<unsigned int>();
      BitsPel = appSettings.value( "VideoStreamSubWindow/LastBitsPerPixel" ).value<unsigned int>();
    }
  }
  bool bRet = false;
  for( int iPass = 0; iPass < 2 && !bRet; iPass++ )
  {
    if( iPass || bConfig )
    {
      if( formatDialog.runConfigureFormatDialog( QFileInfo( cFilename ).fileName(), Width, Height, InputFormat, BitsPel,
                                                 Endianness, FrameRate ) == QDialog::Rejected )
      {
        return false;
      }
    }
    try
    {
      bRet = m_pCurrStream->open( cFilename.toStdString(), Width, Height, InputFormat, BitsPel, Endianness, FrameRate, true );
    }
    catch( CalypFailure& e )
    {
      if( iPass == 1 )
        throw( e );
    }
  }

  m_sStreamInfo.m_cFilename = cFilename;
  m_sStreamInfo.m_uiWidth = Width;
  m_sStreamInfo.m_uiHeight = Height;
  m_sStreamInfo.m_iPelFormat = InputFormat;
  m_sStreamInfo.m_uiBitsPelPixel = BitsPel;
  m_sStreamInfo.m_iEndianness = Endianness;
  m_sStreamInfo.m_uiFrameRate = FrameRate;
  m_sStreamInfo.m_uiFileSize = QFileInfo( cFilename ).size();

  QVariant var;
  var.setValue<unsigned int>( Width );
  appSettings.setValue( "VideoStreamSubWindow/LastWidth", var );
  var.setValue<unsigned int>( Height );
  appSettings.setValue( "VideoStreamSubWindow/LastHeight", var );
  var.setValue<unsigned int>( BitsPel );
  appSettings.setValue( "VideoStreamSubWindow/LastBitsPerPixel", var );

#ifdef CALYP_MANAGED_RESOURCES
  m_pcResourceManager->startResourceWorker( m_uiResourceId );
#endif

  QApplication::restoreOverrideCursor();

  refreshFrame();

  m_cFilename = cFilename;

  updateVideoWindowInfo();
  setWindowName( QFileInfo( m_cFilename ).fileName() );

  return true;
}

bool VideoStreamSubWindow::loadFile( CalypFileInfo* streamInfo )
{
  assert( m_pcResourceManager != nullptr );

  m_uiResourceId = m_pcResourceManager->getResource( m_pCurrStream );
  m_pCurrStream = m_pcResourceManager->getResourceAsset( m_uiResourceId );

  if( !m_pCurrStream->open( streamInfo->m_cFilename.toStdString(), streamInfo->m_uiWidth, streamInfo->m_uiHeight,
                            streamInfo->m_iPelFormat, streamInfo->m_uiBitsPelPixel, streamInfo->m_iEndianness,
                            streamInfo->m_uiFrameRate, true ) )
  {
    return false;
  }

  m_sStreamInfo = *streamInfo;

#ifdef CALYP_MANAGED_RESOURCES
  m_pcResourceManager->startResourceWorker( m_uiResourceId );
#endif

  QApplication::restoreOverrideCursor();

  refreshFrame();

  m_cFilename = streamInfo->m_cFilename;
  updateVideoWindowInfo();
  setWindowName( QFileInfo( m_cFilename ).fileName() );
  return true;
}

bool VideoStreamSubWindow::guessFormat( QString filename, unsigned int& rWidth, unsigned int& rHeight, int& rInputFormat, unsigned int& rBitsPerPixel,
                                        int& rEndianness, unsigned int& rFrameRate )
{
  std::vector<CalypStandardResolution> stdResList = CalypStream::stdResolutionSizes();
  bool bGuessed = true;
  bool bGuessedByFilesize = false;
  QString FilenameShort = QFileInfo( filename ).fileName();
  QString fileExtension = QFileInfo( filename ).suffix();

  if( filename.startsWith( "/dev/" ) )
  {
    return false;
  }
  if( !fileExtension.compare( "yuv", Qt::CaseInsensitive ) || !fileExtension.compare( "rgb", Qt::CaseInsensitive ) ||
      !fileExtension.compare( "gray", Qt::CaseInsensitive ) )
  {
    bGuessed = false;
    // Guess pixel format
    auto formats_list = CalypFrame::supportedPixelFormatListNames();
    for( std::size_t i = 0; i < formats_list.size(); i++ )
    {
      if( FilenameShort.contains( formats_list[i].c_str(), Qt::CaseInsensitive ) )
      {
        rInputFormat = i;
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
      //QRegularExpressionMatch resolutionMatch = QRegularExpression( "_\\d*x\\d*" ).match( FilenameShort );
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

        int count = 0, module, frame_bytes, match;
        for( unsigned int i = 0; i < stdResList.size(); i++ )
        {
          frame_bytes =
              CalypFrame::getBytesPerFrame( stdResList[i].uiWidth, stdResList[i].uiHeight, rInputFormat, 8 );
          module = uiFileSize % frame_bytes;
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
    QRegularExpressionMatch BppOnlybMatch = QRegularExpression( "_[0-9]+b" ).match( FilenameShort );
    if( BppOnlybMatch.hasMatch() )
    {
      QString matchString = BppOnlybMatch.captured( BppOnlybMatch.lastCapturedIndex() );
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

    if( rWidth > 0 && rHeight > 0 && rInputFormat >= 0 )
      bGuessed = true && !bGuessedByFilesize;
  }
  return !bGuessed;
}

void VideoStreamSubWindow::refreshSubWindow()
{
#ifdef CALYP_MANAGED_RESOURCES
  m_pcResourceManager->stopResourceWorker( m_uiResourceId );
#endif
  if( !m_pCurrStream->reload() )
  {
    close();
    return;
  }
#ifdef CALYP_MANAGED_RESOURCES
  m_pcResourceManager->startResourceWorker( m_uiResourceId );
#endif

  updateVideoWindowInfo();
  refreshFrame();
}

void VideoStreamSubWindow::refreshFrame()
{
  bool bSetFrame = true;
  m_pcCurrFrameAsset = m_pCurrStream->getCurrFrameAsset();

  if( m_pcDisplayModule )
  {
    m_bWindowBusy = true;
    //bool disableThreads = !m_bIsPlaying && hasAssociatedModule();
    m_pcDisplayModule->apply( m_bIsPlaying, true );
    bSetFrame = false;
  }
  if( bSetFrame )
    VideoSubWindow::refreshFrame();
}

void VideoStreamSubWindow::refreshFrame( bool bThreaded )
{
#ifndef QT_NO_CONCURRENT
  m_cRefreshResult.waitForFinished();
  if( bThreaded )
  {
    m_cRefreshResult = QtConcurrent::run( this, &VideoSubWindow::refreshFrame );
  }
  else
#endif
  {
    refreshFrame();
  }
}

bool VideoStreamSubWindow::goToNextFrame( bool bThreaded )
{
#ifndef QT_NO_CONCURRENT
  m_cRefreshResult.waitForFinished();
  m_cReadResult.waitForFinished();
#endif
#ifdef CALYP_MANAGED_RESOURCES
  while( !m_pCurrStream->hasNextFrame() && !m_pCurrStream->isEof() )
    ;
#endif
  bool bEndOfSeq = m_pCurrStream->setNextFrame();
#ifdef CALYP_MANAGED_RESOURCES
  bThreaded = false;
  m_pcResourceManager->wakeResourceWorker( m_uiResourceId );
#endif
  if( !bEndOfSeq )
  {
#ifndef QT_NO_CONCURRENT
    m_cRefreshResult.waitForFinished();
    if( bThreaded )
    {
      m_cReadResult = QtConcurrent::run( m_pCurrStream, &CalypStream::readNextFrameFillRGBBuffer );
      refreshFrame( bThreaded );
    }
    else
#endif
    {
      refreshFrame( bThreaded );
#ifndef CALYP_MANAGED_RESOURCES
      m_pCurrStream->readNextFrameFillRGBBuffer();
#endif
    }
  }
  return bEndOfSeq;
}

bool VideoStreamSubWindow::saveStream( QString filename )
{
  bool iRet = false;
  QApplication::setOverrideCursor( Qt::WaitCursor );

  // TODO: implement this
  QApplication::restoreOverrideCursor();
  return iRet;
}

void VideoStreamSubWindow::setPlaying( bool isPlaying )
{
  m_bIsPlaying = isPlaying;
  // for( auto module : m_associatedModules )
  // {
  //   module->setPlaying( isPlaying );
  // }
}

bool VideoStreamSubWindow::isPlaying()
{
  if( getCategory() & SubWindowCategory::MODULE_SUBWINDOW )
  {
    for( auto window : m_pcDisplayModule->getSubWindowList() )
    {
      if( window->isPlaying() )
        return true;
    }
    return false;
  }
  return m_bIsPlaying;
}

bool VideoStreamSubWindow::play()
{
  bool isPlaying = false;
  if( m_pCurrStream && m_pCurrStream->getFrameNum() > 1 )
  {
    isPlaying = true;
  }
  setPlaying( isPlaying );
  return m_bIsPlaying;
}

bool VideoStreamSubWindow::playEvent()
{
  bool bEndOfSeq = false;
  if( m_pCurrStream && m_bIsPlaying )
  {
    bEndOfSeq = goToNextFrame( true );
  }
  return bEndOfSeq;
}

void VideoStreamSubWindow::pause()
{
  setPlaying( false );
  refreshFrame();
}

void VideoStreamSubWindow::seekAbsoluteEvent( unsigned int new_frame_num )
{
  if( m_pCurrStream )
  {
    if( m_pCurrStream->seekInput( new_frame_num ) )
      refreshFrame();

#ifdef CALYP_MANAGED_RESOURCES
    m_pcResourceManager->wakeResourceWorker( m_uiResourceId );
#endif
  }
}

void VideoStreamSubWindow::seekRelativeEvent( bool bIsFoward )
{
  if( m_pCurrStream )
  {
    if( bIsFoward )
    {
      goToNextFrame( true );
    }
    else
    {
      m_pCurrStream->seekInputRelative( bIsFoward );
#ifdef CALYP_MANAGED_RESOURCES
      m_pcResourceManager->wakeResourceWorker( m_uiResourceId );
#endif
      refreshFrame();
    }
  }
}

void VideoStreamSubWindow::stop()
{
#ifndef QT_NO_CONCURRENT
  m_cRefreshResult.waitForFinished();
  m_cReadResult.waitForFinished();
#endif
  setPlaying( false );
  seekAbsoluteEvent( 0 );
  return;
}

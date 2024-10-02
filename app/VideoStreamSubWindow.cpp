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

VideoStreamSubWindow::VideoStreamSubWindow( ResourceHandle* resourceManager, QWidget* parent )
    : VideoSubWindow( VideoSubWindow::VIDEO_STREAM_SUBWINDOW, parent )
    , m_pcResourceManager{ resourceManager }
    , m_pCurrStream{ NULL }
    , m_bIsPlaying{ false }
{
}

VideoStreamSubWindow::~VideoStreamSubWindow()
{
  if( m_pcResourceManager ) m_pcResourceManager->removeResource( m_uiResourceId );
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
    if( m_pcDisplayModule->hasFeature( ClpModuleFeature::HasInfo ) )
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
      if( !m_pCurrStream->isNative() ) m_cPelFmtName += "*";
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

void VideoStreamSubWindow::setResource( std::size_t id )
{
  m_uiResourceId = id;
  m_pCurrStream = m_pcResourceManager->getResourceAsset( m_uiResourceId );
#ifdef CALYP_MANAGED_RESOURCES
  m_pcResourceManager->startResourceWorker( m_uiResourceId );
#endif

  QApplication::restoreOverrideCursor();

  refreshFrame();

  m_cFilename = QString::fromStdString( m_pCurrStream->getFileName() );

  updateVideoWindowInfo();
  setWindowName( QFileInfo( m_cFilename ).fileName() );
}

// bool VideoStreamSubWindow::loadFile( QString cFilename, bool bForceDialog )
// {
//   assert( m_pcResourceManager != nullptr );

//   ConfigureFormatDialog formatDialog( this );
//   unsigned int Width = 0, Height = 0, BitsPel = 8, FrameRate = 30;
//   int Endianness = CLP_LITTLE_ENDIAN;
//   auto InputFormat = ClpPixelFormats::YUV420p;
//   QSettings appSettings;

//   if( m_pCurrStream )
//     m_pCurrStream->getFormat( Width, Height, InputFormat, BitsPel, Endianness, FrameRate );
//   else
//     m_uiResourceId = m_pcResourceManager->getResource( nullptr );

//   m_pCurrStream = m_pcResourceManager->getResourceAsset( m_uiResourceId );

//   bool bConfig = true;
//   if( !bForceDialog )
//   {
//     bConfig = guessFormat( cFilename, Width, Height, InputFormat, BitsPel, Endianness, FrameRate );
//     if( bConfig )
//     {
//       // Pre-load values with last opened file
//       Width = appSettings.value( "VideoStreamSubWindow/LastWidth" ).value<unsigned int>();
//       Height = appSettings.value( "VideoStreamSubWindow/LastHeight" ).value<unsigned int>();
//       BitsPel = appSettings.value( "VideoStreamSubWindow/LastBitsPerPixel" ).value<unsigned int>();
//     }
//   }
//   bool bRet{ false };
//   bool forceRaw{ false };
//   for( int iPass = 0; iPass < 2 && !bRet; iPass++ )
//   {
//     if( iPass || bConfig )
//     {
//       if( formatDialog.runConfigureFormatDialog( QFileInfo( cFilename ).fileName(), Width, Height, InputFormat,
//       BitsPel,
//                                                  Endianness, FrameRate ) == QDialog::Rejected )
//       {
//         return false;
//       }
//     }
//     try
//     {
//       bRet = m_pCurrStream->open( cFilename.toStdString(), Width, Height, InputFormat, BitsPel, Endianness,
//       FrameRate,
//                                   iPass == 1, CalypStream::Type::Input );
//       forceRaw = iPass > 0;
//     }
//     catch( CalypFailure& e )
//     {
//       if( iPass > 0 )
//         throw( e );
//     }
//   }

//   if( !bRet )
//   {
//     return false;
//   }

//   m_sStreamInfo.m_cFilename = cFilename;
//   m_sStreamInfo.m_uiWidth = Width;
//   m_sStreamInfo.m_uiHeight = Height;
//   m_sStreamInfo.m_iPelFormat = InputFormat;
//   m_sStreamInfo.m_uiBitsPelPixel = BitsPel;
//   m_sStreamInfo.m_iEndianness = Endianness;
//   m_sStreamInfo.m_uiFrameRate = FrameRate;
//   m_sStreamInfo.m_uiFileSize = QFileInfo( cFilename ).size();
//   m_sStreamInfo.m_bForceRaw = forceRaw;

//   QVariant var;
//   var.setValue<unsigned int>( Width );
//   appSettings.setValue( "VideoStreamSubWindow/LastWidth", var );
//   var.setValue<unsigned int>( Height );
//   appSettings.setValue( "VideoStreamSubWindow/LastHeight", var );
//   var.setValue<unsigned int>( BitsPel );
//   appSettings.setValue( "VideoStreamSubWindow/LastBitsPerPixel", var );

// #ifdef CALYP_MANAGED_RESOURCES
//   m_pcResourceManager->startResourceWorker( m_uiResourceId );
// #endif

//   QApplication::restoreOverrideCursor();

//   refreshFrame();

//   m_cFilename = cFilename;

//   updateVideoWindowInfo();
//   setWindowName( QFileInfo( m_cFilename ).fileName() );

//   return true;
// }

// bool VideoStreamSubWindow::loadFile( CalypFileInfo streamInfo )
// {
//   assert( m_pcResourceManager != nullptr );

//   m_uiResourceId = m_pcResourceManager->getResource( m_pCurrStream );
//   m_pCurrStream = m_pcResourceManager->getResourceAsset( m_uiResourceId );

//   if( !m_pCurrStream->open( streamInfo.m_cFilename.toStdString(), streamInfo.m_uiWidth, streamInfo.m_uiHeight,
//                             static_cast<ClpPixelFormats>( streamInfo.m_iPelFormat ), streamInfo.m_uiBitsPelPixel,
//                             streamInfo.m_iEndianness, streamInfo.m_uiFrameRate, streamInfo.m_bForceRaw,
//                             CalypStream::Type::Input ) )
//   {
//     return false;
//   }

//   m_cFilename = streamInfo.m_cFilename;
//   m_sStreamInfo = std::move( streamInfo );

// #ifdef CALYP_MANAGED_RESOURCES
//   m_pcResourceManager->startResourceWorker( m_uiResourceId );
// #endif

//   QApplication::restoreOverrideCursor();

//   refreshFrame();

//   updateVideoWindowInfo();
//   setWindowName( QFileInfo( m_cFilename ).fileName() );
//   return true;
// }

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
    // bool disableThreads = !m_bIsPlaying && hasAssociatedModule();
    m_pcDisplayModule->apply( m_bIsPlaying, true );
    bSetFrame = false;
  }
  if( bSetFrame ) VideoSubWindow::refreshFrame();
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
  {
  }
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

bool VideoStreamSubWindow::saveStream( const QString& filename )
{
  bool iRet = false;
  QApplication::setOverrideCursor( Qt::WaitCursor );

  // TODO: implement this
  QApplication::restoreOverrideCursor();
  return iRet;
}

bool VideoStreamSubWindow::isPlaying()
{
  return m_bIsPlaying;
}

bool VideoStreamSubWindow::play()
{
  bool isPlaying = false;
  if( m_pCurrStream && m_pCurrStream->getFrameNum() > 1 )
  {
    isPlaying = true;
  }
  m_bIsPlaying = isPlaying;
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
  m_bIsPlaying = false;
  refreshFrame();
}

void VideoStreamSubWindow::seekAbsoluteEvent( unsigned int new_frame_num )
{
  if( m_pCurrStream )
  {
    if( m_pCurrStream->seekInput( new_frame_num ) ) refreshFrame();

#ifdef CALYP_MANAGED_RESOURCES
    m_pcResourceManager->wakeResourceWorker( m_uiResourceId );
#endif
  }
}

void VideoStreamSubWindow::seekRelativeEvent( bool bIsForward )
{
  if( m_pCurrStream )
  {
    if( bIsForward )
    {
      goToNextFrame( true );
    }
    else
    {
      m_pCurrStream->seekInputRelative( bIsForward );
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
  m_bIsPlaying = false;
  seekAbsoluteEvent( 0 );
  return;
}

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
 * \file     CalypAppModuleIf.cpp
 * \brief    Calyp modules interface
 */

#include "CalypAppModuleIf.h"

#include <QAction>
#include <QApplication>
#include <QDockWidget>

#include "VideoSubWindow.h"
#include "lib/CalypFrame.h"
#include "lib/CalypStream.h"

CalypAppModuleIf::CalypAppModuleIf( QObject* parent, QAction* action, CalypModulePtr&& module )
    : m_pcModuleAction( action ), m_pcModule( std ::move( module ) )
{
  setParent( parent );
  m_frameList.reserve( m_pcModule->m_uiNumberOfFrames );
  m_frameListPtr.reserve( m_pcModule->m_uiNumberOfFrames );
  m_pcSubWindow.resize( m_pcModule->m_uiNumberOfFrames, nullptr );
}

CalypAppModuleIf::~CalypAppModuleIf()
{
  while( m_bIsRunning )
  {
  }

  if( m_pcDockWidget )
  {
    m_pcDockWidget->close();
  }

  for( auto& window : m_pcSubWindow )
  {
    if( window != nullptr )
      window->disableModule( this );
    window = nullptr;
  }
  if( m_pcModule )
  {
    m_pcModule->destroy();
    m_pcModule = nullptr;
  }

  //assert( m_pcProcessedFrame.use_count() == 1 );
}

void CalypAppModuleIf::update( bool isPlaying )
{
  if( m_pcDisplaySubWindow )
  {
    m_pcDisplaySubWindow->refreshFrame();
  }
  else
  {
    apply( isPlaying );
  }
}

void CalypAppModuleIf::setPlaying( bool isPlaying )
{
  if( m_pcDisplaySubWindow )
  {
    m_pcDisplaySubWindow->setPlaying( isPlaying );
  }
}

bool CalypAppModuleIf::isRunning()
{
#ifdef CALYP_THREADED_MODULES
  return QThread::isRunning();
#else
  return false;
#endif
}

bool CalypAppModuleIf::apply( bool isPlaying, bool disableThreads )
{
  bool bRet = false;

  QApplication::setOverrideCursor( Qt::WaitCursor );
  if( m_pcModule->m_iModuleType == CLP_FRAME_PROCESSING_MODULE )
  {
    if( m_pcDisplaySubWindow )
    {
      m_pcDisplaySubWindow->setFillWindow( true );
    }
    else
    {
      m_pcSubWindow[0]->setFillWindow( true );
    }
  }
  if( !( isPlaying && ( m_pcModule->m_uiModuleRequirements & CLP_MODULE_REQUIRES_SKIP_WHILE_PLAY ) ) )
  {
    m_frameList.clear();
    for( unsigned int i = 0; i < m_pcModule->m_uiNumberOfFrames; i++ )
    {
      m_frameList.push_back( m_pcSubWindow[i]->getCurrFrameAsset() );
    }

#ifdef CALYP_THREADED_MODULES
    if( !disableThreads )
    {
      wait();
      start();
    }
    else
#endif
    {
      run();
      show();
    }

    if( m_pcDisplaySubWindow || m_pcModule->m_iModuleType == CLP_FRAME_MEASUREMENT_MODULE )
    {
      bRet = true;
    }
  }
  else
  {
    bRet = true;
  }
  QApplication::restoreOverrideCursor();
  return bRet;
}

void CalypAppModuleIf::run()
{
  m_bIsRunning = true;
  m_bSuccess = false;

  QMutexLocker locker( &m_Mutex );

  m_frameListPtr.clear();
  for( auto& frame : m_frameList )
  {
    m_frameListPtr.push_back( frame.get() );
  }

  if( m_pcModule->m_iModuleType == CLP_FRAME_PROCESSING_MODULE )
  {
    CalypFrame* framePtr{ nullptr };
    if( m_pcModule->m_iModuleAPI >= CLP_MODULE_API_2 )
    {
      framePtr = m_pcModule->process( m_frameListPtr );
    }
    else
    {
      framePtr = m_pcModule->process( m_pcSubWindow[0]->getCurrFrame() );
    }
    assert( framePtr != nullptr );
    m_pcProcessedFrame = std::shared_ptr<CalypFrame>{ framePtr, []( CalypFrame* ptr ) {} };
  }
  else if( m_pcModule->m_iModuleType == CLP_FRAME_MEASUREMENT_MODULE )
  {
    if( m_pcModule->m_iModuleAPI >= CLP_MODULE_API_2 )
    {
      m_dMeasurementResult = m_pcModule->measure( m_frameListPtr );
    }
    else
    {
      m_dMeasurementResult = m_pcModule->measure( m_pcSubWindow[0]->getCurrFrame() );
    }
  }
  else
  {
    return;
  }
#ifdef CALYP_THREADED_MODULES
  EventData* eventData = new EventData( m_bSuccess, this );
  if( parent() )
    QCoreApplication::postEvent( parent(), eventData );
#endif
  m_bSuccess = true;
  m_bIsRunning = false;
  return;
}

void CalypAppModuleIf::show()
{
  switch( m_pcModule->m_iModuleType )
  {
  case CLP_FRAME_PROCESSING_MODULE:
    if( m_pcDisplaySubWindow )
    {
      m_pcDisplaySubWindow->setCurrFrame( m_pcProcessedFrame );
      m_pcDisplaySubWindow->setFillWindow( isRunning() );
    }
    else
    {
      m_pcSubWindow[0]->setCurrFrame( m_pcProcessedFrame );
      m_pcSubWindow[0]->setFillWindow( isRunning() );
    }
    break;
  case CLP_FRAME_MEASUREMENT_MODULE:
    m_pcModuleDock->setModuleReturnValue( m_dMeasurementResult );
    break;
  }
}

void CalypAppModuleIf::destroy()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );
  QMutexLocker locker( &m_Mutex );
  QApplication::restoreOverrideCursor();

  m_frameList.clear();
  m_frameListPtr.clear();
  if( m_pcModuleDock )
  {
    auto* ptr = m_pcModuleDock;
    m_pcModuleDock = NULL;
    ptr->close();
  }
  if( m_pcDisplaySubWindow )
  {
    auto* ptr = m_pcDisplaySubWindow;
    m_pcDisplaySubWindow = NULL;
    ptr->closeSubWindow();
  }
}

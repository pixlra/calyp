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

#include "ModuleHandleDock.h"
#include "ModuleSubWindow.h"
#include "VideoSubWindow.h"
#include "lib/CalypFrame.h"
#include "lib/CalypStream.h"

CalypAppModuleIf::CalypAppModuleIf( QObject* parent, QAction* action, CalypModulePtr&& module )
    : m_pcModuleAction( action ), m_pcModule( std ::move( module ) )
{
  setParent( parent );
#ifdef CALYP_THREADED_MODULES
  setObjectName( m_pcModule->m_pchModuleName );
#endif
  m_frameList.reserve( m_pcModule->m_uiNumberOfFrames );
  m_frameListPtr.reserve( m_pcModule->m_uiNumberOfFrames );
  m_pcSubWindow.resize( m_pcModule->m_uiNumberOfFrames, nullptr );
}

CalypAppModuleIf::~CalypAppModuleIf()
{
#ifdef CALYP_THREADED_MODULES
  wait();
#endif
  if( m_pcModuleDock )
  {
    auto* ptr = m_pcModuleDock;
    m_pcModuleDock = NULL;
    ptr->close();
    delete ptr;
  }
  if( m_pcDisplaySubWindow )
  {
    auto* ptr = m_pcDisplaySubWindow;
    m_pcDisplaySubWindow = NULL;
    ptr->closeSubWindow();
    delete ptr;
  }
  if( m_pcDockWidget )
  {
    m_pcDockWidget->close();
    delete m_pcDockWidget;
  }
  if( m_pcModule )
  {
    m_pcModule->destroy();
    m_pcModule = nullptr;
  }
}

void CalypAppModuleIf::update( bool isPlaying )
{
  if( m_pcDisplaySubWindow )
  {
    m_pcDisplaySubWindow->refreshSubWindow();
  }
  else
  {
    apply( isPlaying );
  }
}

// void CalypAppModuleIf::setPlaying( bool isPlaying )
// {
//   if( m_pcDisplaySubWindow )
//   {
//     m_pcDisplaySubWindow->setPlaying( isPlaying );
//   }
// }

bool CalypAppModuleIf::isRunning() const
{
#ifdef CALYP_THREADED_MODULES
  return QThread::isRunning();
#else
  return false;
#endif
}

void CalypAppModuleIf::run()
{
  bool result = process();
  if( result )
  {
    EventData* eventData = new EventData( m_bSuccess, this );
    QCoreApplication::postEvent( parent(), eventData );
  }
}

bool CalypAppModuleIf::apply( bool isPlaying, bool disableThreads )
{
  bool moduleExecuted = false;
  if( m_pcModule->m_iModuleType == ClpModuleType::FrameProcessing )
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

  if( !( isPlaying && m_pcModule->m_uiModuleRequirements.is_set( ClpModuleFeature::SkipWhilePlaying ) ) )
  {
    m_frameList.clear();
    for( unsigned int i = 0; i < m_pcModule->m_uiNumberOfFrames; i++ )
    {
      m_frameList.push_back( m_pcSubWindow[i]->getCurrFrameAsset() );
    }

    // Skip module is is already running
    if( isRunning() )
      return moduleExecuted;

    moduleExecuted = true;
#ifdef CALYP_THREADED_MODULES
    if( !disableThreads )
    {
      wait();
      start();
    }
    else
#endif
    {
      QApplication::setOverrideCursor( Qt::WaitCursor );
      process();
      QApplication::restoreOverrideCursor();
      show();
    }
  }
  return moduleExecuted;
}

bool CalypAppModuleIf::process()
{
  m_bSuccess = false;

  if( m_frameList.empty() )
    return false;

#ifdef CALYP_THREADED_MODULES
  QMutexLocker locker( &m_Mutex );
#endif

  m_frameListPtr.clear();
  for( auto& frame : m_frameList )
  {
    m_frameListPtr.push_back( frame.get() );
  }

  if( m_pcModule->m_iModuleType == ClpModuleType::FrameProcessing )
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
  else if( m_pcModule->m_iModuleType == ClpModuleType::FrameMeasurement )
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
    return false;
  }
  m_bSuccess = true;
  return true;
}

void CalypAppModuleIf::show()
{
  if( m_canceling )
    return;
  switch( m_pcModule->m_iModuleType )
  {
  case ClpModuleType::FrameProcessing: {
    auto displayWindow = qobject_cast<VideoSubWindow*>( m_pcDisplaySubWindow );
    if( displayWindow == nullptr )
      displayWindow = m_pcSubWindow[0];
#ifdef CALYP_THREADED_MODULES
    if( m_Mutex.try_lock() )
    {
      displayWindow->setFillWindow( false );
      displayWindow->setCurrFrame( m_pcProcessedFrame );
      m_Mutex.unlock();
    }
#else
    displayWindow->setFillWindow( false );
    displayWindow->setCurrFrame( m_pcProcessedFrame );
#endif
    break;
  }
  case ClpModuleType::FrameMeasurement:
    m_pcModuleDock->setModuleReturnValue( m_dMeasurementResult );
    break;
  }
}

void CalypAppModuleIf::disable()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );
#ifdef CALYP_THREADED_MODULES
  m_canceling = true;
  m_Mutex.lock();
  m_Mutex.unlock();
#endif
  m_frameList.clear();
  m_frameListPtr.clear();
  while( m_pcSubWindow.size() > 0 )
  {
    auto window = m_pcSubWindow.back();
    m_pcSubWindow.erase( m_pcSubWindow.end() - 1 );
    if( window != nullptr )
      window->disableModule( this );
  }
  QApplication::restoreOverrideCursor();
}

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
 * \file     ModulesHandle.cpp
 * \brief    Calyp modules handle
 */

#include "ModulesHandle.h"

#include <cstdio>

#include "MainWindow.h"
#include "ModulesHandleOptDialog.h"
#include "SubWindowHandle.h"
#include "SubWindowSelectorDialog.h"
#include "VideoHandle.h"
#include "VideoSubWindow.h"
#include "modules/CalypModulesFactory.h"

ModulesHandle::ModulesHandle( QWidget* parent, SubWindowHandle* windowManager, VideoHandle* moduleVideo )
    : QWidget( parent ), m_pcParent( parent ), m_pcMainWindowManager( windowManager ), m_appModuleVideo( moduleVideo )
{
  setParent( m_pcParent );
}

ModulesHandle::~ModulesHandle() {}

void ModulesHandle::createActions()
{
  m_arrayActions.resize( MODULES_TOTAL_ACT );

  m_pcActionMapper = new QSignalMapper( this );
  connect( m_pcActionMapper, SIGNAL( mapped( int ) ), this, SLOT( processOpt( int ) ) );

  //  m_arrayActions[FORCE_PLAYING_REFRESH_ACT] = new QAction( "Refresh while
  //  playing", parent() );
  //  m_arrayActions[FORCE_PLAYING_REFRESH_ACT]->setStatusTip( "Force module
  //  refreshing while playing" );
  //  m_arrayActions[FORCE_PLAYING_REFRESH_ACT]->setCheckable( true );

  m_arrayActions[LOAD_EXTERNAL_ACT] = new QAction( "Load external module", parent() );
  m_arrayActions[LOAD_EXTERNAL_ACT]->setStatusTip( "Load an externally build module" );
  connect( m_arrayActions[LOAD_EXTERNAL_ACT], SIGNAL( triggered() ), this, SLOT( loadExternalModule() ) );

  m_arrayActions[APPLY_ALL_ACT] = new QAction( "Apply to All", parent() );
  m_arrayActions[APPLY_ALL_ACT]->setStatusTip( "Apply module to all frames and save the result" );
  connect( m_arrayActions[APPLY_ALL_ACT], SIGNAL( triggered() ), m_pcActionMapper, SLOT( map() ) );
  m_pcActionMapper->setMapping( m_arrayActions[APPLY_ALL_ACT], APPLY_ALL_ACT );

  m_arrayActions[SWAP_FRAMES_ACT] = new QAction( "Swap frames", parent() );
  m_arrayActions[SWAP_FRAMES_ACT]->setStatusTip( "Swap Sub Window order" );
  connect( m_arrayActions[SWAP_FRAMES_ACT], SIGNAL( triggered() ), m_pcActionMapper, SLOT( map() ) );
  m_pcActionMapper->setMapping( m_arrayActions[SWAP_FRAMES_ACT], SWAP_FRAMES_ACT );

  m_arrayActions[DISABLE_ACT] = new QAction( "Disable Modules", parent() );
  connect( m_arrayActions[DISABLE_ACT], SIGNAL( triggered() ), this, SLOT( destroyWindowModules() ) );

  m_arrayActions[DISABLE_ALL_ACT] = new QAction( "Disable All Modules", parent() );
  connect( m_arrayActions[DISABLE_ALL_ACT], SIGNAL( triggered() ), this, SLOT( destroyAllModulesIf() ) );

  m_arrayActions[FORCE_NEW_WINDOW_ACT] = new QAction( "Use New Window", parent() );
  m_arrayActions[FORCE_NEW_WINDOW_ACT]->setStatusTip(
      "Show module result in a new window. Some modules already force this "
      "feature" );
  m_arrayActions[FORCE_NEW_WINDOW_ACT]->setCheckable( true );
}

QMenu* ModulesHandle::createMenu()
{
  m_pcModulesMenu = new QMenu( "&Modules", this );
  buildMenu();
  return m_pcModulesMenu;
}

void ModulesHandle::buildMenu()
{
  CalypModuleIf* pcCurrModuleIf;
  QString ModuleIfinternalName;
  QAction* currAction;
  QMenu* currSubMenu;

  for( int i = 0; i < m_arrayModulesActions.size(); i++ )
  {
    delete m_arrayModulesActions.at( i );
  }
  m_arrayModulesActions.clear();
  m_pcModulesMenu->clear();
  m_pcModulesSubMenuList.clear();

  CalypModulesFactoryMap& moduleFactoryMap = CalypModulesFactory::Get()->getMap();
  CalypModulesFactoryMap::iterator it = moduleFactoryMap.begin();
  for( ; it != moduleFactoryMap.end(); ++it )
  {
    pcCurrModuleIf = it->second();
    ModuleIfinternalName = QString::fromLocal8Bit( it->first );

    currSubMenu = NULL;
    if( pcCurrModuleIf->m_iModuleAPI <= CLP_MODULE_API_2 )
    {
      if( pcCurrModuleIf->m_pchModuleCategory )
      {
        for( int j = 0; j < m_pcModulesSubMenuList.size(); j++ )
        {
          if( m_pcModulesSubMenuList.at( j )->title() == QString( pcCurrModuleIf->m_pchModuleCategory ) )
          {
            currSubMenu = m_pcModulesSubMenuList.at( j );
            break;
          }
        }
        if( !currSubMenu )
        {
          currSubMenu = m_pcModulesMenu->addMenu( pcCurrModuleIf->m_pchModuleCategory );
          m_pcModulesSubMenuList.append( currSubMenu );
        }
      }
    }

    currAction = new QAction( pcCurrModuleIf->getModuleLongName(), parent() );
    QString moduleTooltip = pcCurrModuleIf->m_pchModuleTooltip;
    if( pcCurrModuleIf->m_uiNumberOfFrames > 1 )
      moduleTooltip += QString( " | Requires %1 frames" ).arg( pcCurrModuleIf->m_uiNumberOfFrames );

    currAction->setStatusTip( moduleTooltip );
    currAction->setData( ModuleIfinternalName );
    currAction->setCheckable( false );
    connect( currAction, SIGNAL( triggered() ), this, SLOT( activateModule() ) );
    m_arrayModulesActions.append( currAction );

    if( currSubMenu )
      currSubMenu->addAction( currAction );
    else
      m_pcModulesMenu->addAction( currAction );

    pcCurrModuleIf->Delete();
  }

  m_pcModulesMenu->addAction( m_arrayActions[LOAD_EXTERNAL_ACT] );
  m_pcModulesMenu->addSeparator();
  m_pcModulesMenu->addAction( m_arrayActions[APPLY_ALL_ACT] );
  m_pcModulesMenu->addAction( m_arrayActions[SWAP_FRAMES_ACT] );
  m_pcModulesMenu->addAction( m_arrayActions[DISABLE_ACT] );
  m_pcModulesMenu->addAction( m_arrayActions[DISABLE_ALL_ACT] );
  m_pcModulesMenu->addAction( m_arrayActions[FORCE_NEW_WINDOW_ACT] );
}

void ModulesHandle::updateMenus()
{
  VideoSubWindow* pcSubWindow = qobject_cast<VideoSubWindow*>( m_pcMainWindowManager->activeSubWindow() );
  bool hasSubWindow = pcSubWindow ? true : false;
  QAction* currModuleAction;

  for( int i = 0; i < m_arrayModulesActions.size(); i++ )
  {
    currModuleAction = m_arrayModulesActions.at( i );
    //currModuleAction->setEnabled( hasSubWindow );
    currModuleAction->setChecked( false );
  }
  //for( int i = 0; i < m_pcModulesSubMenuList.size(); i++ )
  //{
  //	m_pcModulesSubMenuList.at( i )->setEnabled( hasSubWindow );
  //}

  for( int i = 0; i < m_arrayActions.size(); i++ )
  {
    m_arrayActions.at( i )->setEnabled( false );
  }
  m_arrayActions[LOAD_EXTERNAL_ACT]->setEnabled( true );
  m_arrayActions[FORCE_NEW_WINDOW_ACT]->setEnabled( hasSubWindow );
  m_arrayActions[DISABLE_ALL_ACT]->setEnabled( m_pcCalypAppModuleIfList.size() > 0 );

  if( pcSubWindow )
  {
    QList<CalypAppModuleIf*> apcCurrentModule = pcSubWindow->getModuleArray();
    m_arrayActions[DISABLE_ACT]->setEnabled( apcCurrentModule.size() > 0 );
    if( pcSubWindow->getDisplayModule() )
    {
      m_arrayActions[APPLY_ALL_ACT]->setEnabled( true );
      m_arrayActions[SWAP_FRAMES_ACT]->setEnabled( true );
    }
    for( int i = 0; i < apcCurrentModule.size(); i++ )
    {
      currModuleAction = apcCurrentModule.at( i )->m_pcModuleAction;
      currModuleAction->setChecked( true );
    }
  }
}

void ModulesHandle::readSettings()
{
  QSettings appSettings;
  m_arrayActions[FORCE_NEW_WINDOW_ACT]->setChecked( appSettings.value( "ModulesHandle/UseNewWindow", false ).toBool() );
}

void ModulesHandle::writeSettings()
{
  QSettings appSettings;
  appSettings.setValue( "ModulesHandle/UseNewWindow", m_arrayActions[FORCE_NEW_WINDOW_ACT]->isChecked() );
}

void ModulesHandle::processOpt( int index )
{
  VideoSubWindow* pcSubWindow = qobject_cast<VideoSubWindow*>( m_pcMainWindowManager->activeSubWindow() );
  if( pcSubWindow )
  {
    CalypAppModuleIf* pcCurrentModule = pcSubWindow->getDisplayModule();
    if( pcCurrentModule )
    {
      CalypAppModuleIf* pcCurrModule = pcCurrentModule;
      switch( index )
      {
      case INVALID_OPT:
        break;
      case SWAP_FRAMES_ACT:
        swapModulesWindowsIf( pcCurrModule );
        break;
      case APPLY_ALL_ACT:
        applyAllModuleIf( pcCurrModule );
        break;
      default:
        Q_ASSERT( 0 );
      }
    }
    emit changed();
  }
}

void ModulesHandle::loadExternalModule()
{
  bool bUpdate = false;
  QString supported = tr( "Supported Files (*.so)" );
  QStringList formatsList;
  formatsList << "Shared Libraries (*.so)";
  QStringList filter;
  filter << supported << formatsList << tr( "All Files (*)" );
  QStringList fileNames =
      QFileDialog::getOpenFileNames( m_pcParent, tr( "Open File" ), QString(), filter.join( ";;" ) );

  for( int i = 0; i < fileNames.size(); i++ )
  {
    bUpdate |= CalypModulesFactory::Get()->RegisterDl( fileNames.at( i ).toStdString().data() );
  }
  if( bUpdate )
    buildMenu();
}

void ModulesHandle::activateModule()
{
  QAction* pcAction = qobject_cast<QAction*>( sender() );
  Qt::KeyboardModifiers keyModifiers = QApplication::keyboardModifiers();
  bool bTmpForceNewWindow = false;

  VideoSubWindow* pcVideoSubWindow = qobject_cast<VideoSubWindow*>( m_pcMainWindowManager->activeSubWindow() );
  if( !pcVideoSubWindow )
  {
    return;
  }

  if( ( keyModifiers & Qt::ControlModifier ) || pcVideoSubWindow->checkCategory( VideoSubWindow::MODULE_SUBWINDOW ) )
  {
    bTmpForceNewWindow = true;
  }

  QString ModuleIfName = pcAction->data().toString();
  CalypModuleIf* pcCurrModuleIf =
      CalypModulesFactory::Get()->CreateModule( ModuleIfName.toLocal8Bit().constData() );
  CalypAppModuleIf* pcCurrAppModuleIf = new CalypAppModuleIf( this, pcAction, pcCurrModuleIf );

  QList<VideoSubWindow*> videoSubWindowList;
  int numberOfFrames = pcCurrAppModuleIf->m_pcModule->m_uiNumberOfFrames;
  if( numberOfFrames > 1 )  // Show dialog to select sub windows
  {
    int minNumFrames = numberOfFrames;
    int maxNumFrames = numberOfFrames;
    if( pcCurrAppModuleIf->m_pcModule->m_uiModuleRequirements & CLP_MODULES_VARIABLE_NUM_FRAMES )
    {
      minNumFrames = numberOfFrames;
      maxNumFrames = 255;
    }
    SubWindowSelectorDialog dialogWindowsSelection( m_pcParent, m_pcMainWindowManager,
                                                    SubWindowAbstract::VIDEO_SUBWINDOW, minNumFrames, maxNumFrames );
    QList<SubWindowAbstract*> windowsList = m_pcMainWindowManager->findSubWindow( SubWindowAbstract::VIDEO_SUBWINDOW );

    if( windowsList.size() == minNumFrames && windowsList.size() == maxNumFrames )
    {
      for( int i = 0; i < windowsList.size(); i++ )
      {
        videoSubWindowList.append( qobject_cast<VideoSubWindow*>( windowsList.at( i ) ) );
      }
    }
    else
    {
      if( windowsList.size() <= minNumFrames )
      {
        for( int i = 0; i < windowsList.size(); i++ )
        {
          dialogWindowsSelection.selectSubWindow( windowsList.at( i ) );
        }
      }
      else
      {
        dialogWindowsSelection.selectSubWindow( pcVideoSubWindow );
      }
      if( dialogWindowsSelection.exec() == QDialog::Accepted )
      {
        VideoSubWindow* videoSubWindow;
        QList<SubWindowAbstract*> subWindowList = dialogWindowsSelection.getSelectedWindows();
        for( int i = 0; i < subWindowList.size(); i++ )
        {
          videoSubWindow = qobject_cast<VideoSubWindow*>( subWindowList.at( i ) );
          videoSubWindowList.append( videoSubWindow );
        }
      }
    }
    if( pcCurrAppModuleIf->m_pcModule->m_iModuleAPI == CLP_MODULE_API_1 )
    {
      // Check for same fmt in more than one frame modules
      for( int i = 1; i < videoSubWindowList.size(); i++ )
      {
        if( !videoSubWindowList.at( i )->getCurrFrame()->haveSameFmt( videoSubWindowList.at( 0 )->getCurrFrame() ) )
        {
          videoSubWindowList.clear();
          qobject_cast<MainWindow*>( m_pcParent )->printMessage( "Error! Incompatible frames", CLP_LOG_ERROR );
          destroyModuleIf( pcCurrAppModuleIf );
          return;
        }
      }
    }
  }
  else
  {
    videoSubWindowList.append( pcVideoSubWindow );
  }

  if( videoSubWindowList.size() == 0 )
  {
    qobject_cast<MainWindow*>( m_pcParent )
        ->printMessage( "Error! There is no windows to apply the module", CLP_LOG_ERROR );
    destroyModuleIf( pcCurrAppModuleIf );
    return;
  }

  for( int i = 0; i < videoSubWindowList.size(); i++ )
  {
    pcCurrAppModuleIf->m_pcSubWindow[i] = videoSubWindowList.at( i );
  }

  if( pcCurrAppModuleIf->m_pcModule->m_uiModuleRequirements & CLP_MODULE_REQUIRES_OPTIONS )
  {
    ModulesHandleOptDialog moduleOptDialog( m_pcParent, pcCurrAppModuleIf );
    if( moduleOptDialog.runConfiguration() == QDialog::Rejected )
    {
      qobject_cast<MainWindow*>( m_pcParent )->printMessage( "Module canceled by user!", CLP_LOG_WARNINGS );
      destroyModuleIf( pcCurrAppModuleIf );
      return;
    }
  }

  bool bShowModulesNewWindow = m_arrayActions[FORCE_NEW_WINDOW_ACT]->isChecked() | bTmpForceNewWindow;

  QString windowName;

  VideoSubWindow* pcModuleSubWindow = NULL;
  if( pcCurrAppModuleIf->m_pcModule->m_iModuleType == CLP_FRAME_PROCESSING_MODULE )
  {
    if( ( pcCurrAppModuleIf->m_pcModule->m_uiModuleRequirements & CLP_MODULE_REQUIRES_NEW_WINDOW ) || bShowModulesNewWindow )
    {
      pcModuleSubWindow = new VideoSubWindow( VideoSubWindow::MODULE_SUBWINDOW );
      windowName.append( QStringLiteral( "Module " ) );
      windowName.append( pcCurrAppModuleIf->m_pcModule->m_pchModuleLongName );
      pcCurrAppModuleIf->m_pcDisplaySubWindow = pcModuleSubWindow;
      //      connect( pcModuleSubWindow->getViewArea(), SIGNAL( selectionChanged( QRect ) ), m_appModuleVideo,
      //               SLOT( updateSelectionArea( QRect ) ) );
      //      connect( pcModuleSubWindow, SIGNAL( zoomFactorChanged_SWindow( const double, const QPoint ) ), m_appModuleVideo,
      //               SLOT( zoomToFactorAll( double, QPoint ) ) );
      //      connect( pcModuleSubWindow, SIGNAL( scrollBarMoved_SWindow( const QPoint ) ), m_appModuleVideo,
      //               SLOT( moveAllScrollBars( const QPoint ) ) );
    }
  }
  else if( pcCurrAppModuleIf->m_pcModule->m_iModuleType == CLP_FRAME_MEASUREMENT_MODULE )
  {
    pcCurrAppModuleIf->m_pcDisplaySubWindow = NULL;
    pcCurrAppModuleIf->m_pcModuleDock = new ModuleHandleDock( m_pcParent, pcCurrAppModuleIf );
    QString titleDockWidget;
    titleDockWidget.append( pcCurrAppModuleIf->m_pcModule->m_pchModuleName );
    titleDockWidget.append( " Information" );
    pcCurrAppModuleIf->m_pcDockWidget = new QDockWidget( titleDockWidget, m_pcParent );
    pcCurrAppModuleIf->m_pcDockWidget->setFeatures( QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable );
    pcCurrAppModuleIf->m_pcDockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    pcCurrAppModuleIf->m_pcDockWidget->setWidget( pcCurrAppModuleIf->m_pcModuleDock );
    qobject_cast<QMainWindow*>( m_pcParent )
        ->addDockWidget( Qt::RightDockWidgetArea, pcCurrAppModuleIf->m_pcDockWidget );
  }

  // Associate module with subwindows
  if( !pcCurrAppModuleIf->m_pcDisplaySubWindow && !pcCurrAppModuleIf->m_pcModuleDock )
  {
    pcCurrAppModuleIf->m_pcSubWindow[0]->enableModule( pcCurrAppModuleIf );
  }
  else
  {
    if( pcCurrAppModuleIf->m_pcDisplaySubWindow )
      pcCurrAppModuleIf->m_pcDisplaySubWindow->enableModule( pcCurrAppModuleIf );
    for( int i = 0; i < videoSubWindowList.size(); i++ )
    {
      pcCurrAppModuleIf->m_pcSubWindow[i]->associateModule( pcCurrAppModuleIf );
    }
  }

  // Create Module
  bool moduleCreated = false;
  if( pcCurrAppModuleIf->m_pcModule->m_iModuleAPI >= CLP_MODULE_API_2 )
  {
    std::vector<CalypFrame*> apcFrameList;
    for( int i = 0; i < videoSubWindowList.size(); i++ )
    {
      apcFrameList.push_back( pcCurrAppModuleIf->m_pcSubWindow[i]->getCurrFrame() );
    }
    moduleCreated = pcCurrAppModuleIf->m_pcModule->create( apcFrameList );
  }
  else if( pcCurrAppModuleIf->m_pcModule->m_iModuleAPI == CLP_MODULE_API_1 )
  {
    pcCurrAppModuleIf->m_pcModule->create( pcCurrAppModuleIf->m_pcSubWindow[0]->getCurrFrame() );
    moduleCreated = true;
  }

  if( !moduleCreated )
  {
    qobject_cast<MainWindow*>( m_pcParent )->printMessage( "Error! Module cannot be created", CLP_LOG_ERROR );
    destroyModuleIf( pcCurrAppModuleIf );
    return;
  }

  pcCurrAppModuleIf->apply( false, true );
  QCoreApplication::processEvents();

  if( pcModuleSubWindow )
  {
    pcModuleSubWindow->setWindowName( windowName );
    m_pcMainWindowManager->addSubWindow( pcModuleSubWindow );
    pcModuleSubWindow->show();
    m_appModuleVideo->addSubWindow( pcModuleSubWindow );
  }
  else
  {
    windowName = videoSubWindowList.at( 0 )->getWindowName();
    windowName.append( QStringLiteral( " - Module " ) );
    windowName.append( pcCurrAppModuleIf->m_pcModule->m_pchModuleName );
    videoSubWindowList.at( 0 )->setWindowName( windowName );
  }

  m_pcCalypAppModuleIfList.append( pcCurrAppModuleIf );

  emit changed();
}

void ModulesHandle::destroyModuleIf( CalypAppModuleIf* pcCurrModuleIf )
{
  if( pcCurrModuleIf )
  {
    pcCurrModuleIf->destroy();
  }
}

void ModulesHandle::destroyWindowModules()
{
  VideoSubWindow* pcVideoSubWindow = qobject_cast<VideoSubWindow*>( m_pcMainWindowManager->activeSubWindow() );
  if( pcVideoSubWindow )
  {
    pcVideoSubWindow->disableModule();
    emit changed();
  }
}

void ModulesHandle::destroyAllModulesIf()
{
  for( int i = 0; i < m_pcCalypAppModuleIfList.size(); i++ )
  {
    destroyModuleIf( m_pcCalypAppModuleIfList.at( i ) );
  }
  emit changed();
}

void ModulesHandle::applyModuleIf( QList<CalypAppModuleIf*> pcCurrModuleIfList, bool isPlaying, bool disableThreads )
{
  for( int i = 0; i < pcCurrModuleIfList.size() && !isPlaying; i++ )
  {
    pcCurrModuleIfList.at( i )->apply( isPlaying, disableThreads );
  }
}

void ModulesHandle::applyAllModuleIf( CalypAppModuleIf* pcCurrModuleIf )
{
  if( pcCurrModuleIf )
  {
    if( !pcCurrModuleIf->m_pcModuleStream )
    {
      unsigned int Width = 0, Height = 0, FrameRate = 30;
      int BitsPixel, InputFormat = -1;

      QString supported = tr( "Supported Files (" );
      QStringList formatsList;
      std::vector<CalypStreamFormat> supportedFmts = CalypStream::supportedWriteFormats();

      for( unsigned int i = 0; i < supportedFmts.size(); i++ )
      {
        std::vector<ClpString> arrayExt = supportedFmts[i].getExts();
        if( arrayExt.size() > 0 )
        {
          QString currFmt( QString::fromStdString( supportedFmts[i].formatName ) );
          currFmt.append( " (" );
          for( std::vector<ClpString>::iterator e = arrayExt.begin(); e != arrayExt.end(); ++e )
          {
            supported.append( " *." );
            currFmt.append( "*." );
            supported.append( QString::fromStdString( *e ) );
            currFmt.append( QString::fromStdString( *e ) );
          }
          currFmt.append( ")" );
          formatsList << currFmt;
        }
      }
      supported.append( " )" );

      QStringList filter;
      filter << supported << formatsList << tr( "All Files (*)" );

      QString fileName = QFileDialog::getSaveFileName( m_pcParent, tr( "Open File" ), QString(), filter.join( ";;" ) );
      if( fileName.isEmpty() )
      {
        return;
      }
      Width = pcCurrModuleIf->m_pcProcessedFrame->getWidth();
      Height = pcCurrModuleIf->m_pcProcessedFrame->getHeight();
      InputFormat = pcCurrModuleIf->m_pcProcessedFrame->getPelFormat();
      BitsPixel = pcCurrModuleIf->m_pcProcessedFrame->getBitsPel();
      FrameRate = pcCurrModuleIf->m_pcSubWindow[0]->getInputStream()->getFrameRate();

      pcCurrModuleIf->m_pcModuleStream = new CalypStream;
      if( !pcCurrModuleIf->m_pcModuleStream->open( fileName.toStdString(), Width, Height, InputFormat, BitsPixel,
                                                   CLP_LITTLE_ENDIAN, FrameRate, false ) )
      {
        delete pcCurrModuleIf->m_pcModuleStream;
        pcCurrModuleIf->m_pcModuleStream = NULL;
        return;
      }
    }
  }
  if( pcCurrModuleIf->m_pcModuleStream )
  {
    unsigned int numberOfWindows = pcCurrModuleIf->m_pcModule->m_uiNumberOfFrames;
    ClpULong currFrames = 0;
    ClpULong numberOfFrames = -1;
    for( unsigned int i = 0; i < numberOfWindows; i++ )
    {
      currFrames = pcCurrModuleIf->m_pcSubWindow[i]->getInputStream()->getFrameNum();
      if( currFrames < numberOfFrames )
        numberOfFrames = currFrames;
      pcCurrModuleIf->m_pcSubWindow[i]->stop();
    }
    QApplication::setOverrideCursor( Qt::WaitCursor );
    for( unsigned int f = 1; f < numberOfFrames; f++ )
    {
      pcCurrModuleIf->apply( false, true );
      QCoreApplication::processEvents();
      pcCurrModuleIf->m_pcModuleStream->writeFrame( pcCurrModuleIf->m_pcProcessedFrame );
      for( unsigned int i = 0; i < numberOfWindows; i++ )
      {
        pcCurrModuleIf->m_pcSubWindow[i]->play();
        pcCurrModuleIf->m_pcSubWindow[i]->playEvent();
      }
    }
    for( unsigned int i = 0; i < numberOfWindows; i++ )
    {
      pcCurrModuleIf->m_pcSubWindow[i]->stop();
    }
    QApplication::restoreOverrideCursor();
  }
}

void ModulesHandle::swapModulesWindowsIf( CalypAppModuleIf* pcCurrModuleIf )
{
  if( pcCurrModuleIf->m_pcModule->m_uiNumberOfFrames == 2 )
  {
    VideoSubWindow* auxWindowHandle = pcCurrModuleIf->m_pcSubWindow[0];
    pcCurrModuleIf->m_pcSubWindow[0] = pcCurrModuleIf->m_pcSubWindow[1];
    pcCurrModuleIf->m_pcSubWindow[1] = auxWindowHandle;
    pcCurrModuleIf->m_pcDisplaySubWindow->updateVideoWindowInfo();
    pcCurrModuleIf->apply();
  }
}

void ModulesHandle::customEvent( QEvent* event )
{
  if( !event )
  {
    return;
  }
  CalypAppModuleIf::EventData* eventData = (CalypAppModuleIf::EventData*)event;
  if( eventData->m_bSuccess )
  {
    eventData->m_pcModule->show();
  }
}

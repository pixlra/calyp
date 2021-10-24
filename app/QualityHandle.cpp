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
 * \file     QualityHandle.cpp
 * \brief    Definition of the quality measurement sidebar
 */

#include "QualityHandle.h"

#include <QtGui>

#include "PlotSubWindow.h"
#include "ProgressBar.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "SubWindowHandle.h"
#include "SubWindowSelectorDialog.h"
#include "VideoSubWindow.h"

QualityHandle::QualityHandle( QWidget* parent, SubWindowHandle* windowManager )
    : m_pcParent( parent ), m_pcMainWindowManager( windowManager )
{
}

QualityHandle::~QualityHandle() {}

void QualityHandle::createActions()
{
  m_actionGroupQualityMetric = new QActionGroup( this );
  m_actionGroupQualityMetric->setExclusive( true );

  m_mapperQualityMetric = new QSignalMapper( this );

  QAction* currAction;
  for( unsigned int i = 0; i < CalypFrame::supportedQualityMetricsList().size(); i++ )
  {
    currAction = new QAction( CalypFrame::supportedQualityMetricsList()[i].c_str(), this );
    currAction->setCheckable( true );
    m_actionGroupQualityMetric->addAction( currAction );
    connect( currAction, SIGNAL( triggered() ), m_mapperQualityMetric, SLOT( map() ) );
    m_mapperQualityMetric->setMapping( currAction, i );
  }

  m_arrayActions.resize( TOTAL_ACT );

  m_arrayActions[SELECT_CURR_REF_ACT] = new QAction( "Mark as Reference", this );
  connect( m_arrayActions[SELECT_CURR_REF_ACT], SIGNAL( triggered() ), this, SLOT( slotSelectCurrentAsReference() ) );

  m_arrayActions[PLOT_QUALITY] = new QAction( "Plot Window's Quality", this );
  connect( m_arrayActions[PLOT_QUALITY], SIGNAL( triggered() ), this, SLOT( slotPlotQualitySingle() ) );
  m_arrayActions[PLOT_SEVERAL_QUALITY] = new QAction( "Plot Several Quality", this );
  connect( m_arrayActions[PLOT_SEVERAL_QUALITY], SIGNAL( triggered() ), this, SLOT( slotPlotQualitySeveral() ) );
}

QMenu* QualityHandle::createMenu()
{
  m_pcMenuQuality = new QMenu( "Quality", m_pcParent );
  m_pcSubMenuQualityMetrics = m_pcMenuQuality->addMenu( "Quality Metrics" );
  m_pcSubMenuQualityMetrics->addActions( m_actionGroupQualityMetric->actions() );
  m_pcMenuQuality->addAction( m_arrayActions[SELECT_CURR_REF_ACT] );
  m_pcMenuQuality->addSeparator();
  m_pcMenuQuality->addAction( m_arrayActions[PLOT_QUALITY] );
  m_pcMenuQuality->addAction( m_arrayActions[PLOT_SEVERAL_QUALITY] );
  return m_pcMenuQuality;
}

QDockWidget* QualityHandle::createDock()
{
  m_pcQualityHandleSideBar = new QualityMeasurementSidebar( m_pcParent, m_pcMainWindowManager );
  m_pcQualityHandleDock = new QDockWidget( tr( "Quality Measurement" ), m_pcParent );
  m_pcQualityHandleDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
  m_pcQualityHandleDock->setWidget( m_pcQualityHandleSideBar );

  connect( m_pcQualityHandleSideBar, SIGNAL( signalQualityMetricChanged( int ) ), this,
           SLOT( slotQualityMetricChanged( int ) ) );
  connect( m_mapperQualityMetric, SIGNAL( mapped( int ) ), this, SLOT( slotQualityMetricChanged( int ) ) );

  slotQualityMetricChanged( 0 );

  return m_pcQualityHandleDock;
}

void QualityHandle::updateMenus()
{
  VideoSubWindow* pcCurrentVideoSubWindow = m_pcMainWindowManager->activeSubWindow<VideoSubWindow>();
  bool hasSubWindow = pcCurrentVideoSubWindow ? true : false;
  bool hasReference = false;
  bool isReference = false;

  if( hasSubWindow )
  {
    hasReference = pcCurrentVideoSubWindow->getRefSubWindow() != NULL ? true : false;
    if( hasSubWindow && !hasReference )
    {
      VideoSubWindow* pcVideoSubWindow;
      QList<SubWindowAbstract*> subWindowList =
          m_pcMainWindowManager->findSubWindow( SubWindowAbstract::VIDEO_STREAM_SUBWINDOW );
      for( int i = 0; i < subWindowList.size(); i++ )
      {
        pcVideoSubWindow = qobject_cast<VideoSubWindow*>( subWindowList.at( i ) );
        if( pcVideoSubWindow->getRefSubWindow() == pcCurrentVideoSubWindow )
        {
          isReference = true;
          break;
        }
      }
    }
  }

  m_pcSubMenuQualityMetrics->setEnabled( hasSubWindow );
  m_arrayActions[SELECT_CURR_REF_ACT]->setEnabled( hasSubWindow );

  m_arrayActions[PLOT_QUALITY]->setEnabled( hasReference );
  m_arrayActions[PLOT_SEVERAL_QUALITY]->setEnabled( hasReference | isReference );

  m_pcQualityHandleSideBar->updateSideBar( hasSubWindow );
}

void QualityHandle::readSettings()
{
  QSettings appSettings;
  int metric = appSettings.value( "QualityHandle/Metric", 0 ).toInt();
  slotQualityMetricChanged( metric );
  if( !appSettings.value( "QualityHandle/QualitySideBar", true ).toBool() )
    m_pcQualityHandleDock->close();
}

void QualityHandle::writeSettings()
{
  QSettings appSettings;
  appSettings.setValue( "QualityHandle/Metric", m_iQualityMetricIdx );
  appSettings.setValue( "QualityHandle/QualitySideBar", m_pcQualityHandleDock->isVisible() );
}

void QualityHandle::update( VideoSubWindow* currSubWindow )
{
  m_pcQualityHandleSideBar->updateCurrentWindow( currSubWindow );
}

void QualityHandle::measureQuality( QVector<VideoSubWindow*> apcWindowList )
{
  unsigned numberOfWindows = apcWindowList.size();
  std::uint64_t currFrames = 0;
  std::uint64_t numberOfFrames = -1;

  //! Check reference window
  for( unsigned i = 0; i < numberOfWindows; i++ )
  {
    if( !apcWindowList.at( i )->getRefSubWindow() )
    {
      return;
    }
  }
  VideoSubWindow* pcReferenceWindow = apcWindowList.at( 0 )->getRefSubWindow();

  numberOfFrames = pcReferenceWindow->getFrameNum();
  for( unsigned i = 0; i < numberOfWindows; i++ )
  {
    currFrames = apcWindowList.at( i )->getFrameNum();
    if( currFrames < numberOfFrames )
      numberOfFrames = currFrames;
    apcWindowList.at( i )->stop();
  }
  pcReferenceWindow->stop();

  ProgressBar* pcProgressBar = new ProgressBar( m_pcParent, numberOfFrames );

  QVector<double>* padQualityValues = new QVector<double>[numberOfWindows + 1];
  double* padAverageQuality = new double[numberOfWindows + 1];

  for( unsigned i = 0; i < numberOfWindows + 1; i++ )
  {
    padAverageQuality[i] = 0;
  }

  CalypFrame* pcReferenceFrame;
  CalypFrame* pcCurrFrame;
  for( unsigned int f = 0; f < numberOfFrames; f++ )
  {
    padQualityValues[0].append( f );
    pcReferenceFrame = pcReferenceWindow->getCurrFrame();
    for( unsigned int i = 0; i < numberOfWindows; i++ )
    {
      pcCurrFrame = apcWindowList.at( i )->getCurrFrame();
      double dCurrentQuality = pcCurrFrame->getQuality( m_iQualityMetricIdx, pcReferenceFrame, CLP_LUMA );
      padAverageQuality[i + 1] = ( padAverageQuality[i + 1] * double( f ) + dCurrentQuality ) / double( f + 1 );
      padQualityValues[i + 1].append( dCurrentQuality );
      apcWindowList.at( i )->advanceOneFrame();
    }
    pcReferenceWindow->advanceOneFrame();
    pcProgressBar->incrementProgress( 1 );
  }
  for( unsigned int i = 0; i < numberOfWindows; i++ )
  {
    apcWindowList.at( i )->stop();
  }
  pcReferenceWindow->stop();

  QString qualityName = QString::fromStdString( CalypFrame::supportedQualityMetricsList()[m_iQualityMetricIdx] );
  QString qualityUnits = QString::fromStdString( CalypFrame::supportedQualityMetricsUnitsList()[m_iQualityMetricIdx] );

  QString plotWindowTitle( QStringLiteral( "Quality" ) );
  if( apcWindowList.size() == 1 )
  {
    plotWindowTitle += " - " + apcWindowList.at( 0 )->getWindowName();
  }
  PlotSubWindow* pcPlotWindow = new PlotSubWindow( plotWindowTitle );
  pcPlotWindow->setAxisName( "Frame Number", QString( "%1 [%2]" ).arg( qualityName ).arg( qualityUnits ) );

  QString key;
  for( unsigned int i = 0; i < numberOfWindows; i++ )
  {
    key = apcWindowList.at( i )->getWindowName();
    key = QString( "[%1 %2] " ).arg( padAverageQuality[i + 1] ).arg( qualityUnits ) + key;
    pcPlotWindow->addPlot( padQualityValues[0], padQualityValues[i + 1], key );
  }

  m_pcMainWindowManager->addSubWindow( pcPlotWindow );
  pcPlotWindow->show();

  pcProgressBar->close();
}

void QualityHandle::slotQualityMetricChanged( int idx )
{
  m_actionGroupQualityMetric->actions().at( idx )->setChecked( true );
  m_pcQualityHandleSideBar->updateQualityMetric( idx );
  m_pcQualityHandleDock->show();
  m_iQualityMetricIdx = idx;
}

void QualityHandle::slotSelectCurrentAsReference()
{
  VideoSubWindow* pcRefSubWindow = m_pcMainWindowManager->activeSubWindow<VideoSubWindow>();
  if( pcRefSubWindow )
  {
    VideoSubWindow* pcVideoSubWindow;
    QList<SubWindowAbstract*> subWindowList =
        m_pcMainWindowManager->findSubWindow( SubWindowAbstract::VIDEO_SUBWINDOW );
    for( int i = 0; i < subWindowList.size(); i++ )
    {
      pcVideoSubWindow = qobject_cast<VideoSubWindow*>( subWindowList.at( i ) );
      if( pcVideoSubWindow != pcRefSubWindow )
        pcVideoSubWindow->setRefSubWindow( pcRefSubWindow );
    }
    m_pcQualityHandleSideBar->updateSidebarData();
    m_pcQualityHandleDock->show();
    emit changed();
  }
}

void QualityHandle::slotPlotQualitySingle()
{
  VideoSubWindow* pcSubWindow = m_pcMainWindowManager->activeSubWindow<VideoSubWindow>();
  if( pcSubWindow )
  {
    if( pcSubWindow->getRefSubWindow() )
    {
      QVector<VideoSubWindow*> apcWindowList;
      apcWindowList.append( pcSubWindow );

      //#ifndef QT_NO_CONCURRENT
      //      m_cMeasurementResult.waitForFinished();
      //      m_cMeasurementResult = QtConcurrent::run( this,
      //      &QualityHandle::measureQuality, apcWindowList );
      //#else
      QApplication::setOverrideCursor( Qt::WaitCursor );
      measureQuality( apcWindowList );
      QApplication::restoreOverrideCursor();
      //#endif
      emit changed();
    }
  }
}

void QualityHandle::slotPlotQualitySeveral()
{
  VideoSubWindow* pcSubWindow = m_pcMainWindowManager->activeSubWindow<VideoSubWindow>();
  if( pcSubWindow )
  {
    QVector<VideoSubWindow*> apcWindowList;

    VideoSubWindow* pcRefSubWindow = pcSubWindow->getRefSubWindow();
    if( !pcRefSubWindow )
    {
      pcRefSubWindow = pcSubWindow;
    }

    VideoSubWindow* pcVideoSubWindow;
    QList<SubWindowAbstract*> subWindowList =
        m_pcMainWindowManager->findSubWindow( SubWindowAbstract::VIDEO_STREAM_SUBWINDOW );
    for( int i = 0; i < subWindowList.size(); i++ )
    {
      pcVideoSubWindow = qobject_cast<VideoSubWindow*>( subWindowList.at( i ) );
      if( pcVideoSubWindow->getRefSubWindow() == pcRefSubWindow )
      {
        apcWindowList.append( pcVideoSubWindow );
      }
    }

    if( apcWindowList.size() > 0 )
    {
      //#ifndef QT_NO_CONCURRENT
      //      m_cMeasurementResult.waitForFinished();
      //      m_cMeasurementResult = QtConcurrent::run( this,
      //      &QualityHandle::measureQuality, apcWindowList );
      //#else
      QApplication::setOverrideCursor( Qt::WaitCursor );
      measureQuality( apcWindowList );
      QApplication::restoreOverrideCursor();
      //#endif
      emit changed();
    }
  }
}

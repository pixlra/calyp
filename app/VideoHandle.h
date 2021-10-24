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
 * \file     VideoHandle.h
 * \brief    Class to handle video playback
 */

#ifndef __VIDEOHANDLE_H__
#define __VIDEOHANDLE_H__

#include <QElapsedTimer>
#include <QMenu>
#include <QPoint>
#include <QThread>
#include <QVector>
#include <QWidget>

#include "CommonDefs.h"
#include "VideoSubWindow.h"
#include "config.h"

class QToolBar;
class QDockWidget;
class QSignalMapper;
class QLabel;
class QSlider;

class SubWindowHandle;
class SubWindowAbstract;
class VideoStreamSubWindow;
class FramePropertiesDock;
class FrameNumberWidget;
class QTimer;

class VideoHandle : public QObject
{
  Q_OBJECT
public:
  VideoHandle( QWidget*, SubWindowHandle* );
  ~VideoHandle();

  void createActions();
  QMenu* createVideoMenu();
  QMenu* createImageMenu();
  QToolBar* createToolBar();
  QDockWidget* createDock();
  QWidget* createStatusBarMessage();
  void updateMenus();

  void readSettings();
  void writeSettings();

  void update( VideoSubWindow* currSubWindow );

  void addSubWindow( VideoSubWindow* subWindow );

private:
  QWidget* m_pcParent;
  SubWindowHandle* m_pcMainWindowManager;
  enum
  {
    PLAY_ACT,
    STOP_ACT,
    VIDEO_FORWARD_ACT,
    VIDEO_BACKWARD_ACT,
    VIDEO_GOTO_ACT,
    VIDEO_REPEAT_ACT,
    VIDEO_ZOOM_LOCK_ACT,
    VIDEO_LOCK_ACT,
    VIDEO_LOCK_SELECTION_ACT,
    NAVIGATION_TOOL_ACT,
    SELECTION_TOOL_ACT,
    BLOCK_SELECTION_TOOL_ACT,
    SHOW_GRID_ACT,
    TOTAL_ACT,
  };
  QVector<QAction*> m_arrayActions;
  QSignalMapper* m_mapperVideoSeek;

  QSlider* m_pcFrameSlider;
  FrameNumberWidget* m_pcFrameNumInfo;

  // Tools Actions
  QActionGroup* m_actionGroupTools;
  QSignalMapper* m_mapperTools;
  unsigned int m_uiViewTool;

  // Grid Actions
  QSignalMapper* m_mapperGrid;
  QActionGroup* m_actionGroupGrid;

  QMenu* m_pcMenuVideo;
  QMenu* m_pcMenuImage;
  QToolBar* m_toolbarVideo;

  QDockWidget* m_pcFramePropertiesDock;
  FramePropertiesDock* m_pcFramePropertiesSideBar;

  QLabel* m_pcPlayingFPSLabel;
  QLabel* m_pcVideoFormatLabel;
  QLabel* m_pcResolutionLabel;

  QPointer<VideoSubWindow> m_pcCurrentVideoSubWindow;
  QVector<VideoStreamSubWindow*> m_acPlayingSubWindows;

  QTimer* m_pcPlayingTimer;
  bool m_bIsPlaying;

  unsigned int m_uiNumberPlayedFrames{ 0 };
  unsigned int m_uiRealAverageFrameRate{ 0 };
  std::unique_ptr<QElapsedTimer> m_pcFrameRateFeedbackTimer;

  void configureFrameRateTimer();
  void calculateRealFrameRate();
  unsigned long getMaxFrameNumber();
  void setTimerStatus();

Q_SIGNALS:
  void changed();

private Q_SLOTS:
  void update();
  void updateSelectionArea( QRect area );
  void closeSubWindow( SubWindowAbstract* subWindow );
  void zoomToFactorAll( const double factor, const QPoint center = QPoint() );
  void moveAllScrollBars( const double&, const double& );

  void play();
  void stop();

  void playEvent();
  void seekSliderEvent( int new_frame_num );
  void seekEvent( int direction );
  void seekVideo();
  void videoSelectionButtonEvent();
  void setTool( int tool );
  void toggleGrid( bool checked );
  void setGridSize( int size );
};

#endif  // __VIDEOHANDLE_H__

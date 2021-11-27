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
 * \file     VideoSubWindow.h
 * \brief    Video Sub windows handling
 */

#ifndef __VIDEOSUBWINDOW_H__
#define __VIDEOSUBWINDOW_H__

#include <QDataStream>
#include <QFuture>
#include <QRect>
#include <QStaticText>
#include <QString>
#include <QVector>

#include "CalypAppModuleIf.h"
#include "CommonDefs.h"
#include "SubWindowAbstract.h"
#include "ViewArea.h"
#include "config.h"
#include "lib/CalypStream.h"

class QScrollArea;

class VideoInformation;
class VideoSubWindow;
class ResourceHandle;
class CalypAppModuleIf;

/**
 * \class VideoInformation
 * \brief Shows information about video sub window
 */
class VideoInformation : public QWidget
{
public:
  VideoInformation( QWidget* parent );
  void setInformationTopLeft( const QStringList& textLines );
  void setBusyWindow( bool bFlag );

protected:
  void paintEvent( QPaintEvent* event ) override;

private:
  QTimer* m_pcRefreshTimer;
  QList<QStaticText> m_cTopLeftTextList;
  QFont m_cTopLeftTextFont;
  QFont m_cCenterTextFont;
  bool m_bBusyWindow;
};

class VideoSubWindow : public SubWindowAbstract
{
  Q_OBJECT

public:
  enum VideoSubWindowCategories
  {
    VIDEO_STREAM_SUBWINDOW = SubWindowAbstract::VIDEO_STREAM_SUBWINDOW,
    MODULE_SUBWINDOW = SubWindowAbstract::MODULE_SUBWINDOW,
  };
  VideoSubWindow( enum VideoSubWindowCategories category, QWidget* parent = 0 );
  ~VideoSubWindow();

  virtual void resetWindowName() = 0;
  virtual void updateVideoWindowInfo() = 0;

  virtual bool isPlaying() = 0;
  virtual void stop() = 0;
  virtual void advanceOneFrame() = 0;
  virtual auto getFrameNum() -> std::uint64_t { return 1; };

  QString getVideoInformation() { return m_cStreamInformation; }
  bool save( const QString& filename );

  void setCurrFrame( std::shared_ptr<CalypFrame> pcCurrFrame );

  auto getCurrFrame() -> CalypFrame* { return m_pcCurrFrameAsset.get(); }
  auto getCurrFrameAsset() -> std::shared_ptr<CalypFrame> { return m_pcCurrFrameAsset; };
  auto getViewArea() -> ViewArea* { return m_cViewArea; }

  VideoSubWindow* getRefSubWindow() { return m_pcReferenceSubWindow; }
  void setRefSubWindow( VideoSubWindow* subWindow )
  {
    m_pcReferenceSubWindow = NULL;
    if( subWindow )
      if( m_pcCurrFrameAsset->haveSameFmt( subWindow->getCurrFrame() ) )
        m_pcReferenceSubWindow = subWindow;
  }

  /**
   * Functions to enable a module in the
   * current SubWindow
   */
  auto hasModules() -> bool { return m_associatedModules.size() > 0; }
  bool hasAssociatedModule();
  void setDisplayModule( CalypAppModuleIf* pcModule );
  void associateModule( std::shared_ptr<CalypAppModuleIf> pcModule );
  void disableModule( CalypAppModuleIf* pcModule );
  bool disableAllModules();
  CalypAppModuleIf* getDisplayModule() { return m_pcDisplayModule; }

  /**
   * Virtual functions from SubWindowAbstract
   */
  void normalSize() override;
  void zoomToFit() override;
  void scaleView( double scale, QPoint center = QPoint() ) override;
  void zoomToFactor( double factor, QPoint center = QPoint() ) override;

  double getScaleFactor() override { return m_cViewArea->getZoomFactor(); }
  /**
   * Size related functions
   */
  QSize sizeHint( const QSize& ) const override;

  void setFillWindow( bool bFlag );

  void adjustScrollBarToRatio( const double horRatio, const double verRatio );

private:
  bool hasRunningModule();

  /**
   * Private zoom function to handle
   * zoom to fit
   */
  void scaleView( const QSize& size, QPoint center = QPoint() );

  QSize getScrollSize();

protected:
  void refreshFrame();

  void keyPressEvent( QKeyEvent* event ) override;
  void resizeEvent( QResizeEvent* event ) override;
  void closeEvent( QCloseEvent* event ) override;

public Q_SLOTS:
  void updateWindowOnTimeout();
  void adjustScrollBarByScale( double scale, QPoint center );
  void adjustScrollBarByOffset( QPoint Offset );
  void updateScrollValues();
  void updateSelectedArea( QRect area );
  void updatePixelValueStatusBar( const QPoint& pos );

protected:
  VideoInformation* m_pcVideoInfo;
  QString m_cStreamInformation;
  std::shared_ptr<CalypFrame> m_pcCurrFrameAsset;
  // bool m_bWindowBusy;
  // bool m_bIsPlaying;

  std::vector<std::shared_ptr<CalypAppModuleIf>> m_associatedModules;
  CalypAppModuleIf* m_pcDisplayModule{ nullptr };
  // bool m_hasAssociatedModules;

private:
  QScrollArea* m_pcScrollArea;
  QPoint m_cCurrScroll;
  double m_dHorScroll{ 0 };
  double m_dVerScroll{ 0 };
  ViewArea* m_cViewArea;
  QRect m_cSelectedArea;
  VideoSubWindow* m_pcReferenceSubWindow;
  QTimer* m_pcUpdateTimer;
};

#endif  // __VIDEOSUBWINDOW_H__

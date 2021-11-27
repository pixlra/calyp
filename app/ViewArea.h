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
 * \file     ViewArea.h
 * \brief    Function to manage image display
 *           Based on the work of Ricardo N. Rocha Sardo in SCode project
 */

#ifndef __VIEWAREA_H__
#define __VIEWAREA_H__

#include <QBitmap>
#include <QColor>
#include <QPixmap>
#include <QTimer>
#include <QWidget>

#include "CommonDefs.h"
#include "GridManager.h"
#include "config.h"
#include "lib/CalypFrame.h"

class CalypStream;

/**
 *
 */
class ViewArea : public QWidget
{
  Q_OBJECT

public:
  enum ViewModes
  {
    NavigationView,
    NormalSelectionView,
    BlockSelectionView,
  };

  ViewArea( QWidget* parent = 0 );

  void setImage( std::shared_ptr<CalypFrame> frame );

  auto getFilteredChannel() -> std::optional<std::size_t>;
  void setFilteredChannel( std::size_t channel );
  void clearFilteredChannel();

  void setMode( int mode );
  void setMaskColor( const QColor& color = QColor() );

  /**
   * Select tool from the menu
   */
  void setTool( unsigned int view );

  /**
   * Grid options
   */
  void setGridVisible( bool enable );
  void setGridSize( int size );

  /**
   * Clears any mask content.
   */
  void clearMask();

  QRect selectedArea() const { return m_selectedArea; }
  GridManager gridManager() const { return m_grid; }
  QColor maskColor() const { return m_maskColor; }
  double getZoomFactor() { return m_dZoomFactor; }
  // Scale function. Return used scale value (it may change when it touches the
  // min or max zoom value)
  double scaleZoomFactor( double scale, QPoint center, QSize minimumSize );

Q_SIGNALS:
  void selectionChanged( const QRect& rect );
  void positionChanged( const QPoint& pos );
  void scrollBarMoved( QPoint offset );
  void zoomFactorChanged_byWheel( const double factor, const QPoint center );

public Q_SLOTS:
  //  void setNormalMode();
  //  void setMaskMode();
  //  void setMaskTool();
  //  void setEraserTool();
  //  void setNormalSelectionTool();
  //  void setBlockSelectionTool();
  void setSnapToGrid( bool enable );
  //  void setSelectedArea( QRect &rect );

protected:
  void paintEvent( QPaintEvent* event ) override;
  void resizeEvent( QResizeEvent* event ) override;
  void mousePressEvent( QMouseEvent* event ) override;
  void mouseMoveEvent( QMouseEvent* event ) override;
  void mouseReleaseEvent( QMouseEvent* event ) override;
  void wheelEvent( QWheelEvent* event ) override;

  void updateSize();
  void updateOffset();

private:
  enum ViewMode
  {
    NormalMode,
    MaskMode,
  };

  enum Tool
  {
    NavigationTool,
    SelectionTool,
    MaskTool,
    EraserTool
  };

  ViewMode mode() const { return m_mode; }
  Tool tool() const { return m_eTool; }
  void initZoomWinRect();
  void startZoomWinTimer();
  void setZoomFactor( double );

  bool isPosValid( const QPoint& pos ) const;
  void updateMask( const QRect& rect );

  QPoint windowToView( const QPoint& pt ) const;
  QRect windowToView( const QRect& rc ) const;

  QPoint viewToWindow( const QPoint& pt ) const;
  QRect viewToWindow( const QRect& rc ) const;

  std::shared_ptr<CalypFrame> m_currFrame;
  QImage m_image;
  std::shared_ptr<CalypFrame> m_nextFrame;

  std::optional<std::size_t> m_filterSingleChannel;
  ClpPel m_uiPixelHalfScale;

  Tool m_eTool{ NavigationTool };
  bool m_bGridVisible{ false };

  QTimer m_zoomWinTimer;
  double m_dZoomWinRatio;
  double m_dZoomFactor{ 1.0 };

  // QPixmap m_pixmap;
  QBitmap m_mask;
  QRect m_selectedArea;
  QPoint m_lastPos;
  QPoint m_lastWindowPos;
  GridManager m_grid;
  ViewMode m_mode{ NormalMode };

  QColor m_maskColor;
  int m_xOffset{ 0 };
  int m_yOffset{ 0 };
  bool m_blockTrackEnable{ false };
  bool m_newShape;
  bool m_snapToGrid{ false };
  bool m_cursorInGrid;
  bool m_visibleZoomRect{ true };
};

#endif  // __VIEWAREA_H__

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
 * \file     ViewArea.cpp
 * \brief    Function to manage image display
 *           Based on the work of Ricardo N. Rocha Sardo in SCode project
 */

#include "ViewArea.h"

#include <QColor>
#include <QCoreApplication>
#include <QDebug>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmapCache>
#include <QRectF>
#include <QString>
#include <QWidget>
#include <cmath>

#include "GridManager.h"

static const QColor selectionColor = Qt::cyan;
static const QColor imageMaskColor = Qt::green;
static const QColor eraserColor = Qt::red;

ViewArea::ViewArea( QWidget* parent )
    : QWidget( parent )
{
  setMouseTracking( true );

  m_pcCurrFrame = NULL;
  m_pixmap = QPixmap();
  m_grid = GridManager();
  m_mask = QBitmap();
  m_selectedArea = QRect();
  m_dZoomFactor = 1;
  m_xOffset = 0;
  m_yOffset = 0;
  m_mode = NormalMode;
  m_eTool = NavigationTool;
  m_bGridVisible = false;
  m_snapToGrid = false;
  m_blockTrackEnable = false;
  m_visibleZoomRect = true;

  m_zoomWinTimer.setSingleShot( true );
  m_zoomWinTimer.setInterval( 2000 );
  connect( &m_zoomWinTimer, SIGNAL( timeout() ), this, SLOT( update() ) );
}

void ViewArea::setImage( CalypFrame* pcFrame )
{
  m_pcCurrFrame = pcFrame;
  m_uiPixelHalfScale = 1 << ( m_pcCurrFrame->getBitsPel() - 1 );
  m_pcCurrFrame->fillRGBBuffer();
  QImage qimg = QImage( m_pcCurrFrame->getRGBBuffer(), m_pcCurrFrame->getWidth(), m_pcCurrFrame->getHeight(),
                        QImage::Format_ARGB32 );
  setImage( QPixmap::fromImage( qimg ) );
}

void ViewArea::setImage( const QPixmap& pixmap )
{
  m_pixmap = pixmap;
  m_mask = QBitmap( pixmap.width(), pixmap.height() );
  m_mask.clear();
  updateSize();
  update();
  updateGeometry();

  initZoomWinRect();
}

void ViewArea::clear()
{
  m_pixmap = QPixmap();
  m_mask = QBitmap();
  m_mode = NormalMode;
  updateSize();
  update();
  updateGeometry();
}

void ViewArea::setTool( unsigned int view )
{
  switch( view )
  {
  case NavigationView:
    m_eTool = NavigationTool;
    m_snapToGrid = false;
    m_blockTrackEnable = false;
    break;
  case NormalSelectionView:
    m_eTool = SelectionTool;
    m_snapToGrid = false;
    m_blockTrackEnable = false;
    break;
  case BlockSelectionView:
    m_eTool = SelectionTool;
    m_snapToGrid = true;
    m_blockTrackEnable = false;
    break;
  }

  if( m_eTool != SelectionTool )
  {
    m_selectedArea = QRect();
    emit selectionChanged( m_selectedArea );
  }
  update();
}

void ViewArea::setGridVisible( bool enable )
{
  m_bGridVisible = enable;
  update();
}

void ViewArea::setGridSize( int size )
{
  m_grid.setGridSize( size );
  update();
}

void ViewArea::clearMask()
{
  m_mask.clear();
}

/**
 * Zoom related function
 */
void ViewArea::startZoomWinTimer()
{
  m_zoomWinTimer.start();
}

void ViewArea::setZoomFactor( double f )
{
  m_dZoomFactor = f;

  updateSize();
  startZoomWinTimer();
  update();
}

double ViewArea::scaleZoomFactor( double scale, QPoint center, QSize minimumSize )
{
  double maxZoom = 100.0;
  double minZoom = 0.01;  //( 1.0 / m_pixmap.width() );
  double new_scale = 1.0;

  if( ( m_dZoomFactor == minZoom ) && ( scale < 1 ) )
    return new_scale;

  if( ( m_dZoomFactor == maxZoom ) && ( scale > 1 ) )
    return new_scale;

  double newZoomFactor = m_dZoomFactor * scale * 100.0;
  newZoomFactor = round( newZoomFactor );
  newZoomFactor = newZoomFactor / 100.0;
  scale = newZoomFactor / m_dZoomFactor;

  if( !minimumSize.isNull() )
  {
    double cw = m_pixmap.width() * m_dZoomFactor;
    double ch = m_pixmap.height() * m_dZoomFactor;
    double fw = m_pixmap.width() * newZoomFactor;
    double fh = m_pixmap.height() * newZoomFactor;
    double mw = minimumSize.width();
    double mh = minimumSize.height();

    if( ( cw < mw ) && ( ch < mh ) && ( scale < 1 ) )
    {
      return new_scale;
    }

    if( ( fw < mw ) && ( fh < mh ) && ( scale < 1 ) )
    {
      double wfactor = mw / fw;
      double hfactor = mh / fh;

      if( wfactor < hfactor )
        scale = wfactor;
      else
        scale = hfactor;

      newZoomFactor = newZoomFactor * scale * 100.0;
      newZoomFactor = floor( newZoomFactor );
      newZoomFactor = newZoomFactor / 100.0;
      scale = newZoomFactor / m_dZoomFactor;
    }
  }

  new_scale = scale;

  if( newZoomFactor < minZoom )
  {
    newZoomFactor = minZoom;
    new_scale = newZoomFactor / m_dZoomFactor;
  }
  else
  {
    if( newZoomFactor > maxZoom )
    {
      newZoomFactor = maxZoom;
      new_scale = newZoomFactor / m_dZoomFactor;
    }
  }

  setZoomFactor( newZoomFactor );

  return new_scale;
}

// void ViewArea::setMode( int mode )
//{
//  if( m_mode == mode )
//    return;
//
//  m_mode = mode;
//  update();
//}
//
// void ViewArea::setNormalMode()
//{
//  setMode( NormalMode );
//}
//
// void ViewArea::setMaskMode()
//{
//  setMode( MaskMode );
//}
//
// void ViewArea::setMaskColor( const QColor &color )
//{
//  m_maskColor = color;
//  update();
//}
//
// void ViewArea::setMaskTool()
//{
//  setMode( MaskMode );
//  setTool( MaskTool );
//  m_blockTrackEnable = true;
//}
//
// void ViewArea::setEraserTool()
//{
//  setMode( MaskMode );
//  setTool( EraserTool );
//  m_blockTrackEnable = true;
//}
//
// void ViewArea::setSelectionTool()
//{
//  setMode( NormalMode );
//  setTool( SelectionTool );
//  m_blockTrackEnable = false;
//}
//
// void ViewArea::setBlockSelectionTool()
//{
//  setMode( NormalMode );
//  setTool( BlockSelectionTool );
//  m_blockTrackEnable = true;
//}

void ViewArea::setSnapToGrid( bool enable )
{
  m_snapToGrid = enable;
}

// void ViewArea::setSelectedArea( QRect &rect )
//{
//  if( rect.isNull() )
//  {
//    update();
//    return;
//  }
//
//  QRect updateRect;
// // setNormalMode();
//
//  if( m_selectedArea.isNull() )
//  {
//    m_selectedArea = rect.normalized();
//    updateRect = m_selectedArea;
//  }
//  else
//  {
//    updateRect = m_selectedArea;
//    m_selectedArea = rect.normalized();
//    updateRect = updateRect.united( m_selectedArea );
//  }
//  updateRect.adjust( 0, 0, 1, 1 );
//  update( updateRect.normalized() );
//}

void ViewArea::initZoomWinRect()
{
  int iMinX = 80;
  int iMinY = 80;

  int iMaxX = iMinX * 5;
  int iMaxY = iMinY * 5;

  double dSizeRatio, dWinZoomRatio;

  int iHorizontalImg = ( m_pixmap.width() > m_pixmap.height() ) ? 1 : 0;

  if( iHorizontalImg )
  {
    dSizeRatio = (double)m_pixmap.width() / (double)m_pixmap.height();
  }
  else
  {
    dSizeRatio = (double)m_pixmap.height() / (double)m_pixmap.width();
  }

  if( dSizeRatio > 5.0 )
  {
    if( iHorizontalImg )
    {
      dWinZoomRatio = (double)( m_pixmap.width() * 1024 / iMaxX ) / 1000.0;
    }
    else
    {
      dWinZoomRatio = (double)( m_pixmap.height() * 1024 / iMaxY ) / 1000.0;
    }
  }
  else
  {
    if( iHorizontalImg )
    {
      dWinZoomRatio = (double)( m_pixmap.height() * 1024 / iMinY ) / 1000.0;
    }
    else
    {
      dWinZoomRatio = (double)( m_pixmap.width() * 1024 / iMinX ) / 1000.0;
    }
  }

  m_dZoomWinRatio = dWinZoomRatio;
}

////////////////////////////////////////////////////////////////////////////////
//                            Geometry Updates
////////////////////////////////////////////////////////////////////////////////
void ViewArea::updateSize()
{
  int w = m_pixmap.width() * m_dZoomFactor;
  int h = m_pixmap.height() * m_dZoomFactor;
  setMinimumSize( w, h );

  QWidget* p = parentWidget();
  if( p )
  {
    // If the parent size is bigger than the minimum area to view the
    // image, resize() will call resizeEvent(); otherwise, we need to
    // perform the necessary updates (updateOffset).
    resize( p->width(), p->height() );
  }

  if( w <= width() && h <= height() )
    updateOffset();
}

void ViewArea::updateOffset()
{
  if( width() > m_pixmap.width() * m_dZoomFactor )
  {
    m_xOffset = ( width() - m_pixmap.width() * m_dZoomFactor ) / 2;
  }
  else
  {
    m_xOffset = 0;
  }
  if( height() > m_pixmap.height() * m_dZoomFactor )
  {
    m_yOffset = ( height() - m_pixmap.height() * m_dZoomFactor ) / 2;
  }
  else
  {
    m_yOffset = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
//                              Resize Event
////////////////////////////////////////////////////////////////////////////////
void ViewArea::resizeEvent( QResizeEvent* event )
{
  if( size().isEmpty() || m_pixmap.isNull() )
    return;

  updateOffset();
  startZoomWinTimer();
  update();
}
////////////////////////////////////////////////////////////////////////////////
//                              Paint Event
////////////////////////////////////////////////////////////////////////////////

void ViewArea::paintEvent( QPaintEvent* event )
{
  QRect winRect = event->rect();

  if( visibleRegion().isEmpty() )
    return;

  if( size().isEmpty() || m_pixmap.isNull() )
    return;

  QSize windowSize = parentWidget()->size();
  QPainter painter( this );

  // Save the actual painter properties and scales the coordinate system.
  painter.save();
  painter.translate( m_xOffset, m_yOffset );
  painter.scale( m_dZoomFactor, m_dZoomFactor );

  // This line is for fast paiting. Only visible area of the image is painted.
  // We take the exposed rect from the event (that gives us scroll/expose
  // optimizations for free – no need
  // to draw the whole pixmap if your widget is only partially exposed), and
  // reverse map it with the painter matrix.
  // That gives us the part of the pixmap that has actually been exposed.
  // See:
  // http://blog.qt.digia.com/blog/2006/05/13/fast-transformed-pixmapimage-drawing/
  QRect exposedRect = painter.worldTransform().inverted().mapRect( event->rect() ).adjusted( -1, -1, 1, 1 );
  // Draw the pixmap.
  painter.drawPixmap( exposedRect, m_pixmap, exposedRect );

  // Draw the Grid if it's visible.
  if( m_bGridVisible )
  {
    // Do we need to draw the whole grid?
    // To know that, we need to perform a transformation of the rectangle
    // area that the painter needs to update - transform the windows
    // coordinates (origin at the top-left corner of the widget), to the
    // relatives coordinates of the image at it's original size (origin at
    // the top-left corner of the image).
    QRect vr = windowToView( winRect );

    // Now we have the (to update) rectangle area on a coordinates system
    // that has it's origin at the top-left corner of the image. That
    // is, is referenced to the not scaled image.
    // To know what image area we need to update, just intersects the
    // rectangle area with the image area.
    vr &= QRect( 0, 0, m_pixmap.width(), m_pixmap.height() );

    // Set up for the grid drawer.
    painter.setRenderHint( QPainter::Antialiasing );

    // Draw grid.
    m_grid.drawGrid( m_pixmap, vr, &painter );
  }

  painter.restore();

  // Draw a border around the image.
  /*  if( m_xOffset || m_yOffset )
	 {
	 painter.setPen( Qt::black );
	 painter.drawRect( m_xOffset - 1, m_yOffset - 1, m_pixmap.width() *
	 m_dZoomFactor + 1, m_pixmap.height() *
	 m_dZoomFactor + 1 );
	 }*/

  int frFormat = m_pcCurrFrame->getColorSpace();

  // Draw pixel values in grid
  if( m_dZoomFactor >= 60.0 || ( frFormat != CLP_COLOR_RGBA && m_dZoomFactor >= 50.0 ) )
  {
    int imageWidth = m_pixmap.width();
    int imageHeight = m_pixmap.height();
    ;

    QFont font( "Helvetica" );
    font.setPixelSize( 12 );
    painter.setFont( font );

    QRect vr = windowToView( winRect );
    vr &= QRect( 0, 0, imageWidth, imageHeight );

    for( int i = vr.x(); i <= vr.right(); i++ )
    {
      for( int j = vr.y(); j <= vr.bottom(); j++ )
      {
        QPoint pixelTopLeft( i, j );

        QRect pixelRect( viewToWindow( pixelTopLeft ), QSize( m_dZoomFactor, m_dZoomFactor ) );
        QString pixelValue;
        CalypPixel pixel = m_pcCurrFrame->getPixel( pixelTopLeft.x(), pixelTopLeft.y() );

        if( frFormat == CLP_COLOR_YUV || frFormat == CLP_COLOR_GRAY )
        {
          if( pixel[0] < m_uiPixelHalfScale )
            painter.setPen( QColor( Qt::white ) );
          else
            painter.setPen( QColor( Qt::black ) );

          pixelValue = QString( "Y: %1" ).arg( pixel[0] );
          if( frFormat == CLP_COLOR_YUV )
            pixelValue += QString( "\nU: %1\nV: %2" ).arg( pixel[1] ).arg( pixel[2] );
        }

        if( frFormat == CLP_COLOR_RGB || frFormat == CLP_COLOR_RGBA )
        {
          if( ( pixel[0] + pixel[1] + pixel[2] ) < ( m_uiPixelHalfScale * 3 ) )
            painter.setPen( QColor( Qt::white ) );
          else
            painter.setPen( QColor( Qt::black ) );

          pixelValue = QString( "R: %1\nG: %2\nB: %3" ).arg( pixel[0] ).arg( pixel[1] ).arg( pixel[2] );
          if( frFormat == CLP_COLOR_RGBA )
            pixelValue += QString( "\nA: %1" ).arg( pixel[3] );
        }
        painter.drawText( pixelRect, Qt::AlignCenter, pixelValue );
      }
    }

    QColor color( Qt::white );
    QPen mainPen = QPen( color, 1, Qt::SolidLine );
    painter.setPen( mainPen );

    // Draw vertical line
    for( int x = vr.x(); x <= ( vr.right() + 1 ); x++ )
    {
      // Always draw the full line otherwise the line stippling
      // varies with the location of view area and we get glitchy
      // patterns.
      painter.drawLine( viewToWindow( QPoint( x, 0 ) ), viewToWindow( QPoint( x, imageHeight ) ) );
    }
    // Draw horizontal line
    for( int y = vr.y(); y <= ( vr.bottom() + 1 ); y++ )
    {
      painter.drawLine( viewToWindow( QPoint( 0, y ) ), viewToWindow( QPoint( imageWidth, y ) ) );
    }
  }
  bool showZoomRect = m_pixmap.width() * m_dZoomFactor > windowSize.width() ||
                      m_pixmap.height() * m_dZoomFactor > windowSize.height();

  // VISIBLE ZOOM RECT
  if( m_visibleZoomRect && m_zoomWinTimer.isActive() && showZoomRect )
  {
    double dRatio = m_dZoomWinRatio;

    QRect cImg = QRect( 0, 0, m_pixmap.width(), m_pixmap.height() );
    QPoint cZWinRBpos = QPoint( winRect.bottomRight() ) - QPoint( 15, 15 );
    QRect cImgWinRect( 0, 0, round( (double)cImg.width() / dRatio ), round( (double)cImg.height() / dRatio ) );
    cImgWinRect.moveBottomRight( cZWinRBpos );

    QRect vr = windowToView( winRect );
    QRect cVisibleImg = vr & cImg;
    //cVisibleImg.moveTopLeft(vr.topLeft());
    QRect cVisibleWinRect;
    cVisibleWinRect.setLeft( floor( (double)cVisibleImg.x() / dRatio ) );
    cVisibleWinRect.setTop( floor( (double)cVisibleImg.y() / dRatio ) );
    cVisibleWinRect.setRight( round( (double)cVisibleImg.right() / dRatio ) );
    cVisibleWinRect.setBottom( round( (double)cVisibleImg.bottom() / dRatio ) );

    //painter.fillRect( cImgWinRect, QBrush( QColor( 128, 128, 128, 128 ) ) );
    painter.setPen( QColor( 50, 50, 50, 128 ) );
    //painter.drawRect( cImgWinRect );
    painter.setOpacity( 0.7 );
    painter.drawPixmap( cImgWinRect, m_pixmap );
    painter.setOpacity( 1 );

    cVisibleWinRect.moveTopLeft( cImgWinRect.topLeft() + cVisibleWinRect.topLeft() );

    cVisibleWinRect = cVisibleWinRect & cImgWinRect;

    if( cVisibleWinRect.left() < 0 )
      cVisibleWinRect.moveLeft( 0 );
    if( cVisibleWinRect.top() < 0 )
      cVisibleWinRect.moveTop( 0 );

    if( cVisibleWinRect.width() <= 0 )
      cVisibleWinRect.setWidth( 1 );
    if( cVisibleWinRect.height() <= 0 )
      cVisibleWinRect.setHeight( 1 );

    painter.fillRect( cVisibleWinRect, QBrush( QColor( 200, 200, 200, 128 ) ) );
    painter.setPen( QColor( 255, 255, 255, 128 ) );
    painter.drawRect( cVisibleWinRect );

    //qDebug() << "Debug VisibleZoomRect: " << winRect << vr << cImgWinRect << cVisibleWinRect << cVisibleImg << dRatio;
  }

  QRect sr = viewToWindow( m_selectedArea );
  QRect ir = sr & winRect;

  if( ( mode() == NormalMode ) && !selectedArea().isNull() )
  {
    if( m_newShape || m_blockTrackEnable )
    {
      // Set the tool color
      QColor selectColor = selectionColor;

      selectColor.setAlpha( 120 );

      QBrush brush( selectColor );

      if( m_blockTrackEnable )
        painter.setBrush( Qt::NoBrush );
      else
        painter.setBrush( brush );

      painter.setPen( Qt::darkCyan );

      //             painter.drawRect( sr/*m_selectedArea*/ );
    }
    else
    {
      // 1) Cover the image with a light gray except the selected area
      QColor fill( Qt::lightGray );
      fill.setAlpha( 150 );

      QBrush brush( fill );
      painter.setBrush( fill );
      painter.setPen( Qt::NoPen );
      QPainterPath myPath;
      QRect imgr = viewToWindow( QRect( 0, 0, m_pixmap.width(), m_pixmap.height() ) );

      myPath.addRect( imgr );
      myPath.addRect( sr );  // m_selectedArea
      painter.drawPath( myPath );

      // 2) Draw the selection rectangle
      painter.setBrush( Qt::NoBrush );
      painter.setPen( Qt::darkCyan );
      //             painter.drawRect( sr/*m_selectedArea*/ );
    }

    if( !ir.isNull() )
      painter.drawRect( sr /*m_selectedArea*/ );
  }

  // Mask Mode ----------------------------------------------------------

  if( mode() != MaskMode )
    return;

  QColor color;
  QBrush brush;

  // Draw Mask
  // Set the tool color
  if( m_maskColor.isValid() )
    color = m_maskColor;
  else
    color = imageMaskColor;

  color.setAlpha( 120 );
  painter.setPen( color );
  painter.save();
  painter.translate( m_xOffset, m_yOffset );
  painter.scale( m_dZoomFactor, m_dZoomFactor );
  painter.drawPixmap( QPoint( 0, 0 ), m_mask );
  painter.restore();

  if( !m_selectedArea.isNull() )
  {
    if( m_newShape || m_blockTrackEnable )
    {
      switch( tool() )
      {
      case MaskTool: {
        if( m_maskColor.isValid() )
          color = m_maskColor;
        else
          color = imageMaskColor;

        break;
      }
      case EraserTool: {
        color = eraserColor;
        break;
      }
      default:
        color = selectionColor;  // ?Problems!
      }

      color.setAlpha( 120 );
      brush = QBrush( color );

      if( m_blockTrackEnable )
        painter.setBrush( Qt::NoBrush );
      else
        painter.setBrush( brush );

      painter.setPen( color );
      if( !ir.isNull() )
        painter.drawRect( sr );
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//                              Mouse Events
////////////////////////////////////////////////////////////////////////////////
void ViewArea::wheelEvent( QWheelEvent* event )
{
  double scale;
  double usedScale;

  event->accept();

  if( event->modifiers() & Qt::ControlModifier )
  {
    scale = 0.001 * event->delta();
    if( scale > 0 )
      scale = 1.25;
    else
      scale = 0.8;

    QWidget* p = parentWidget();
    QSize minimumSize = QSize( p->size().width() - 5, p->size().height() - 5 );
    usedScale = scaleZoomFactor( scale, event->pos(), minimumSize );
    if( usedScale != 1.0 )
    {
      emit zoomFactorChanged_byWheel( usedScale, event->pos() );
    }
  }
}

void ViewArea::mousePressEvent( QMouseEvent* event )
{
  event->accept();

  QPoint vpos = windowToView( event->pos() );

  if( event->button() == Qt::MidButton )
  {
  }
  if( event->button() == Qt::LeftButton )
  {
    if( tool() == NavigationTool )
    {
      if( !( m_xOffset && m_yOffset ) )
        setCursor( Qt::ClosedHandCursor );

      m_lastWindowPos = event->pos();
    }
    // Is the mouse over the image? If yes, save the mouse position;
    // otherwise, QPoint( -1, -1 ) indicates an invalid position.
    if( isPosValid( vpos ) )
    {
      m_lastPos = vpos;
    }
    else
    {
      m_lastPos = QPoint( -1, -1 );
      return;
    }

    // If grid tracking
    if( m_snapToGrid )
    {
      // Find if the cursor is near a grid intersection
      bool isNear = m_grid.isNear( m_lastPos );
      isNear = true;
      if( isNear )
      {
        // The grid 'near intersection' found when used isNear()
        m_lastPos = m_grid.nearPos();
      }
    }

    m_newShape = true;

    if( tool() == SelectionTool )
    {
      m_blockTrackEnable = false;
      return;
    }

    if( mode() == MaskMode )
    {
      // Disable the block track to enable fill the block
      m_blockTrackEnable = false;
    }

    // Block selection
    //    m_selectedArea = m_grid.rectContains( m_lastPos );

    QRect updateRect = viewToWindow( m_selectedArea );
    updateRect.adjust( 0, 0, 1, 1 );
    update( updateRect.normalized() );
  }
}

void ViewArea::mouseMoveEvent( QMouseEvent* event )
{
  event->accept();

  //#ifndef _MSC_VER
  //  // Add this code line to avoid slow navigation with some specific mouses
  //  // This seems to always appear on windows
  //  if( qApp->hasPendingEvents() )
  //    return;
  //#endif

  QPoint actualPos = windowToView( event->pos() );
  QRect updateRect;

  emit positionChanged( actualPos );

  // If mouse left button pressed
  if( event->buttons() == Qt::LeftButton && m_lastPos != QPoint( -1, -1 ) )
  {
    // Grid tracking
    if( m_snapToGrid )
    {
      // Find if the cursor is near a grid intersection
      bool isNear = m_grid.isNear( actualPos );

      if( isNear )
      {
        // Return the last grid near intersection found
        actualPos = m_grid.nearPos();
      }
    }

    if( tool() == NavigationTool )
    {
      QPoint offset = m_lastWindowPos - event->pos();

      startZoomWinTimer();
      emit scrollBarMoved( offset );
    }

    updateRect = viewToWindow( m_selectedArea );

    if( tool() == SelectionTool )
    {
      // If the selection is only vertical or horizontal then we have one
      // of the dimentions null.
      if( actualPos.x() == m_lastPos.x() && actualPos.y() == m_lastPos.y() )
      {
        m_selectedArea = QRect();
      }
      // Selection from top to bottom
      else if( m_lastPos.y() < actualPos.y() )
      {
        // From left to right
        if( m_lastPos.x() < actualPos.x() )
        {
          QPoint bottomR = actualPos;  // - QPoint( 1, 1 );
          m_selectedArea = QRect( m_lastPos, bottomR );
        }
        // From right to left
        else
        {
          QPoint topL( actualPos.x(), m_lastPos.y() );
          QPoint bottomR( m_lastPos.x(), actualPos.y() );

          m_selectedArea = QRect( topL, bottomR );
        }
      }
      // Selection from bottom to top
      else
      {
        // From left to right
        if( m_lastPos.x() < actualPos.x() )
        {
          QPoint topL( m_lastPos.x(), actualPos.y() );
          QPoint bottomR( actualPos.x(), m_lastPos.y() );
          m_selectedArea = QRect( topL, bottomR );
        }
        // From right to left
        else
        {
          QPoint bottomR = m_lastPos;  // - QPoint( 1, 1 );
          m_selectedArea = QRect( actualPos, bottomR );
        }
      }
    }
    /*    else // if tool() == BlockSelectionTool || MaskTool || EraserTool
		 {
		 m_blockTrackEnable = false;
		 // If cursor is inside the selected area, we most redraw
		 // the selection rect because it may be smaller.
		 if( m_selectedArea.contains( actualPos ) )
		 {
		 m_selectedArea = m_grid.rectContains( m_lastPos );
		 }

		 m_selectedArea = m_selectedArea.united( m_grid.rectContains( actualPos ) );
		 }
		 */
    if( tool() == SelectionTool )
    {
      // intercept the selected area with the image area to limit the
      // selection only to the image area, preventing it to come outside
      // the image.
      m_selectedArea &= QRect( 0, 0, m_pixmap.width(), m_pixmap.height() );

      // Update only the united area
      //      updateRect = updateRect.united( viewToWindow( m_selectedArea ) );

      // "When rendering with a one pixel wide pen the QRect's
      // boundary line will be rendered to the right and below the
      // mathematical rectangle's boundary line.", in QT4 doc.
      // Our selection pen width is 1, let's adjust the rendering area.
      //      updateRect.adjust( 0, 0, 1, 1 );

      //      update( updateRect.normalized() );
      update();
    }
    return;
  }

  // If no mouse button is pressed and bloks selection tool is set, then
  // track the bloks.
  if( m_blockTrackEnable )
  {
    updateRect = viewToWindow( m_selectedArea );

    if( isPosValid( actualPos ) )
    {
      m_selectedArea = m_grid.rectContains( actualPos );
    }
    else
    {
      m_selectedArea = QRect();
    }
    updateRect = updateRect.united( viewToWindow( m_selectedArea ) );
    updateRect.adjust( 0, 0, 1, 1 );
    update( updateRect );
    return;
  }
}

void ViewArea::mouseReleaseEvent( QMouseEvent* event )
{
  event->accept();

  QPoint vpos = windowToView( event->pos() );

  if( event->button() == Qt::LeftButton && m_lastPos != QPoint( -1, -1 ) )
  {
    if( tool() == NavigationTool )
    {
      unsetCursor();
    }
    else if( tool() == SelectionTool )
    {
      // Normal Mode ------------------------------------------------------
      if( mode() == NormalMode )
      {
        if( vpos == m_lastPos )
        {
          m_selectedArea = QRect();
        }
        emit selectionChanged( m_selectedArea );
      }

      // Mask Mode -----------------------------------------------------------
      else if( mode() == MaskMode )
      {
        if( !m_selectedArea.isNull() )
        {
          updateMask( m_selectedArea );
        }

        m_selectedArea = QRect();
        m_blockTrackEnable = true;
      }
      m_newShape = false;
      update();
      m_lastPos = QPoint( -1, -1 );
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

bool ViewArea::isPosValid( const QPoint& pos ) const
{
  if( pos.x() < 0 || pos.y() < 0 || pos.x() >= m_pixmap.width() || pos.y() >= m_pixmap.height() )
    return false;
  else
    return true;
}

////////////////////////////////////////////////////////////////////////////////

QPoint ViewArea::windowToView( const QPoint& pt ) const
{
  QPoint p;
  p.setX( static_cast<int>( ( pt.x() - m_xOffset ) / m_dZoomFactor ) );
  p.setY( static_cast<int>( ( pt.y() - m_yOffset ) / m_dZoomFactor ) );

  return p;
}

QRect ViewArea::windowToView( const QRect& rc ) const
{
  QRect r;

  r.setTopLeft( windowToView( rc.topLeft() ) );
  //     r.setRight ( (int)( ceil(( rc.right()  - m_xOffset)/m_dZoomFactor  )));
  //     r.setBottom( (int)( ceil(( rc.bottom()- m_yOffset)/m_dZoomFactor  )));
  //     r.setRight ( static_cast<int>(( rc.right() - m_xOffset ) /
  //     m_dZoomFactor +1));
  //     r.setBottom( static_cast<int>(( rc.bottom() - m_xOffset ) /
  //     m_dZoomFactor+1));
  r.setBottomRight( windowToView( rc.bottomRight() ) );
  return r;
}

QPoint ViewArea::viewToWindow( const QPoint& pt ) const
{
  QPoint p;

  p.setX( static_cast<int>( pt.x() * m_dZoomFactor + m_xOffset ) );
  p.setY( static_cast<int>( pt.y() * m_dZoomFactor + m_yOffset ) );

  return p;
}

QRect ViewArea::viewToWindow( const QRect& rc ) const
{
  QRect r;

  r.setTopLeft( viewToWindow( rc.topLeft() ) );
  //     r.setRight ( (int)( ceil(( rc.right() +1+m_xOffset )*m_dZoomFactor ) -
  //     1 ));
  //     r.setBottom( (int)( ceil(( rc.bottom()+1+m_yOffset )*m_dZoomFactor ) -
  //     1 ));
  //     r.setRight ( (int)( ceil(( rc.right()+0.5)*m_dZoomFactor  )+ m_xOffset
  //     )-1);
  //     r.setBottom( (int)( ceil(( rc.bottom()+0.5)*m_dZoomFactor ) +m_yOffset
  //     )-1);
  // qDebug()<<"Right = "<< r.right();
  //     r.setRight ( static_cast<int>(( rc.right()+1) * m_dZoomFactor +
  //     m_xOffset -1) );
  //     r.setBottom( static_cast<int>(( rc.bottom()+1) * m_dZoomFactor +
  //     m_yOffset -1));
  r.setBottomRight( viewToWindow( rc.bottomRight() ) );

  return r;
}

////////////////////////////////////////////////////////////////////////////////
//                           Masks Management
////////////////////////////////////////////////////////////////////////////////
void ViewArea::updateMask( const QRect& rect )
{
  switch( tool() )
  {
  case MaskTool: {
    // Add rect to the mask
    QPainter painter( &m_mask );
    painter.setBrush( Qt::color1 );
    painter.setPen( Qt::NoPen );
    painter.drawRect( rect );
    painter.end();
    break;
  }
  case EraserTool: {
    // Clears rect area in the mask
    QPainter painter( &m_mask );
    painter.setBrush( Qt::color0 );
    painter.setPen( Qt::NoPen );
    painter.drawRect( rect );
    painter.end();
    break;
  }
  default: /* Do Nothing */
           ;
  }
}

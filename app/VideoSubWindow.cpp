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
 * \file     VideoSubWindow.cpp
 * \brief    Video Sub windows handling
 */

#include "VideoSubWindow.h"

#include <QScrollArea>
#include <QSettings>
#include <QStaticText>
#include <cassert>

#include "ConfigureFormatDialog.h"
#include "ModulesHandle.h"
#include "SubWindowAbstract.h"

VideoInformation::VideoInformation( QWidget* parent )
    : QWidget( parent ), m_bBusyWindow( false )
{
  setPalette( Qt::transparent );
  setAttribute( Qt::WA_TransparentForMouseEvents );
  m_cTopLeftTextFont.setPointSize( 8 );
  m_cCenterTextFont.setPointSize( 12 );
  m_pcRefreshTimer = new QTimer;
  m_pcRefreshTimer->setSingleShot( true );
}

void VideoInformation::setInformationTopLeft( const QStringList& textLines )
{
  m_cTopLeftTextList.clear();
  for( int i = 0; i < textLines.size(); i++ )
  {
    m_cTopLeftTextList.append( QStaticText( textLines.at( i ) ) );
  }
}

void VideoInformation::setBusyWindow( bool bFlag )
{
  m_bBusyWindow = bFlag;
  // Do not show the refresh screen if it takes less than 500 ms
  m_pcRefreshTimer->start( 500 );
}

void VideoInformation::paintEvent( QPaintEvent* event )
{
  QPainter painter( this );
  painter.setFont( m_cTopLeftTextFont );
  if( !m_cTopLeftTextList.isEmpty() && size().width() > 300 )
  {
    QPoint topLeftCorner( 10, 10 );
    for( int i = 0; i < m_cTopLeftTextList.size(); i++ )
    {
      painter.drawStaticText( topLeftCorner, m_cTopLeftTextList.at( i ) );
      topLeftCorner += QPoint( 0, 15 );
    }
  }
  if( m_bBusyWindow && !m_pcRefreshTimer->isActive() )
  {
    painter.setFont( m_cCenterTextFont );
    painter.drawText( rect(), Qt::AlignHCenter | Qt::AlignVCenter, QStringLiteral( "Refreshing..." ) );
    painter.fillRect( rect(), QBrush( QColor::fromRgb( 255, 255, 255, 50 ), Qt::SolidPattern ) );
  }
}

VideoSubWindow::VideoSubWindow( enum VideoSubWindowCategories category, QWidget* parent )
    : SubWindowAbstract( parent, SubWindowAbstract::VIDEO_SUBWINDOW | category )
    , m_pcReferenceSubWindow( NULL )
    , m_pcUpdateTimer( NULL )
{
  // Create a new scroll area inside the sub-window
  m_pcScrollArea = new QScrollArea;
  connect( m_pcScrollArea->horizontalScrollBar(), SIGNAL( actionTriggered( int ) ), this,
           SLOT( updateScrollValues() ) );
  connect( m_pcScrollArea->verticalScrollBar(), SIGNAL( actionTriggered( int ) ), this,
           SLOT( updateScrollValues() ) );

  // Create a new interface to show images
  m_cViewArea = new ViewArea;
  connect( m_cViewArea, SIGNAL( zoomFactorChanged_byWheel( double, QPoint ) ), this,
           SLOT( adjustScrollBarByScale( double, QPoint ) ) );

  connect( m_cViewArea, SIGNAL( zoomFactorChanged_byWheel( double, QPoint ) ), this,
           SIGNAL( zoomFactorChanged( double, QPoint ) ) );

  connect( m_cViewArea, SIGNAL( scrollBarMoved( QPoint ) ), this, SLOT( adjustScrollBarByOffset( QPoint ) ) );
  //connect( m_cViewArea, SIGNAL( scrollBarMoved( QPoint ) ), this, SIGNAL( scrollBarMoved( QPoint ) ) );

  connect( m_cViewArea, SIGNAL( selectionChanged( QRect ) ), this, SLOT( updateSelectedArea( QRect ) ) );
  connect( m_cViewArea, SIGNAL( positionChanged( const QPoint& ) ), this,
           SLOT( updatePixelValueStatusBar( const QPoint& ) ) );

  m_pcScrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  m_pcScrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

  // Define the cViewArea as the widget inside the scroll area
  m_pcScrollArea->setWidget( m_cViewArea );
  m_pcScrollArea->setWidgetResizable( true );
  setWidget( m_pcScrollArea );

  m_associatedModules.clear();

  m_pcUpdateTimer = new QTimer();
  m_pcUpdateTimer->setInterval( 800 );
  connect( m_pcUpdateTimer, SIGNAL( timeout() ), this, SLOT( updateWindowOnTimeout() ) );

  //! Add video information
  m_pcVideoInfo = new VideoInformation( this );
}

VideoSubWindow::~VideoSubWindow()
{
  auto tmpModules = m_associatedModules;
  m_associatedModules.clear();
  for( auto module : tmpModules )
  {
    module->disable();
  }

  m_pcCurrFrameAsset = nullptr;
  delete m_pcUpdateTimer;
}

/**
 * Events handlers
 */

void VideoSubWindow::closeEvent( QCloseEvent* event )
{
  if( m_pcDisplayModule )
  {
    m_pcDisplayModule->disable();
    event->ignore();
    return;
  }
  SubWindowAbstract::closeEvent( event );
}

void VideoSubWindow::keyPressEvent( QKeyEvent* event )
{
  if( m_pcDisplayModule )
  {
    if( m_pcDisplayModule->getModuleRequirements().is_set( ClpModuleFeature::KeysShortcuts ) )
    {
      bool bRet = false;
      switch( event->key() )
      {
      case Qt::Key_A:
        bRet = m_pcDisplayModule->getModule()->keyPressed( MODULE_KEY_LEFT );
        break;
      case Qt::Key_D:
        bRet = m_pcDisplayModule->getModule()->keyPressed( MODULE_KEY_RIGHT );
        break;
      case Qt::Key_W:
        bRet = m_pcDisplayModule->getModule()->keyPressed( MODULE_KEY_UP );
        break;
      case Qt::Key_S:
        bRet = m_pcDisplayModule->getModule()->keyPressed( MODULE_KEY_DOWN );
        break;
      }
      if( bRet )
      {
        refreshFrame();
        updateVideoWindowInfo();
        return;
      }
    }
  }
  QWidget::keyPressEvent( event );
}

void VideoSubWindow::resizeEvent( QResizeEvent* event )
{
  m_pcVideoInfo->resize( event->size() );
}

/**
 * Size related handlers
 */
QSize VideoSubWindow::sizeHint( const QSize& maxSize ) const
{
  QSize isize;
  if( m_pcCurrFrameAsset )
  {
    isize = QSize( m_pcCurrFrameAsset->getWidth() + 50, m_pcCurrFrameAsset->getHeight() + 50 );
    // If the VideoSubWindow needs more space that the avaiable, we'll give
    // to the subwindow a reasonable size preserving the image aspect ratio.
    if( !( isize.width() < maxSize.width() && isize.height() < maxSize.height() ) )
    {
      isize.scale( maxSize, Qt::KeepAspectRatio );
    }
  }
  return isize;
}

void VideoSubWindow::updateWindowOnTimeout()
{
  m_pcVideoInfo->update();
}

void VideoSubWindow::updateSelectedArea( QRect area )
{
  m_cSelectedArea = area;
}

/**
 * Functions to enable a module in the
 * current SubWindow
 */
// void VideoSubWindow::enableModule( std::shared_ptr<CalypAppModuleIf> pcModule )
// {
//   auto it = std::find_if( m_associatedModules.begin(), m_associatedModules.end(),
//                           [&]( const auto& module ) { return module.get() == pcModule.get(); } );
//   if( it != m_associatedModules.end() )
//   {
//     m_associatedModules.erase( it );
//   }
//   m_pcDisplayModule = std::move( pcModule );
//   updateVideoWindowInfo();
// }

bool VideoSubWindow::hasAssociatedModule()
{
  if( m_pcDisplayModule == nullptr )
    return !m_associatedModules.empty();
  return m_associatedModules.size() > 1;
}

void VideoSubWindow::setDisplayModule( CalypAppModuleIf* pcModule )
{
  if( m_pcDisplayModule != nullptr )
  {
    m_pcDisplayModule->disable();
  }
  m_pcDisplayModule = pcModule;
  updateVideoWindowInfo();
}

void VideoSubWindow::associateModule( std::shared_ptr<CalypAppModuleIf> pcModule )
{
  auto it = std::find_if( m_associatedModules.begin(), m_associatedModules.end(),
                          [&]( const auto& module ) { return module.get() == pcModule.get(); } );
  if( it == m_associatedModules.end() )
  {
    m_associatedModules.push_back( std::move( pcModule ) );
  }
}

void VideoSubWindow::disableModule( CalypAppModuleIf* pcModule )
{
  if( pcModule == nullptr )
    return;
  bool bRefresh = false;

  auto it = std::find_if( m_associatedModules.begin(), m_associatedModules.end(),
                          [&]( const auto& module ) { return module.get() == pcModule; } );
  if( it != m_associatedModules.end() )
  {
    m_associatedModules.erase( it );
  }
  if( pcModule == m_pcDisplayModule )
  {
    bRefresh |= true;
    m_pcDisplayModule = nullptr;
    resetWindowName();
  }
  if( bRefresh )
  {
    refreshSubWindow();
    updateVideoWindowInfo();
  }
  // if( pcModule )
  // {
  //   int modIdx = m_apcCurrentModule.indexOf( pcModule );
  //   if( modIdx != -1 )
  //   {
  //     m_apcCurrentModule.removeAt( modIdx );
  //     ModulesHandle::destroyModuleIf( pcModule );
  //     bRefresh |= true;
  //   }
  //   if( pcModule == m_pcDisplayModule )
  //   {
  //     pcModule = m_pcDisplayModule;
  //     m_pcDisplayModule = 0;
  //     ModulesHandle::destroyModuleIf( pcModule );
  //     setWindowName( QFileInfo( m_cFilename ).fileName() );
  //     bRefresh |= true;
  //   }
  // }
  // else
  // {
  //   QList<CalypAppModuleIf*> apcCurrentModule = m_apcCurrentModule;
  //   for( int i = 0; i < apcCurrentModule.size(); i++ )
  //   {
  //     m_apcCurrentModule.removeOne( apcCurrentModule.at( i ) );
  //     ModulesHandle::destroyModuleIf( apcCurrentModule.at( i ) );
  //     bRefresh |= true;
  //   }
  //   assert( m_apcCurrentModule.size() == 0 );
  //   if( m_pcDisplayModule )
  //   {
  //     pcModule = m_pcDisplayModule;
  //     m_pcDisplayModule = NULL;
  //     ModulesHandle::destroyModuleIf( pcModule );
  //     setWindowName( QFileInfo( m_cFilename ).fileName() );
  //     bRefresh |= true;
  //   }
  // }
}

bool VideoSubWindow::disableAllModules()
{
  if( m_associatedModules.empty() )
    return false;
  // For loop is required as we will invalidate the iterator
  while( m_associatedModules.size() > 0 )
  {
    auto module = m_associatedModules.back();
    m_associatedModules.pop_back();
    module->disable();
  }
  return true;
}

bool VideoSubWindow::hasRunningModule()
{
  bool bRet = false;
  for( std::size_t i = 0; i < m_associatedModules.size() && !bRet; i++ )
  {
    bRet |= m_associatedModules.at( i )->isRunning();
  }
  return bRet;
}

void VideoSubWindow::setFillWindow( bool bFlag )
{
  m_pcVideoInfo->setBusyWindow( bFlag );
}

void VideoSubWindow::setCurrFrame( std::shared_ptr<CalypFrame> pcCurrFrame )
{
  m_pcCurrFrameAsset = std::move( pcCurrFrame );
  refreshFrame();
}

void VideoSubWindow::refreshFrame()
{
  m_cViewArea->setImage( m_pcCurrFrameAsset );
  for( auto& module : m_associatedModules )
  {
    if( module.get() != m_pcDisplayModule )
      module->update( isPlaying() );
  }
}

bool VideoSubWindow::save( QString filename )
{
  bool iRet = false;
  QApplication::setOverrideCursor( Qt::WaitCursor );

  std::unique_ptr<CalypFrame> new_frame{ nullptr };
  CalypFrame* saveFrame = m_pcCurrFrameAsset.get();
  if( m_cSelectedArea.isValid() )
  {
    new_frame = std::make_unique<CalypFrame>( m_pcCurrFrameAsset.get(), m_cSelectedArea.x(), m_cSelectedArea.y(), m_cSelectedArea.width(),
                                              m_cSelectedArea.height() );
    saveFrame = new_frame.get();
  }
  if( !saveFrame )
  {
    return false;
  }
  iRet = CalypStream::saveFrame( filename.toStdString(), *saveFrame );
  QApplication::restoreOverrideCursor();
  return iRet;
}

QSize VideoSubWindow::getScrollSize()
{
  return QSize( m_pcScrollArea->viewport()->size().width() - 5, m_pcScrollArea->viewport()->size().height() - 5 );
}

void VideoSubWindow::adjustScrollBarToRatio( const double& horRatio, const double& verRatio )
{
  QScrollBar* scrollBarH = m_pcScrollArea->horizontalScrollBar();
  QScrollBar* scrollBarV = m_pcScrollArea->verticalScrollBar();
  scrollBarH->setValue( scrollBarH->maximum() * horRatio );
  scrollBarV->setValue( scrollBarV->maximum() * verRatio );
  updateScrollValues();
}

void VideoSubWindow::adjustScrollBarByOffset( QPoint Offset )
{
  QPoint cLastScroll = m_cCurrScroll;
  QScrollBar* scrollBarH = m_pcScrollArea->horizontalScrollBar();
  QScrollBar* scrollBarV = m_pcScrollArea->verticalScrollBar();

  int valueX = int( cLastScroll.x() + Offset.x() );
  int valueY = int( cLastScroll.y() + Offset.y() );

  scrollBarH->setValue( valueX );
  scrollBarV->setValue( valueY );

  updateScrollValues();

  scrollBarMoved( m_dHorScroll, m_dVerScroll );
}

// This function was developed with help of the schematics presented in
// http://stackoverflow.com/questions/13155382/jscrollpane-zoom-relative-to-mouse-position
void VideoSubWindow::adjustScrollBarByScale( double scale, QPoint center )
{
  QPoint cLastScroll = m_cCurrScroll;
  QScrollBar* scrollBarH = m_pcScrollArea->horizontalScrollBar();
  QScrollBar* scrollBarV = m_pcScrollArea->verticalScrollBar();

  if( center.isNull() )
  {
    m_cCurrScroll.setX( int( scale * cLastScroll.x() + ( ( scale - 1 ) * scrollBarH->pageStep() / 2 ) ) );
    m_cCurrScroll.setY( int( scale * cLastScroll.y() + ( ( scale - 1 ) * scrollBarV->pageStep() / 2 ) ) );
  }
  else
  {
    int value;

    int x = center.x() - cLastScroll.x();
    value = int( scale * cLastScroll.x() + ( ( scale - 1 ) * x ) );
    m_cCurrScroll.setX( value );

    int y = center.y() - cLastScroll.y();
    value = int( scale * cLastScroll.y() + ( ( scale - 1 ) * y ) );
    m_cCurrScroll.setY( value );
  }

  // Update window scroll
  scrollBarH->setValue( m_cCurrScroll.x() );
  scrollBarV->setValue( m_cCurrScroll.y() );

  updateScrollValues();

  scrollBarMoved( m_dHorScroll, m_dVerScroll );
}

void VideoSubWindow::updateScrollValues()
{
  double xPos = m_pcScrollArea->horizontalScrollBar()->value();
  double yPos = m_pcScrollArea->verticalScrollBar()->value();

  m_cCurrScroll.setX( xPos );
  m_cCurrScroll.setY( yPos );

  m_dHorScroll = m_dVerScroll = 0;

  if( xPos )
    m_dHorScroll = xPos / double( m_pcScrollArea->horizontalScrollBar()->maximum() );

  if( yPos )
    m_dVerScroll = yPos / double( m_pcScrollArea->verticalScrollBar()->maximum() );
}

void VideoSubWindow::normalSize()
{
  double factor = 1.0;
  double curFactor = getScaleFactor();
  if( factor != curFactor )
  {
    double usedScale;
    usedScale = m_cViewArea->scaleZoomFactor( factor / curFactor, QPoint(), QSize() );
    adjustScrollBarByScale( usedScale, QPoint() );
  }
}

void VideoSubWindow::zoomToFit()
{
  // Scale to a smaller size that the real to a nicer look
  scaleView( getScrollSize() );
}

void VideoSubWindow::zoomToFactor( double factor, QPoint center )
{
  double curFactor;
  curFactor = getScaleFactor();
  if( factor != curFactor )
  {
    double usedScale;
    usedScale = m_cViewArea->scaleZoomFactor( factor / curFactor, center, QSize() );
    adjustScrollBarByScale( usedScale, center );
  }
}

void VideoSubWindow::scaleView( double scale, QPoint center )
{
  Q_ASSERT( !m_cViewArea->image().isNull() );
  if( scale != 1.0 )
  {
    double usedScale;
    usedScale = m_cViewArea->scaleZoomFactor( scale, center, getScrollSize() );
    adjustScrollBarByScale( usedScale, center );
  }
}

void VideoSubWindow::scaleView( const QSize& size, QPoint center )
{
  QSize imgViewSize;
  if( m_pcCurrFrameAsset )
    imgViewSize = QSize( m_pcCurrFrameAsset->getWidth(), m_pcCurrFrameAsset->getHeight() );
  //else
  //  imgViewSize = QSize( m_pCurrStream->getWidth(), m_pCurrStream->getHeight() );
  QSize newSize = imgViewSize;
  newSize.scale( size, Qt::KeepAspectRatio );

  // Calc the zoom factor
  double wfactor = 1;
  double hfactor = 1;
  double factor;

  wfactor = (double)newSize.width() / imgViewSize.width();
  hfactor = (double)newSize.height() / imgViewSize.height();

  if( wfactor < hfactor )
    factor = wfactor;
  else
    factor = hfactor;

  double curFactor = getScaleFactor();
  if( factor != curFactor )
  {
    double usedScale;
    usedScale = m_cViewArea->scaleZoomFactor( factor / curFactor, center, getScrollSize() );
    adjustScrollBarByScale( usedScale, center );
  }
}

void VideoSubWindow::updatePixelValueStatusBar( const QPoint& pos )
{
  if( m_pcCurrFrameAsset )
  {
    int iWidth, iHeight;
    int posX = pos.x();
    int posY = pos.y();
    QString strStatus;

    iWidth = m_pcCurrFrameAsset->getWidth();
    iHeight = m_pcCurrFrameAsset->getHeight();

    if( ( posX < iWidth ) && ( posX >= 0 ) && ( posY < iHeight ) && ( posY >= 0 ) )
    {
      strStatus = QString( "(%1,%2)   " ).arg( posX ).arg( posY );

      int colorSpace = m_pcCurrFrameAsset->getColorSpace();
      CalypPixel pixelValue = m_pcCurrFrameAsset->getPixel( pos.x(), pos.y() );
      switch( colorSpace )
      {
      case CLP_COLOR_GRAY:
        strStatus.append( QString( "Y: %1" ).arg( pixelValue[0] ) );
        break;
      case CLP_COLOR_YUV:
        strStatus.append( QString( "Y: %1   U: %2   V: %3" )
                              .arg( pixelValue[0] )
                              .arg( pixelValue[1] )
                              .arg( pixelValue[2] ) );
        break;
      case CLP_COLOR_RGB:
        strStatus.append( QString( "R: %1   G: %2   B: %3" )
                              .arg( pixelValue[0] )
                              .arg( pixelValue[1] )
                              .arg( pixelValue[2] ) );
        break;
      case CLP_COLOR_RGBA:
        strStatus.append( QString( "R: %1   G: %2   B: %3   A: %4" )
                              .arg( pixelValue[0] )
                              .arg( pixelValue[1] )
                              .arg( pixelValue[2] )
                              .arg( pixelValue[3] ) );

        break;
      }
      emit updateStatusBar( strStatus );
    }
  }
}

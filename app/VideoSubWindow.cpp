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
#include "QtConcurrent/qtconcurrentrun.h"
#include "ResourceHandle.h"
#include "SubWindowAbstract.h"

#define QT_NO_CONCURRENT

/**
 * \brief Functions to control data stream from stream information
 */

QDataStream& operator<<( QDataStream& out, const CalypFileInfoVector& array )
{
  CalypFileInfo d;
  out << array.size();
  for( int i = 0; i < array.size(); i++ )
  {
    d = array.at( i );
    out << d.m_cFilename << d.m_uiWidth << d.m_uiHeight << d.m_iPelFormat << d.m_uiBitsPelPixel << d.m_iEndianness
        << d.m_uiFrameRate << d.m_uiFileSize;
  }
  return out;
}

QDataStream& operator>>( QDataStream& in, CalypFileInfoVector& array )
{
  CalypFileInfo d;
  int array_size{ 0 };
  in >> array_size;
  for( int i = 0; i < array_size; i++ )
  {
    in >> d.m_cFilename;
    in >> d.m_uiWidth;
    in >> d.m_uiHeight;
    in >> d.m_iPelFormat;
    in >> d.m_uiBitsPelPixel;
    in >> d.m_iEndianness;
    in >> d.m_uiFrameRate;
    in >> d.m_uiFileSize;
    array.append( d );
  }
  return in;
}

int findCalypStreamInfo( CalypFileInfoVector array, QString filename )
{
  for( int i = 0; i < array.size(); i++ )
    if( array.at( i ).m_cFilename == filename )
      return i;
  return -1;
}

/**
 * \class VideoInformation
 * \brief Shows information about video sub window
 */

class VideoInformation : public QWidget
{
private:
  QTimer* m_pcRefreshTimer;
  QList<QStaticText> m_cTopLeftTextList;
  QFont m_cTopLeftTextFont;
  QFont m_cCenterTextFont;
  bool m_bBusyWindow;

public:
  VideoInformation( QWidget* parent )
      : QWidget( parent ), m_bBusyWindow( false )
  {
    setPalette( Qt::transparent );
    setAttribute( Qt::WA_TransparentForMouseEvents );
    m_cTopLeftTextFont.setPointSize( 8 );
    m_cCenterTextFont.setPointSize( 12 );
    m_pcRefreshTimer = new QTimer;
    m_pcRefreshTimer->setSingleShot( true );
  }
  void setInformationTopLeft( const QStringList& textLines )
  {
    m_cTopLeftTextList.clear();
    for( int i = 0; i < textLines.size(); i++ )
    {
      m_cTopLeftTextList.append( QStaticText( textLines.at( i ) ) );
    }
  }
  void setBusyWindow( bool bFlag )
  {
    m_bBusyWindow = bFlag;
    // Do not show the refresh screen if it takes less than 500 ms
    m_pcRefreshTimer->start( 500 );
  }

protected:
  void paintEvent( QPaintEvent* event )
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
};

VideoSubWindow::VideoSubWindow( enum VideoSubWindowCategories category, QWidget* parent )
    : SubWindowAbstract( parent, SubWindowAbstract::VIDEO_SUBWINDOW | category )
    , m_pcResourceManager( nullptr )
    , m_bWindowBusy( false )
    , m_pCurrStream( NULL )
    , m_pcReferenceSubWindow( NULL )
    , m_bIsPlaying( false )
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

VideoSubWindow::VideoSubWindow( enum VideoSubWindowCategories category, CalypAppModuleIf* displayModule, QWidget* parent )
    : VideoSubWindow( category, parent )
{
  m_pcCurrentDisplayModule = displayModule;
}

VideoSubWindow::~VideoSubWindow()
{
  m_pcCurrFrameAsset = nullptr;
  if( m_pcResourceManager )
    m_pcResourceManager->removeResource( m_uiResourceId );
  delete m_pcUpdateTimer;
}

/**
 * Events handlers
 */

void VideoSubWindow::closeEvent( QCloseEvent* event )
{
  //stop();
  auto tmpModules = m_associatedModules;
  m_associatedModules.clear();
  for( auto module : tmpModules )
  {
    module->disable();
  }
  if( m_pcCurrentDisplayModule )
  {
    m_pcCurrentDisplayModule->disable();
    event->ignore();
    return;
  }
  SubWindowAbstract::closeEvent( event );
}

void VideoSubWindow::keyPressEvent( QKeyEvent* event )
{
  if( m_pcCurrentDisplayModule )
  {
    if( m_pcCurrentDisplayModule->getModuleRequirements() & CLP_MODULE_USES_KEYS )
    {
      bool bRet = false;
      switch( event->key() )
      {
      case Qt::Key_A:
        bRet = m_pcCurrentDisplayModule->getModule()->keyPressed( MODULE_KEY_LEFT );
        break;
      case Qt::Key_D:
        bRet = m_pcCurrentDisplayModule->getModule()->keyPressed( MODULE_KEY_RIGHT );
        break;
      case Qt::Key_W:
        bRet = m_pcCurrentDisplayModule->getModule()->keyPressed( MODULE_KEY_UP );
        break;
      case Qt::Key_S:
        bRet = m_pcCurrentDisplayModule->getModule()->keyPressed( MODULE_KEY_DOWN );
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

QSize VideoSubWindow::sizeHint() const
{
  QSize maxSize;

  QWidget* p = parentWidget();
  if( p )
  {
    maxSize = p->size();
  }
  else
  {
    maxSize = QGuiApplication::screens()[0]->availableGeometry().size();
  }
  return sizeHint( maxSize );
}

QSize VideoSubWindow::sizeHint( const QSize& maxSize ) const
{
  QSize isize;
  if( m_pcCurrFrameAsset )
    isize = QSize( m_pcCurrFrameAsset->getWidth() + 50, m_pcCurrFrameAsset->getHeight() + 50 );
  else if( m_pCurrStream )
    isize = QSize( m_pCurrStream->getWidth() + 50, m_pCurrStream->getHeight() + 50 );

  // If the VideoSubWindow needs more space that the avaiable, we'll give
  // to the subwindow a reasonable size preserving the image aspect ratio.
  if( !( isize.width() < maxSize.width() && isize.height() < maxSize.height() ) )
  {
    isize.scale( maxSize, Qt::KeepAspectRatio );
  }
  return isize;
}

void VideoSubWindow::updateWindowOnTimeout()
{
  m_pcVideoInfo->update();
}

void VideoSubWindow::loadAll()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );
  m_pCurrStream->loadAll();
  refreshFrame();
  QApplication::restoreOverrideCursor();
}

void VideoSubWindow::refreshSubWindow()
{
  if( getCategory() & SubWindowAbstract::VIDEO_STREAM_SUBWINDOW )
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
  }
  updateVideoWindowInfo();
  refreshFrame();
}

bool VideoSubWindow::loadFile( QString cFilename, bool bForceDialog )
{
  assert( m_pcResourceManager != nullptr );

  ConfigureFormatDialog formatDialog( this );
  unsigned int Width = 0, Height = 0, BitsPel = 8, FrameRate = 30;
  int Endianness = CLP_LITTLE_ENDIAN;
  int InputFormat = CLP_YUV420P;
  QSettings appSettings;

  if( m_pCurrStream )
    m_pCurrStream->getFormat( Width, Height, InputFormat, BitsPel, Endianness, FrameRate );
  else
    m_uiResourceId = m_pcResourceManager->getResource( nullptr );

  m_pCurrStream = m_pcResourceManager->getResourceAsset( m_uiResourceId );

  bool bConfig = true;
  if( !bForceDialog )
  {
    bConfig = guessFormat( cFilename, Width, Height, InputFormat, BitsPel, Endianness, FrameRate );
    if( bConfig )
    {
      // Pre-load values with last opened file
      Width = appSettings.value( "VideoSubWindow/LastWidth" ).value<unsigned int>();
      Height = appSettings.value( "VideoSubWindow/LastHeight" ).value<unsigned int>();
      BitsPel = appSettings.value( "VideoSubWindow/LastBitsPerPixel" ).value<unsigned int>();
    }
  }
  bool bRet = false;
  for( int iPass = 0; iPass < 2 && !bRet; iPass++ )
  {
    if( iPass || bConfig )
    {
      if( formatDialog.runConfigureFormatDialog( QFileInfo( cFilename ).fileName(), Width, Height, InputFormat, BitsPel,
                                                 Endianness, FrameRate ) == QDialog::Rejected )
      {
        return false;
      }
    }
    try
    {
      bRet = m_pCurrStream->open( cFilename.toStdString(), Width, Height, InputFormat, BitsPel, Endianness, FrameRate, true );
    }
    catch( CalypFailure& e )
    {
      if( iPass == 1 )
        throw( e );
    }
  }

  m_sStreamInfo.m_cFilename = cFilename;
  m_sStreamInfo.m_uiWidth = Width;
  m_sStreamInfo.m_uiHeight = Height;
  m_sStreamInfo.m_iPelFormat = InputFormat;
  m_sStreamInfo.m_uiBitsPelPixel = BitsPel;
  m_sStreamInfo.m_iEndianness = Endianness;
  m_sStreamInfo.m_uiFrameRate = FrameRate;
  m_sStreamInfo.m_uiFileSize = QFileInfo( cFilename ).size();

  QVariant var;
  var.setValue<unsigned int>( Width );
  appSettings.setValue( "VideoSubWindow/LastWidth", var );
  var.setValue<unsigned int>( Height );
  appSettings.setValue( "VideoSubWindow/LastHeight", var );
  var.setValue<unsigned int>( BitsPel );
  appSettings.setValue( "VideoSubWindow/LastBitsPerPixel", var );

#ifdef CALYP_MANAGED_RESOURCES
  m_pcResourceManager->startResourceWorker( m_uiResourceId );
#endif

  QApplication::restoreOverrideCursor();

  refreshFrame();

  m_cFilename = cFilename;

  updateVideoWindowInfo();
  setWindowName( QFileInfo( m_cFilename ).fileName() );

  return true;
}

bool VideoSubWindow::loadFile( CalypFileInfo* streamInfo )
{
  assert( m_pcResourceManager != nullptr );

  m_uiResourceId = m_pcResourceManager->getResource( m_pCurrStream );
  m_pCurrStream = m_pcResourceManager->getResourceAsset( m_uiResourceId );

  if( !m_pCurrStream->open( streamInfo->m_cFilename.toStdString(), streamInfo->m_uiWidth, streamInfo->m_uiHeight,
                            streamInfo->m_iPelFormat, streamInfo->m_uiBitsPelPixel, streamInfo->m_iEndianness,
                            streamInfo->m_uiFrameRate, true ) )
  {
    return false;
  }

  m_sStreamInfo = *streamInfo;

#ifdef CALYP_MANAGED_RESOURCES
  m_pcResourceManager->startResourceWorker( m_uiResourceId );
#endif

  QApplication::restoreOverrideCursor();

  refreshFrame();

  m_cFilename = streamInfo->m_cFilename;
  updateVideoWindowInfo();
  setWindowName( QFileInfo( m_cFilename ).fileName() );
  return true;
}

void VideoSubWindow::updateVideoWindowInfo()
{
  m_cStreamInformation = "";
  if( m_pcCurrentDisplayModule )
  {
    m_cStreamInformation = "Module";
    const auto& arraySubWindows = m_pcCurrentDisplayModule->getSubWindowList();
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
    if( m_pcCurrentDisplayModule->getModuleRequirements() & CLP_MODULES_HAS_INFO )
    {
      QStringList list = QString::fromStdString( m_pcCurrentDisplayModule->moduleInfo() ).split( '\n' );
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
      if( !m_pCurrStream->isNative() )
        m_cPelFmtName += "*";
    m_cStreamInformation += " | " + m_cPelFmtName;
  }
  if( m_cStreamInformation.isEmpty() )
  {
    m_cStreamInformation = "          ";
  }
}

bool VideoSubWindow::guessFormat( QString filename, unsigned int& rWidth, unsigned int& rHeight, int& rInputFormat, unsigned int& rBitsPerPixel,
                                  int& rEndianness, unsigned int& rFrameRate )
{
  std::vector<CalypStandardResolution> stdResList = CalypStream::stdResolutionSizes();
  bool bGuessed = true;
  bool bGuessedByFilesize = false;
  QString FilenameShort = QFileInfo( filename ).fileName();
  QString fileExtension = QFileInfo( filename ).suffix();

  if( filename.startsWith( "/dev/" ) )
  {
    return false;
  }
  if( !fileExtension.compare( "yuv", Qt::CaseInsensitive ) || !fileExtension.compare( "rgb", Qt::CaseInsensitive ) ||
      !fileExtension.compare( "gray", Qt::CaseInsensitive ) )
  {
    bGuessed = false;
    // Guess pixel format
    QVector<ClpString> formats_list = QVector<ClpString>::fromStdVector( CalypFrame::supportedPixelFormatListNames() );
    for( int i = 0; i < formats_list.size(); i++ )
    {
      if( FilenameShort.contains( formats_list.at( i ).c_str(), Qt::CaseInsensitive ) )
      {
        rInputFormat = i;
        break;
      }
    }

    if( rWidth == 0 || rHeight == 0 )
    {
      // Guess resolution - match  resolution name
      int iMatch = -1;
      for( unsigned int i = 0; i < stdResList.size(); i++ )
      {
        if( FilenameShort.contains( QString::fromStdString( stdResList[i].shortName ) ) )
        {
          iMatch = i;
        }
      }
      if( iMatch >= 0 )
      {
        rWidth = stdResList[iMatch].uiWidth;
        rHeight = stdResList[iMatch].uiHeight;
      }

      // Guess resolution - match %dx%d
      //QRegularExpressionMatch resolutionMatch = QRegularExpression( "_\\d*x\\d*" ).match( FilenameShort );
      QRegularExpressionMatch resolutionMatch = QRegularExpression( "_[0-9]+x[0-9]+" ).match( FilenameShort );
      if( resolutionMatch.hasMatch() )
      {
        QString resolutionString = resolutionMatch.captured( resolutionMatch.lastCapturedIndex() );
        if( resolutionString.startsWith( "_" ) || resolutionString.endsWith( "_" ) )
        {
          resolutionString.remove( "_" );
          QStringList resolutionArgs = resolutionString.split( "x" );
          if( resolutionArgs.size() == 2 )
          {
            rWidth = resolutionArgs.at( 0 ).toUInt();
            rHeight = resolutionArgs.at( 1 ).toUInt();
          }
        }
      }
    }

    // Guess resolution by file size
    if( rWidth == 0 && rHeight == 0 )
    {
      FILE* pF = fopen( filename.toStdString().c_str(), "rb" );
      if( pF )
      {
        fseek( pF, 0, SEEK_END );
        unsigned long long int uiFileSize = ftell( pF );
        fclose( pF );

        int count = 0, module, frame_bytes, match;
        for( unsigned int i = 0; i < stdResList.size(); i++ )
        {
          frame_bytes =
              CalypFrame::getBytesPerFrame( stdResList[i].uiWidth, stdResList[i].uiHeight, rInputFormat, 8 );
          module = uiFileSize % frame_bytes;
          if( module == 0 )
          {
            match = i;
            count++;
          }
        }
        if( count == 1 )
        {
          rWidth = stdResList[match].uiWidth;
          rHeight = stdResList[match].uiHeight;
          bGuessedByFilesize = true;
        }
      }
    }

    // [.|_]
    // Guess bits per pixel - match %dbpp
    QRegularExpressionMatch BppMatch = QRegularExpression( "_[0-9]+bpp" ).match( FilenameShort );
    if( BppMatch.hasMatch() )
    {
      QString matchString = BppMatch.captured( BppMatch.lastCapturedIndex() );
      matchString.remove( "_" );
      matchString.remove( "bpp" );
      rBitsPerPixel = matchString.toUInt();
      if( !( rBitsPerPixel > 0 && rBitsPerPixel < 16 ) )
      {
        rBitsPerPixel = -1;
      }
    }
    QRegularExpressionMatch BppOnlybMatch = QRegularExpression( "_[0-9]+b" ).match( FilenameShort );
    if( BppOnlybMatch.hasMatch() )
    {
      QString matchString = BppOnlybMatch.captured( BppOnlybMatch.lastCapturedIndex() );
      matchString.remove( "_" );
      matchString.remove( "b" );
      rBitsPerPixel = matchString.toUInt();
      if( !( rBitsPerPixel > 0 && rBitsPerPixel < 16 ) )
      {
        rBitsPerPixel = -1;
      }
    }

    // Guess frame rate - match %dbpp
    QRegularExpressionMatch FpsMatch = QRegularExpression( "_[0-9]+fps" ).match( FilenameShort );
    if( FpsMatch.hasMatch() )
    {
      QString matchString = FpsMatch.captured( FpsMatch.lastCapturedIndex() );
      matchString.remove( "_" );
      matchString.remove( "fps" );
      rFrameRate = matchString.toUInt();
      if( rFrameRate < 0 )
      {
        rFrameRate = 30;
      }
    }

    // Guess Endianness
    if( FilenameShort.contains( QStringLiteral( "be" ), Qt::CaseInsensitive ) )
    {
      rEndianness = CLP_BIG_ENDIAN;
    }
    if( FilenameShort.contains( QStringLiteral( "le" ), Qt::CaseInsensitive ) )
    {
      rEndianness = CLP_LITTLE_ENDIAN;
    }

    if( rWidth > 0 && rHeight > 0 && rInputFormat >= 0 )
      bGuessed = true && !bGuessedByFilesize;
  }
  return !bGuessed;
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
//   m_pcCurrentDisplayModule = std::move( pcModule );
//   updateVideoWindowInfo();
// }

bool VideoSubWindow::hasAssociatedModule()
{
  if( m_pcCurrentDisplayModule == nullptr )
    return !m_associatedModules.empty();
  return m_associatedModules.size() > 1;
}

void VideoSubWindow::setDisplayModule( CalypAppModuleIf* pcModule )
{
  if( m_pcCurrentDisplayModule != nullptr )
  {
    m_pcCurrentDisplayModule->disable();
  }
  m_pcCurrentDisplayModule = pcModule;
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
  if( pcModule == m_pcCurrentDisplayModule )
  {
    bRefresh |= true;
    m_pcCurrentDisplayModule = nullptr;
    setWindowName( QFileInfo( m_cFilename ).fileName() );
  }
  if( bRefresh )
  {
    refreshFrame();
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
  //   if( pcModule == m_pcCurrentDisplayModule )
  //   {
  //     pcModule = m_pcCurrentDisplayModule;
  //     m_pcCurrentDisplayModule = 0;
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
  //   if( m_pcCurrentDisplayModule )
  //   {
  //     pcModule = m_pcCurrentDisplayModule;
  //     m_pcCurrentDisplayModule = NULL;
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
    module->disable();
    m_associatedModules.pop_back();
  }
  return true;
}

bool VideoSubWindow::hasRunningModule()
{
  bool bRet = false;
  for( int i = 0; i < m_associatedModules.size() && !bRet; i++ )
  {
    bRet |= m_associatedModules.at( i )->isRunning();
  }
  return bRet;
}

void VideoSubWindow::setFillWindow( bool bFlag )
{
  m_pcVideoInfo->setBusyWindow( bFlag );
}

std::shared_ptr<CalypFrame> VideoSubWindow::getCurrFrameAsset()
{
  if( m_pcCurrFrameAsset == nullptr && m_pCurrStream != nullptr )
  {
    m_pcCurrFrameAsset = m_pCurrStream->getCurrFrameAsset();
  }
  return m_pcCurrFrameAsset;
};

void VideoSubWindow::setCurrFrame( std::shared_ptr<CalypFrame> pcCurrFrame )
{
  m_pcCurrFrameAsset = std::move( pcCurrFrame );
  m_cViewArea->setImage( m_pcCurrFrameAsset );
  for( auto& module : m_associatedModules )
  {
    if( module.get() != m_pcCurrentDisplayModule )
      module->update( m_bIsPlaying );
  }
}

void VideoSubWindow::refreshFrameOperation()
{
  bool bSetFrame = false;
  if( m_pCurrStream )
  {
    m_pcCurrFrameAsset = m_pCurrStream->getCurrFrameAsset();
    bSetFrame |= true;
  }

  if( m_pcCurrentDisplayModule )
  {
    m_bWindowBusy = true;
    bool disableThreads = !m_bIsPlaying && hasAssociatedModule();
    m_pcCurrentDisplayModule->apply( m_bIsPlaying, disableThreads );
    bSetFrame = false;
  }
  if( bSetFrame )
  {
    m_cViewArea->setImage( m_pcCurrFrameAsset );
    for( auto& module : m_associatedModules )
    {
      if( module.get() != m_pcCurrentDisplayModule )
        module->update( m_bIsPlaying );
    }
  }
}

void VideoSubWindow::refreshFrame( bool bThreaded )
{
#ifndef QT_NO_CONCURRENT
  m_cRefreshResult.waitForFinished();
  if( bThreaded )
  {
    m_cRefreshResult = QtConcurrent::run( this, &VideoSubWindow::refreshFrameOperation );
  }
  else
#endif
  {
    refreshFrameOperation();
  }
}

bool VideoSubWindow::goToNextFrame( bool bThreaded )
{
#ifndef QT_NO_CONCURRENT
  m_cRefreshResult.waitForFinished();
  m_cReadResult.waitForFinished();
#endif
#ifdef CALYP_MANAGED_RESOURCES
  while( !m_pCurrStream->hasNextFrame() && !m_pCurrStream->isEof() )
    ;
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

bool VideoSubWindow::save( QString filename )
{
  bool iRet = false;
  QApplication::setOverrideCursor( Qt::WaitCursor );

  CalypFrame* saveFrame = m_pcCurrFrameAsset.get();
  if( m_cSelectedArea.isValid() )
  {
    saveFrame = new CalypFrame( m_pcCurrFrameAsset.get(), m_cSelectedArea.x(), m_cSelectedArea.y(), m_cSelectedArea.width(),
                                m_cSelectedArea.height() );
  }
  if( !saveFrame )
  {
    return false;
  }
  iRet = CalypStream::saveFrame( filename.toStdString(), *saveFrame );
  QApplication::restoreOverrideCursor();
  return iRet;
}

bool VideoSubWindow::saveStream( QString filename )
{
  bool iRet = false;
  QApplication::setOverrideCursor( Qt::WaitCursor );

  // TODO: implement this
  QApplication::restoreOverrideCursor();
  return iRet;
}

void VideoSubWindow::setPlaying( bool isPlaying )
{
  m_bIsPlaying = isPlaying;
  for( auto module : m_associatedModules )
  {
    module->setPlaying( isPlaying );
  }
}

bool VideoSubWindow::isPlaying()
{
  if( getCategory() & SubWindowCategory::MODULE_SUBWINDOW )
  {
    for( auto window : m_pcCurrentDisplayModule->getSubWindowList() )
    {
      if( window->isPlaying() )
        return true;
    }
    return false;
  }
  return m_bIsPlaying;
}

bool VideoSubWindow::play()
{
  bool isPlaying = false;
  if( m_pCurrStream && m_pCurrStream->getFrameNum() > 1 )
  {
    isPlaying = true;
  }
  setPlaying( isPlaying );
  return m_bIsPlaying;
}

bool VideoSubWindow::playEvent()
{
  bool bEndOfSeq = false;
  if( m_pCurrStream && m_bIsPlaying )
  {
    bEndOfSeq = goToNextFrame( true );
  }
  return bEndOfSeq;
}

void VideoSubWindow::pause()
{
  setPlaying( false );
  refreshFrame();
}

void VideoSubWindow::seekAbsoluteEvent( unsigned int new_frame_num )
{
  if( m_pCurrStream )
  {
    if( m_pCurrStream->seekInput( new_frame_num ) )
      refreshFrame();

#ifdef CALYP_MANAGED_RESOURCES
    m_pcResourceManager->wakeResourceWorker( m_uiResourceId );
#endif
  }
}

void VideoSubWindow::seekRelativeEvent( bool bIsFoward )
{
  if( m_pCurrStream )
  {
    if( bIsFoward )
    {
      goToNextFrame( true );
    }
    else
    {
      m_pCurrStream->seekInputRelative( bIsFoward );
#ifdef CALYP_MANAGED_RESOURCES
      m_pcResourceManager->wakeResourceWorker( m_uiResourceId );
#endif
      refreshFrame();
    }
  }
}

void VideoSubWindow::stop()
{
#ifndef QT_NO_CONCURRENT
  m_cRefreshResult.waitForFinished();
  m_cReadResult.waitForFinished();
#endif
  setPlaying( false );
  seekAbsoluteEvent( 0 );
  return;
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

//void VideoSubWindow::setCurScrollValues()
//{
//  QScrollBar* scrollBar = m_pcScrollArea->horizontalScrollBar();
//  scrollBar->setValue( m_cCurrScroll.x() );
//  scrollBar = m_pcScrollArea->verticalScrollBar();
//  scrollBar->setValue( m_cCurrScroll.y() );
//}

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
  else
    imgViewSize = QSize( m_pCurrStream->getWidth(), m_pCurrStream->getHeight() );
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

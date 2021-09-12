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
 * \file     CalypStream.cpp
 * \brief    Input stream handling
 */

// Self
#include "CalypStream.h"

#include <cassert>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>

#include "CalypFrame.h"
#include "CalypStreamHandlerIf.h"
#include "LibMemory.h"
#include "StreamHandlerPortableMap.h"
#include "StreamHandlerRaw.h"
#include "config.h"
#ifdef USE_FFMPEG
#include "StreamHandlerLibav.h"
#endif
#ifdef USE_OPENCV
#include "StreamHandlerOpenCV.h"
#endif

#include <cstdio>

std::vector<CalypStreamFormat> CalypStream::supportedReadFormats()
{
  INI_REGIST_CALYP_SUPPORTED_FMT;
  APPEND_CALYP_SUPPORTED_FMT( StreamHandlerRaw, Read );
  APPEND_CALYP_SUPPORTED_FMT( StreamHandlerPortableMap, Read );
//#ifdef USE_OPENCV
//  APPEND_CALYP_SUPPORTED_FMT( StreamHandlerOpenCV, Read );
//#endif
#ifdef USE_FFMPEG
  APPEND_CALYP_SUPPORTED_FMT( StreamHandlerLibav, Read );
#endif
  END_REGIST_CALYP_SUPPORTED_FMT;
}

std::vector<CalypStreamFormat> CalypStream::supportedWriteFormats()
{
  INI_REGIST_CALYP_SUPPORTED_FMT;
  APPEND_CALYP_SUPPORTED_FMT( StreamHandlerRaw, Write );
  APPEND_CALYP_SUPPORTED_FMT( StreamHandlerPortableMap, Write );
#ifdef USE_FFMPEG
  APPEND_CALYP_SUPPORTED_FMT( StreamHandlerLibav, Write );
#endif
#ifdef USE_OPENCV
  APPEND_CALYP_SUPPORTED_FMT( StreamHandlerOpenCV, Write );
#endif
  END_REGIST_CALYP_SUPPORTED_FMT;
}

std::vector<CalypStandardResolution> CalypStream::stdResolutionSizes()
{
#define REGIST_CALYP_STANDARD_RESOLUTION( name, width, height ) \
  stdResElement.shortName = name;                               \
  stdResElement.uiWidth = width;                                \
  stdResElement.uiHeight = height;                              \
  stdResList.push_back( stdResElement );

  std::vector<CalypStandardResolution> stdResList;
  CalypStandardResolution stdResElement;
  REGIST_CALYP_STANDARD_RESOLUTION( "QCIF", 176, 144 );
  REGIST_CALYP_STANDARD_RESOLUTION( "CIF", 352, 288 );
  REGIST_CALYP_STANDARD_RESOLUTION( "VGA", 640, 480 );
  REGIST_CALYP_STANDARD_RESOLUTION( "WVGA", 832, 480 );
  REGIST_CALYP_STANDARD_RESOLUTION( "XVGA", 1024, 768 );
  REGIST_CALYP_STANDARD_RESOLUTION( "HD", 1280, 720 );
  REGIST_CALYP_STANDARD_RESOLUTION( "SXGA-", 1280, 900 );
  REGIST_CALYP_STANDARD_RESOLUTION( "SXGA", 1280, 1024 );
  REGIST_CALYP_STANDARD_RESOLUTION( "WSXGA", 1440, 900 );
  REGIST_CALYP_STANDARD_RESOLUTION( "FullHD", 1920, 1080 );
  REGIST_CALYP_STANDARD_RESOLUTION( "WQXGA", 2560, 1600 );
  REGIST_CALYP_STANDARD_RESOLUTION( "UltraHD", 3840, 2160 );
  REGIST_CALYP_STANDARD_RESOLUTION( "6K 2:1", 6144, 3072 );
  REGIST_CALYP_STANDARD_RESOLUTION( "8K", 7680, 4320 );
  REGIST_CALYP_STANDARD_RESOLUTION( "8K 2:1", 8192, 4096 );
  return stdResList;
}

class CalypStreamBufferPrivate
{
private:
  std::vector<CalypFrame> m_framePool;
  std::deque<CalypFrame> m_frameBuffer;
  std::int64_t m_iSize;
  std::int64_t m_iReadIndex;
  std::int64_t m_iWriteIndex;

public:
  CalypStreamBufferPrivate( std::size_t size, unsigned int width, unsigned int height, int pelFormat, unsigned int bitsPixel, bool hasNegative )
  {
    m_framePool.reserve( size );
    for( std::size_t i = 0; i < size; i++ )
    {
      m_framePool.push_back( CalypFrame{ width, height, pelFormat, bitsPixel, hasNegative } );
    }
    m_iSize = std::ssize( m_framePool );
    m_iReadIndex = -1;
    m_iWriteIndex = 0;
  }

  ~CalypStreamBufferPrivate() = default;

  void increase( std::size_t newSize )
  {
    m_framePool.reserve( newSize );
    for( std::size_t i = m_iSize; i < newSize; i++ )
    {
      m_framePool.push_back( CalypFrame{ m_framePool[0].getWidth(),
                                         m_framePool[0].getHeight(),
                                         m_framePool[0].getPelFormat(),
                                         m_framePool[0].getBitsPel(),
                                         m_framePool[0].getHasNegativeValues() } );
    }
    m_iSize = std::ssize( m_framePool );
  }

  void setIndex( std::int64_t i )
  {
    m_iWriteIndex = m_iReadIndex = i;
    m_iWriteIndex = nextIndex( m_iWriteIndex );
  }
  void reset()
  {
    m_iReadIndex = -1;
    m_iWriteIndex = 0;
  }
  void startRead()
  {
    assert( m_iWriteIndex >= 0 );
    m_iReadIndex = 0;
  }
  CalypFrame& frame( int i ) { return m_framePool.at( i ); }

  CalypFrame& currentRead()
  {
    assert( m_iReadIndex < m_iWriteIndex && ( m_iReadIndex % m_iSize ) < m_iSize );
    return m_framePool.at( m_iReadIndex % m_iSize );
  }

  const CalypFrame& ref() { return m_framePool.at( m_iWriteIndex % m_iSize ); }

  CalypFrame& readOneFrame()
  {
    assert( m_iReadIndex < m_iWriteIndex && ( m_iReadIndex % m_iSize ) < m_iSize );
    CalypFrame& frameToRead = m_framePool.at( m_iReadIndex % m_iSize );
    m_iReadIndex = nextIndex( m_iReadIndex );
    return frameToRead;
  }
  CalypFrame& writeOneFrame()
  {
    assert( m_iWriteIndex > m_iReadIndex && ( m_iReadIndex % m_iSize ) != ( m_iWriteIndex % m_iSize ) );
    CalypFrame& frameToWrite = m_framePool.at( m_iWriteIndex % m_iSize );
    m_iWriteIndex = nextIndex( m_iWriteIndex );
    return frameToWrite;
  }

  bool hasWritingSlot()
  {
    if( ( m_iReadIndex % m_iSize ) != ( m_iWriteIndex % m_iSize ) )
      return true;
    return false;
  }

  std::shared_ptr<CalypFrame> retrieveFrame()
  {
    assert( m_iReadIndex < m_iWriteIndex && ( m_iReadIndex % m_iSize ) < m_iSize );
    auto ptr = std::make_shared<CalypFrame>( std::move( m_framePool[m_iReadIndex % m_iSize] ) );
    m_framePool[m_iReadIndex % m_iSize] = CalypFrame{ ptr->getWidth(),
                                                      ptr->getHeight(),
                                                      ptr->getPelFormat(),
                                                      ptr->getBitsPel(),
                                                      ptr->getHasNegativeValues() };
    m_iReadIndex = nextIndex( m_iReadIndex );
    return ptr;
  }

private:
  //inline int nextIndex( std::int64_t curr ) { return curr + 1 >= m_iSize ? 0 : curr + 1; }
  inline int nextIndex( std::int64_t curr ) { return curr + 1; }
};

class CalypStream::CalypStreamPrivate
{
public:
  std::recursive_mutex mutex;

  bool isInit;
  bool isInput;

  CalypStreamHandlerIf* handler;
  CreateStreamHandlerFn pfctCreateHandler;

  std::vector<std::unique_ptr<CalypFrame>> framePool;
  std::deque<std::shared_ptr<CalypFrame>> frameFifo;
  std::size_t bufferIdx{ 0 };

  ClpString cFilename;
  long long int iCurrFrameNum;
  bool bLoadAll;

  CalypStreamPrivate()
  {
    handler = nullptr;
    pfctCreateHandler = nullptr;
    isInput = true;
    isInit = false;
    bLoadAll = false;
    iCurrFrameNum = -1;
    cFilename = "";
  }

  ~CalypStreamPrivate()
  {
    close();
  }

  void close()
  {
    if( handler )
    {
      handler->closeHandler();
      handler->Delete();
    }

    bLoadAll = false;
    isInit = false;
    framePool.clear();
    frameFifo.clear();
  }

  void increasePool( std::size_t newSize )
  {
    framePool.reserve( newSize );
    auto oldSize = framePool.size();
    for( std::size_t i = oldSize; i < newSize; i++ )
    {
      framePool.push_back( std::make_unique<CalypFrame>( framePool[0]->getWidth(),
                                                         framePool[0]->getHeight(),
                                                         framePool[0]->getPelFormat(),
                                                         framePool[0]->getBitsPel(),
                                                         framePool[0]->getHasNegativeValues() ) );
    }
  }

  bool readNextFrame( bool fillRgbBuffer = false )
  {
    if( !isInit || !isInput || handler->m_uiCurrFrameFileIdx >= handler->m_uiTotalNumberFrames )
      return false;

    if( bLoadAll )
      return true;

    const std::lock_guard<std::recursive_mutex> lock( mutex );
    mutex.lock();
    assert( bufferIdx > 0 );
    bufferIdx--;
    auto frame = framePool[bufferIdx].release();
    mutex.unlock();

    if( !handler->read( *frame ) )
    {
      throw CalypFailure( "CalypStream", "Cannot read frame from stream" );
      return false;
    }
    if( fillRgbBuffer )
    {
      frame->fillRGBBuffer();
    }

    auto deleter = [this]( CalypFrame* p ) {
      if( isInit )
      {
        const std::lock_guard<std::recursive_mutex> lock( mutex );
        framePool[bufferIdx].reset( p );
        bufferIdx++;
      }
      else
      {
        delete p;
      }
    };
    frameFifo.push_back( std::shared_ptr<CalypFrame>{ frame, deleter } );
    return true;
  }
};

std::vector<ClpString> CalypStreamFormat::getExts()
{
  std::vector<ClpString> arrayExt;
  ClpString::size_type prev_pos = 0, pos = 0;
  while( ( pos = formatExt.find( ',', pos ) ) != ClpString::npos )
  {
    ClpString substring( formatExt.substr( prev_pos, pos - prev_pos ) );
    arrayExt.push_back( substring );
    prev_pos = ++pos;
  }
  arrayExt.push_back( formatExt.substr( prev_pos, pos - prev_pos ) );  // Last word
  return arrayExt;
}

CreateStreamHandlerFn CalypStream::findStreamHandler( ClpString strFilename, bool bRead )
{
  ClpString currExt = strFilename.substr( strFilename.find_last_of( "." ) + 1 );
  currExt = clpLowercase( currExt );

  std::vector<CalypStreamFormat> supportedFmts;
  if( bRead )
  {
    supportedFmts = CalypStream::supportedReadFormats();
  }
  else
  {
    supportedFmts = CalypStream::supportedWriteFormats();
  }
  for( unsigned int i = 0; i < supportedFmts.size(); i++ )
  {
    std::vector<ClpString> arrayExt = supportedFmts[i].getExts();
    for( std::vector<ClpString>::iterator e = arrayExt.begin(); e != arrayExt.end(); ++e )
    {
      if( currExt != "" && currExt == *e )
      {
        return supportedFmts[i].formatFct;
      }
      else if( strFilename.find( *e ) != ClpString::npos )
      {
        return supportedFmts[i].formatFct;
      }
    }
  }

#ifdef USE_FFMPEG
  return &StreamHandlerLibav::Create;
#else
  return &StreamHandlerRaw::Create;
#endif
}

CalypStream::CalypStream()
    : d{ std::make_unique<CalypStreamPrivate>() }
{
}

CalypStream::~CalypStream() = default;

ClpString CalypStream::getFormatName() const
{
  return !d->handler ? "" : d->handler->getFormatName();
}
ClpString CalypStream::getCodecName() const
{
  return !d->handler ? "" : d->handler->getCodecName();
}

bool CalypStream::open( ClpString filename, ClpString resolution, ClpString input_format_name, unsigned int bitsPel, int endianness, bool hasNegative,
                        unsigned int frame_rate, bool bInput )
{
  unsigned int width = 0;
  unsigned int height = 0;
  int input_format = -1;

  if( resolution.size() > 0 )
  {
    sscanf( resolution.c_str(), "%ux%u", &width, &height );
    if( width <= 0 || height <= 0 )
    {
      return false;
    }
  }
  for( unsigned int i = 0; i < CalypFrame::supportedPixelFormatListNames().size(); i++ )
  {
    if( clpLowercase( CalypFrame::supportedPixelFormatListNames()[i] ) == clpLowercase( input_format_name ) )
    {
      input_format = i;
      break;
    }
  }
  return open( filename, width, height, input_format, bitsPel, endianness, hasNegative, frame_rate, bInput );
}

bool CalypStream::open( ClpString filename, unsigned int width, unsigned int height, int input_format, unsigned int bitsPel, int endianness,
                        unsigned int frame_rate, bool bInput )
{
  return open( filename, width, height, input_format, bitsPel, endianness, false, frame_rate, bInput );
}

bool CalypStream::open( ClpString filename, unsigned int width, unsigned int height, int input_format, unsigned int bitsPel, int endianness, bool hasNegative,
                        unsigned int frame_rate, bool bInput )
{
  if( d->isInit )
  {
    d->close();
  }
  d->isInit = false;
  d->isInput = bInput;

  d->pfctCreateHandler = CalypStream::findStreamHandler( filename, d->isInput );
  if( !d->pfctCreateHandler )
  {
    throw CalypFailure( "CalypStream", "Invalid handler" );
  }

  d->handler = d->pfctCreateHandler();

  if( !d->handler )
  {
    throw CalypFailure( "CalypStream", "Cannot create handler" );
  }

  d->cFilename = filename;

  d->handler->m_cFilename = filename;
  d->handler->m_uiWidth = width;
  d->handler->m_uiHeight = height;
  d->handler->m_iPixelFormat = input_format;
  d->handler->m_uiBitsPerPixel = bitsPel;
  d->handler->m_iEndianness = endianness;
  d->handler->m_dFrameRate = frame_rate;

  if( !d->handler->openHandler( d->cFilename, d->isInput ) )
  {
    d->close();
    return d->isInit;
  }

  if( d->handler->m_uiWidth <= 0 || d->handler->m_uiHeight <= 0 || d->handler->m_iPixelFormat < 0 )
  {
    d->close();
    //throw CalypFailure( "CalypStream", "Incorrect configuration: width, height or pixel format" );
    return d->isInit;
  }

  // Keep past, current and future frames
  std::size_t bufferSize = d->isInput ? 4 : 1;
  try
  {
    d->framePool.reserve( bufferSize );
    for( std::size_t i = 0; i < bufferSize; i++ )
    {
      d->framePool.push_back(
          std::make_unique<CalypFrame>(
              d->handler->m_uiWidth,
              d->handler->m_uiHeight,
              d->handler->m_iPixelFormat,
              d->handler->m_uiBitsPerPixel,
              hasNegative ) );
    }
  }
  catch( CalypFailure& e )
  {
    d->close();
    throw CalypFailure( "CalypStream", "Cannot allocated frame buffer" );
    return d->isInit;
  }

  d->frameFifo.clear();
  d->bufferIdx = d->framePool.size();

  const CalypFrame& refFrame = *d->framePool[0];

  d->handler->m_uiNBytesPerFrame = refFrame.getBytesPerFrame();

  // Some handlers need to know how long is a frame to get frame number
  d->handler->calculateFrameNumber();

  if( d->isInput && d->handler->m_uiTotalNumberFrames == 0 )
  {
    d->close();
    throw CalypFailure( "CalypStream", "Incorrect configuration: less than one frame" );
    return d->isInit;
  }

  if( !d->handler->configureBuffer( refFrame ) )
  {
    d->close();
    throw CalypFailure( "CalypStream", "Cannot allocated buffers" );
    return d->isInit;
  }

  d->iCurrFrameNum = -1;
  d->isInit = true;

  seekInput( 0 );

  d->isInit = true;
  return d->isInit;
}

bool CalypStream::reload()
{
  d->frameFifo.clear();
  d->handler->closeHandler();
  if( !d->handler->openHandler( d->cFilename, d->isInput ) )
  {
    throw CalypFailure( "CalypStream", "Cannot open stream " + d->cFilename + " with the " +
                                           ClpString( d->handler->m_pchHandlerName ) + " handler" );
  }
  const CalypFrame& refFrame = *d->framePool[0];
  d->handler->m_uiNBytesPerFrame = refFrame.getBytesPerFrame();
  d->handler->calculateFrameNumber();
  d->handler->configureBuffer( *d->framePool[0] );

  if( d->handler->m_uiWidth <= 0 || d->handler->m_uiHeight <= 0 || d->handler->m_iPixelFormat < 0 ||
      d->handler->m_uiBitsPerPixel == 0 || d->handler->m_uiTotalNumberFrames == 0 )
  {
    return false;
  }
  if( (unsigned int)( d->iCurrFrameNum ) >= d->handler->m_uiTotalNumberFrames )
  {
    d->iCurrFrameNum = 0;
  }
  int currFrameNum = d->iCurrFrameNum;
  d->iCurrFrameNum = -1;
  seekInput( currFrameNum );
  return true;
}

ClpString CalypStream::getFileName() const
{
  return d->cFilename;
}

bool CalypStream::isNative() const
{
  return d->handler->m_bNative;
}

ClpULong CalypStream::getFrameNum() const
{
  return d->handler->m_uiTotalNumberFrames;
}
unsigned int CalypStream::getWidth() const
{
  return d->handler->m_uiWidth;
}
unsigned int CalypStream::getHeight() const
{
  return d->handler->m_uiHeight;
}
unsigned int CalypStream::getBitsPerPixel() const
{
  return d->handler->m_uiBitsPerPixel;
}
int CalypStream::getEndianess() const
{
  return d->handler->m_iEndianness;
}
double CalypStream::getFrameRate() const
{
  return d->handler->m_dFrameRate;
}

long CalypStream::getCurrFrameNum() const
{
  return d->iCurrFrameNum;
}

void CalypStream::getFormat( unsigned int& rWidth, unsigned int& rHeight, int& rInputFormat, unsigned int& rBitsPerPel, int& rEndianness,
                             unsigned int& rFrameRate ) const
{
  if( d->isInit )
  {
    rWidth = d->handler->m_uiWidth;
    rHeight = d->handler->m_uiHeight;
    rInputFormat = d->handler->m_iPixelFormat;
    rBitsPerPel = d->handler->m_uiBitsPerPixel;
    rEndianness = d->handler->m_iEndianness;
    rFrameRate = d->handler->m_dFrameRate;
  }
  else
  {
    rWidth = 0;
    rHeight = 0;
    rInputFormat = CLP_YUV420P;
    rBitsPerPel = 8;
    rEndianness = 0;
    rFrameRate = 30;
  }
}

auto CalypStream::hasNextFrame() -> bool
{
  return d->frameFifo.size() > 1;
}

auto CalypStream::hasWritingSlot() -> bool
{
  return !d->bLoadAll && d->bufferIdx > 0;
}

/**
 * @brief Read operations
 */

void CalypStream::loadAll()
{
  if( d->bLoadAll || !d->isInput )
    return;

  try
  {
    d->increasePool( d->handler->m_uiTotalNumberFrames );
  }
  catch( CalypFailure& e )
  {
    d->close();
    throw CalypFailure( "CalypStream", "Cannot allocated frame buffer for the whole stream" );
  }

  seekInput( 0 );
  for( unsigned int i = 2; i < d->handler->m_uiTotalNumberFrames; i++ )
  {
    d->readNextFrame( false );
  }
  d->bLoadAll = true;
  d->iCurrFrameNum = 0;
}

std::unique_ptr<CalypFrame> CalypStream::getCurrFrame( std::unique_ptr<CalypFrame> buffer )
{
  if( buffer == nullptr )
    buffer = std::make_unique<CalypFrame>( *d->frameFifo.front() );
  else
    buffer->copyFrom( *d->frameFifo.front() );
  return buffer;
}

auto CalypStream::getCurrFrameAsset() -> std::shared_ptr<CalypFrame>
{
  if( d->bLoadAll )
    return d->frameFifo[d->iCurrFrameNum];
  return d->frameFifo.front();
}

auto CalypStream::getCurrFrame() -> CalypFrame*
{
  if( d->bLoadAll )
    return d->frameFifo[d->iCurrFrameNum].get();
  return d->frameFifo.front().get();
}

bool CalypStream::isEof()
{
  if( d->iCurrFrameNum + 1 >= (long)( d->handler->m_uiTotalNumberFrames ) )
  {
    return true;
  }
  return false;
}

bool CalypStream::setNextFrame()
{
  if( isEof() )
  {
    return true;
  }
  d->iCurrFrameNum++;
  if( !d->bLoadAll )
  {
    assert( d->frameFifo.size() > 1 );
    d->frameFifo.pop_front();
  }
  return false;
}

void CalypStream::readNextFrame()
{
  d->readNextFrame();
}

void CalypStream::readNextFrameFillRGBBuffer()
{
  d->readNextFrame( true );
}

/**
 * @brief Write operations
 */
void CalypStream::writeFrame( const CalypFrame& pcFrame )
{
  if( !d->handler->write( pcFrame ) )
  {
    throw CalypFailure( "CalypStream", "Cannot write frame into the stream" );
  }
  return;
}

bool CalypStream::saveFrame( const ClpString& filename )
{
  return saveFrame( filename, *getCurrFrame() );
}

bool CalypStream::saveFrame( const ClpString& filename, const CalypFrame& saveFrame )
{
  CalypStream auxSaveStream;
  if( !auxSaveStream.open( filename, saveFrame.getWidth(), saveFrame.getHeight(), saveFrame.getPelFormat(),
                           saveFrame.getBitsPel(), CLP_LITTLE_ENDIAN, 1, false ) )
  {
    return false;
  }
  auxSaveStream.writeFrame( saveFrame );
  return true;
}

/**
 * @brief Seek operations
 */
bool CalypStream::seekInputRelative( bool bIsFoward )
{
  if( !d->isInit || !d->isInput )
    return false;

  bool bRet = false;
  if( bIsFoward )
  {
    bRet = !setNextFrame();
    d->readNextFrame();
  }
  else
  {
    unsigned long long int newFrameNum = d->iCurrFrameNum - 1;
    bRet = seekInput( newFrameNum );
  }
  return bRet;
}

bool CalypStream::seekInput( unsigned long new_frame_num )
{
  const std::lock_guard<std::recursive_mutex> lock( d->mutex );

  if( !d->isInit || new_frame_num >= d->handler->m_uiTotalNumberFrames || long( new_frame_num ) == d->iCurrFrameNum )
    return false;

  d->frameFifo.clear();

  d->iCurrFrameNum = new_frame_num;

  if( d->bLoadAll )
    return true;

  if( !d->handler->seek( d->iCurrFrameNum ) )
  {
    throw CalypFailure( "CalypStream", "Cannot seek file into desired position" );
  }

  d->readNextFrame();
  if( d->handler->m_uiTotalNumberFrames > 1 )
    d->readNextFrame();
  return true;
}

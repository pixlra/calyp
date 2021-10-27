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
#include <string>

#include "CalypFrame.h"
#include "CalypStreamHandlerIf.h"
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

constexpr auto kDefaultBitsPerPixel = 8;
constexpr auto kDefaultFrameRate = 30;

auto find_stream_handler( const std::string& strFilename, bool bRead ) -> CalypStreamFormat::CreateStreamHandlerFn;

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

struct CalypStreamFrameBuffer : std::enable_shared_from_this<CalypStreamFrameBuffer>
{
  std::mutex buffer_mutex;
  std::vector<std::unique_ptr<CalypFrame>> framePool;
  std::size_t bufferIdx{ 0 };

  CalypStreamFrameBuffer( std::size_t size, unsigned int width, unsigned int height, ClpPixelFormats pelFormat, unsigned int bitsPixel, bool hasNegative )
  {
    framePool.reserve( size );
    for( std::size_t i = 0; i < size; i++ )
    {
      framePool.push_back( std::make_unique<CalypFrame>( width, height, pelFormat, bitsPixel, hasNegative ) );
    }
    bufferIdx = framePool.size();
  }

  CalypStreamFrameBuffer( const CalypStreamFrameBuffer& ) = delete;
  CalypStreamFrameBuffer( CalypStreamFrameBuffer&& ) = delete;
  CalypStreamFrameBuffer& operator=( const CalypStreamFrameBuffer& ) = delete;
  CalypStreamFrameBuffer& operator=( CalypStreamFrameBuffer&& ) = delete;

  ~CalypStreamFrameBuffer() = default;

  void increase( std::size_t newSize )
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
    bufferIdx += newSize - oldSize;
  }

  CalypFrame* ref() { return framePool[0].get(); }

  std::shared_ptr<CalypFrame> getFrame()
  {
    const std::lock_guard<std::mutex> lock( buffer_mutex );
    assert( bufferIdx > 0 );
    bufferIdx--;
    auto frame = framePool[bufferIdx].release();
    auto deleter = [this, lifetime = shared_from_this()]( CalypFrame* p ) {
      const std::lock_guard<std::mutex> lock( buffer_mutex );
      framePool[bufferIdx].reset( p );
      bufferIdx++;
    };
    return std::shared_ptr<CalypFrame>{ frame, deleter };
  }
};

class CalypStream::CalypStreamPrivate
{
public:
  std::recursive_mutex stream_mutex;
  CalypStream::Type streamType;
  bool isInit;

  std::unique_ptr<CalypStreamHandlerIf> handler;

  std::shared_ptr<CalypStreamFrameBuffer> frameBuffer;
  std::deque<std::shared_ptr<CalypFrame>> frameFifo;

  // std::vector<std::unique_ptr<CalypFrame>> framePool;
  // std::deque<std::shared_ptr<CalypFrame>> frameFifo;
  // std::size_t bufferIdx{ 0 };

  std::string cFilename;
  long long int iCurrFrameNum;
  bool bLoadAll;

  CalypStreamPrivate( const CalypStreamPrivate& ) = delete;
  CalypStreamPrivate( CalypStreamPrivate&& ) = delete;
  CalypStreamPrivate& operator=( const CalypStreamPrivate& ) = delete;
  CalypStreamPrivate& operator=( CalypStreamPrivate&& ) = delete;
  CalypStreamPrivate()
  {
    handler = nullptr;
    streamType = CalypStream::Type::Input;
    isInit = false;
    bLoadAll = false;
    iCurrFrameNum = -1;
  }

  ~CalypStreamPrivate()
  {
    close();
  }

  bool open( std::string filename, unsigned int width, unsigned int height, ClpPixelFormats input_format, unsigned int bitsPel, int endianness, bool hasNegative,
             unsigned int frame_rate, bool forceRaw, CalypStream::Type type )
  {
    if( isInit )
    {
      close();
    }
    isInit = false;
    streamType = type;
    bool isInput = streamType == CalypStream::Type::Input;

    if( forceRaw )
    {
      handler = StreamHandlerRaw::Create();
    }
    else
    {
      auto createHandlerFct = find_stream_handler( filename, isInput );
      if( !createHandlerFct )
      {
        throw CalypFailure( "CalypStream", "Invalid handler" );
      }

      handler = createHandlerFct();
    }

    if( !handler )
    {
      throw CalypFailure( "CalypStream", "Cannot create handler" );
    }

    cFilename = std::move( filename );

    handler->m_uiWidth = width;
    handler->m_uiHeight = height;
    handler->m_iPixelFormat = input_format;
    handler->m_uiBitsPerPixel = bitsPel;
    handler->m_iEndianness = bitsPel > 8 ? endianness : CLP_BIG_ENDIAN;
    handler->m_dFrameRate = frame_rate;

    if( !handler->openHandler( cFilename, isInput ) )
    {
      close();
      throw CalypFailure( "CalypStream", "Cannot create the stream handler" );
      return isInit;
    }

    if( handler->m_uiWidth <= 0 || handler->m_uiHeight <= 0 || handler->m_iPixelFormat == ClpPixelFormats::CLP_INVALID_FMT )
    {
      close();
      throw CalypFailure( "CalypStream", "Incorrect configuration: width, height or pixel format" );
      return isInit;
    }

    frameFifo.clear();

    // Keep past, current and future frames
    std::size_t bufferSize = isInput ? 6 : 1;
    try
    {
      frameBuffer = std::make_shared<CalypStreamFrameBuffer>( bufferSize,
                                                              handler->m_uiWidth,
                                                              handler->m_uiHeight,
                                                              handler->m_iPixelFormat,
                                                              handler->m_uiBitsPerPixel,
                                                              hasNegative );
    }
    catch( CalypFailure& e )
    {
      close();
      throw CalypFailure( "CalypStream", "Cannot allocated frame buffer" );
      return isInit;
    }

    const CalypFrame* refFrame = frameBuffer->ref();

    handler->m_uiNBytesPerFrame = refFrame->getBytesPerFrame();

    // Some handlers need to know how long is a frame to get frame number
    handler->calculateFrameNumber();

    if( isInput && handler->m_uiTotalNumberFrames == 0 )
    {
      close();
      throw CalypFailure( "CalypStream", "Incorrect configuration: less than one frame" );
      return isInit;
    }

    if( !handler->configureBuffer( *refFrame ) )
    {
      close();
      throw CalypFailure( "CalypStream", "Cannot allocated buffers" );
      return isInit;
    }

    iCurrFrameNum = -1;
    isInit = true;

    seekInput( 0 );

    isInit = true;
    return isInit;
  }

  void close()
  {
    if( handler )
    {
      handler->closeHandler();
    }

    bLoadAll = false;
    isInit = false;
  }

  bool seekInput( unsigned long new_frame_num )
  {
    const std::lock_guard<std::recursive_mutex> lock( stream_mutex );

    if( !isInit || new_frame_num >= handler->m_uiTotalNumberFrames || long( new_frame_num ) == iCurrFrameNum )
      return false;

    iCurrFrameNum = new_frame_num;

    if( bLoadAll )
      return true;

    frameFifo.clear();
    if( !handler->seek( iCurrFrameNum ) )
    {
      throw CalypFailure( "CalypStream", "Cannot seek file into desired position" );
    }

    readNextFrame();
    if( handler->m_uiTotalNumberFrames > 1 )
      readNextFrame();
    return true;
  }

  bool readNextFrame( bool fillRgbBuffer = false )
  {
    const std::lock_guard<std::recursive_mutex> lock( stream_mutex );

    if( !isInit || streamType != CalypStream::Type::Input || handler->m_uiCurrFrameFileIdx >= handler->m_uiTotalNumberFrames )
      return false;

    if( bLoadAll )
      return true;

    auto frame = frameBuffer->getFrame();

    if( !handler->read( *frame ) )
    {
      throw CalypFailure( "CalypStream", "Cannot read frame from stream" );
      return false;
    }
    frameFifo.push_back( frame );

    if( fillRgbBuffer )
    {
      frame->fillRGBBuffer();
    }
    return true;
  }
};

std::vector<std::string> CalypStreamFormat::getExts()
{
  std::vector<std::string> arrayExt;
  std::string::size_type prev_pos = 0, pos = 0;
  while( ( pos = formatExt.find( ',', pos ) ) != std::string::npos )
  {
    std::string substring( formatExt.substr( prev_pos, pos - prev_pos ) );
    arrayExt.push_back( substring );
    prev_pos = ++pos;
  }
  arrayExt.push_back( formatExt.substr( prev_pos, pos - prev_pos ) );  // Last word
  return arrayExt;
}

CalypStream::CalypStream()
    : d{ std::make_unique<CalypStreamPrivate>() }
{
}

CalypStream::~CalypStream() = default;

std::string CalypStream::getFormatName() const
{
  return !d->handler ? "" : d->handler->getFormatName();
}
std::string CalypStream::getCodecName() const
{
  return !d->handler ? "" : d->handler->getCodecName();
}

bool CalypStream::open( std::string filename, std::string resolution, std::string input_format_name, unsigned int bitsPel, int endianness, bool hasNegative,
                        unsigned int frame_rate, CalypStream::Type type )
{
  unsigned int width = 0;
  unsigned int height = 0;

  if( resolution.size() > 0 )
  {
    sscanf( resolution.c_str(), "%ux%u", &width, &height );
    if( width <= 0 || height <= 0 )
    {
      return false;
    }
  }

  const auto input_format = CalypFrame::findPixelFormat( input_format_name );
  if( !input_format.has_value() )
  {
    return false;
  }
  return d->open( std::move( filename ), width, height, *input_format, bitsPel, endianness, hasNegative, frame_rate, false, type );
}

bool CalypStream::open( std::string filename, unsigned int width, unsigned int height, ClpPixelFormats input_format, unsigned int bitsPel, int endianness,
                        unsigned int frame_rate, CalypStream::Type type )
{
  return d->open( std::move( filename ), width, height, input_format, bitsPel, endianness, false, frame_rate, false, type );
}

bool CalypStream::open( std::string filename, unsigned int width, unsigned int height, ClpPixelFormats input_format, unsigned int bitsPel, int endianness, bool hasNegative,
                        unsigned int frame_rate, CalypStream::Type type )
{
  return d->open( std::move( filename ), width, height, input_format, bitsPel, endianness, hasNegative, frame_rate, false, type );
}

bool CalypStream::open( std::string filename, unsigned int width, unsigned int height, ClpPixelFormats input_format, unsigned int bitsPel, int endianness,
                        unsigned int frame_rate, bool forceRaw, CalypStream::Type type )
{
  return d->open( std::move( filename ), width, height, input_format, bitsPel, endianness, false, frame_rate, forceRaw, type );
}

bool CalypStream::supportsFormatConfiguration()
{
  if( d->handler == nullptr )
    return false;
  return d->handler->m_bSupportsFormat;
}

bool CalypStream::reload()
{
  d->frameFifo.clear();
  d->handler->closeHandler();
  if( !d->handler->openHandler( d->cFilename, d->streamType == CalypStream::Type::Input ) )
  {
    throw CalypFailure( "CalypStream", "Cannot open stream " + d->cFilename + " with the " +
                                           std::string( d->handler->m_pchHandlerName ) + " handler" );
  }
  const CalypFrame* refFrame = d->frameBuffer->ref();
  d->handler->m_uiNBytesPerFrame = refFrame->getBytesPerFrame();
  d->handler->calculateFrameNumber();
  d->handler->configureBuffer( *refFrame );

  if( d->handler->m_uiWidth <= 0 || d->handler->m_uiHeight <= 0 || d->handler->m_iPixelFormat == ClpPixelFormats::CLP_INVALID_FMT ||
      d->handler->m_uiBitsPerPixel == 0 || d->handler->m_uiTotalNumberFrames == 0 )
  {
    return false;
  }
  if( (unsigned int)( d->iCurrFrameNum ) >= d->handler->m_uiTotalNumberFrames )
  {
    d->iCurrFrameNum = 0;
  }
  auto currFrameNum = d->iCurrFrameNum;
  d->iCurrFrameNum = -1;
  seekInput( currFrameNum );
  return true;
}

std::string CalypStream::getFileName() const
{
  return d->cFilename;
}

bool CalypStream::isNative() const
{
  return d->handler->m_bNative;
}

std::uint64_t CalypStream::getFrameNum() const
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

void CalypStream::getFormat( unsigned int& rWidth, unsigned int& rHeight, ClpPixelFormats& rInputFormat, unsigned int& rBitsPerPel, int& rEndianness,
                             unsigned int& rFrameRate ) const
{
  if( d->isInit )
  {
    rWidth = d->handler->m_uiWidth;
    rHeight = d->handler->m_uiHeight;
    rInputFormat = d->handler->m_iPixelFormat;
    rBitsPerPel = d->handler->m_uiBitsPerPixel;
    rEndianness = d->handler->m_iEndianness;
    rFrameRate = static_cast<unsigned>( std::max( 0.0, d->handler->m_dFrameRate ) );
  }
  else
  {
    rWidth = 0;
    rHeight = 0;
    rInputFormat = ClpPixelFormats::CLP_YUV420P;
    rBitsPerPel = kDefaultBitsPerPixel;
    rEndianness = 0;
    rFrameRate = kDefaultFrameRate;
  }
}

auto CalypStream::hasNextFrame() -> bool
{
  return d->frameFifo.size() > 1;
}

auto CalypStream::hasWritingSlot() -> bool
{
  return !d->bLoadAll && d->frameBuffer->bufferIdx > 0;
}

/**
 * @brief Read operations
 */

void CalypStream::loadAll()
{
  const std::lock_guard<std::recursive_mutex> lock( d->stream_mutex );
  if( d->bLoadAll || d->streamType != CalypStream::Type::Input )
    return;

  try
  {
    d->frameBuffer->increase( d->handler->m_uiTotalNumberFrames );
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

bool CalypStream::saveFrame( const std::string& filename )
{
  return saveFrame( filename, *getCurrFrame() );
}

bool CalypStream::saveFrame( const std::string& filename, const CalypFrame& saveFrame )
{
  CalypStream auxSaveStream;
  if( !auxSaveStream.open( filename, saveFrame.getWidth(), saveFrame.getHeight(), saveFrame.getPelFormat(),
                           saveFrame.getBitsPel(), CLP_LITTLE_ENDIAN, 1, Type::Output ) )
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
  if( !d->isInit || d->streamType != Type::Input )
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
  return d->seekInput( new_frame_num );
}

auto find_stream_handler( const std::string& strFilename, bool bRead ) -> CalypStreamFormat::CreateStreamHandlerFn
{
  std::string currExt = strFilename.substr( strFilename.find_last_of( "." ) + 1 );
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
    std::vector<std::string> arrayExt = supportedFmts[i].getExts();
    for( std::vector<std::string>::iterator e = arrayExt.begin(); e != arrayExt.end(); ++e )
    {
      if( currExt != "" && currExt == *e )
      {
        return supportedFmts[i].formatFct;
      }
      else if( strFilename.find( *e ) != std::string::npos )
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
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
 * \file     CalypFrame.cpp
 * \brief    Video Frame handling
 */

#include "CalypFrame.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#include "CalypDefs.h"
#include "config.h"

#ifdef USE_OPENCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif

#include "PixelFormats.h"
#include "config.h"

#define DATA_ALIGN 1  ///< use 32-bit aligned malloc/free
#if DATA_ALIGN && _WIN32 && ( _MSC_VER > 1300 )
#define xMalloc( len ) _aligned_malloc( len, 32 )
#define xFreeMem( ptr ) _aligned_free( ptr )
#else
#define xMalloc( len ) malloc( len )
#define xFreeMem( ptr ) free( ptr )
#endif

constexpr auto kMinBitsPerPixel = 8;
constexpr auto kMaxBitsPerPixel = 16;

std::vector<std::string> CalypFrame::supportedColorSpacesListNames()
{
  return std::vector<std::string>{
      "YUV",
      "RGB",
      "GRAY",
      "ARGB",
  };
}

constexpr auto CalypFrame::numberOfFormats() -> std::size_t
{
  return kNumberOfPixelFormats;
}

auto CalypFrame::findPixelFormat( const std::string& name ) -> std::optional<ClpPixelFormats>
{
  for( const auto& [key, fmt] : g_CalypPixFmtDescriptorsMap )
  {
    if( fmt.name.size() == name.size() &&
        std::equal( fmt.name.begin(), fmt.name.end(), name.begin(),
                    []( auto a, auto b ) { return std::tolower( a ) == std::tolower( b ); } ) )
      return key;
  }
  return {};
}

auto CalypFrame::findPixelFormat( const std::string_view name ) -> std::optional<ClpPixelFormats>
{
  for( const auto& [key, fmt] : g_CalypPixFmtDescriptorsMap )
  {
    if( fmt.name.size() == name.size() &&
        std::equal( fmt.name.begin(), fmt.name.end(), name.begin(),
                    []( auto a, auto b ) { return std::tolower( a ) == std::tolower( b ); } ) )
      return key;
  }
  return {};
}

auto CalypFrame::pelformatColorSpace( ClpPixelFormats idx ) -> int
{
  return g_CalypPixFmtDescriptorsMap.at( idx ).colorSpace;
}

auto CalypFrame::supportedPixelFormatListNames() -> std::map<ClpPixelFormats, std::string_view>
{
  std::map<ClpPixelFormats, std::string_view> formatsList;
  for( const auto& [key, fmt] : g_CalypPixFmtDescriptorsMap )
  {
    formatsList[key] = fmt.name;
  }
  return formatsList;
}

auto CalypFrame::supportedPixelFormatListNames( int colorSpace ) -> std::map<ClpPixelFormats, std::string_view>
{
  std::map<ClpPixelFormats, std::string_view> formatsList;
  for( const auto& [key, fmt] : g_CalypPixFmtDescriptorsMap )
  {
    if( fmt.colorSpace == colorSpace )
    {
      formatsList[key] = fmt.name;
    }
  }
  return formatsList;
}

auto CalypFrame::pixelFormatName( ClpPixelFormats idx ) -> const std::string_view
{
  return g_CalypPixFmtDescriptorsMap.at( idx ).name;
}

class CalypFrame::CalypFramePrivate
{
public:
  bool m_bInit{ false };

  //! Struct with the pixel format description.
  const CalypPixelFormatDescriptor* m_pcPelFormat{ nullptr };

  unsigned int m_uiWidth{ 0 };                                 //!< Width of the frame
  unsigned int m_uiHeight{ 0 };                                //!< Height of the frame
  ClpPixelFormats m_iPixelFormat{ ClpPixelFormats::Invalid };  //!< Pixel format number (it follows the list of supported pixel formats)
  unsigned int m_uiBitsPel{ 0 };                               //!< Bits per pixel/channel
  unsigned int m_uiHalfPelValue{ 0 };                          //!< Bits per pixel/channel
  bool m_bHasNegativeValues{ false };                          //!< Half of the scale correspond to negative values

  ClpPel*** m_pppcInputPel{ nullptr };

  bool m_bHasRGBPel{ false };            //!< Flag indicating that the ARGB buffer was computed
  std::vector<std::uint8_t> m_pcARGB32;  //!< Buffer with the ARGB pixels used in Qt libs

  /** Histogram control variables **/
  bool m_bHasHistogram{ false };
  bool m_bHistogramRunning{ false };
  /** The histogram data.*/
  std::vector<unsigned int> m_puiHistogram;
  /** If the image is RGB and calcLuma is true, we have 1 more channel */
  unsigned int m_uiHistoChannels{ 0 };
  /** Numbers of histogram segments depending of image bytes depth*/
  unsigned int m_uiHistoSegments{ 0 };

  CalypFramePrivate() = default;
  CalypFramePrivate( const CalypFramePrivate& ) = delete;
  CalypFramePrivate( CalypFramePrivate&& ) = delete;
  CalypFramePrivate& operator=( const CalypFramePrivate& ) = delete;
  CalypFramePrivate& operator=( CalypFramePrivate&& ) = delete;

  void init( unsigned int width, unsigned int height, ClpPixelFormats pelFormat, unsigned bitsPixel )
  {
    init( width, height, pelFormat, bitsPixel, false );
  }

  void init( unsigned int width, unsigned int height, ClpPixelFormats pelFormat, unsigned bitsPixel, bool has_negative_values )
  {
    m_uiWidth = width;
    m_uiHeight = height;
    m_iPixelFormat = pelFormat;
    m_uiBitsPel = bitsPixel < kMinBitsPerPixel ? kMinBitsPerPixel : bitsPixel;
    m_uiHalfPelValue = 1 << ( m_uiBitsPel - 1 );
    m_bHasNegativeValues = has_negative_values;

    if( m_uiWidth == 0 || m_uiHeight == 0 || m_iPixelFormat == ClpPixelFormats::Invalid || bitsPixel > kMaxBitsPerPixel )
    {
      throw CalypFailure( "CalypFrame", "Cannot create a CalypFrame of this type" );
    }

    m_pcPelFormat = &( g_CalypPixFmtDescriptorsMap.at( pelFormat ) );
    int iNumberChannels = m_pcPelFormat->numberChannels;

    std::size_t mem_size = 0;
    std::size_t num_of_ptrs = 0;
    for( int ch = 0; ch < iNumberChannels; ch++ )
    {
      int ratioW = ch > 0 ? m_pcPelFormat->log2ChromaWidth : 0;
      int ratioH = ch > 0 ? m_pcPelFormat->log2ChromaHeight : 0;
      // Add pointers mem
      num_of_ptrs += CHROMASHIFT( m_uiHeight, ratioH );
      // Add pixel mem
      mem_size += CHROMASHIFT( m_uiHeight, ratioH ) * CHROMASHIFT( m_uiWidth, ratioW ) * sizeof( ClpPel );
    }
    mem_size += num_of_ptrs * sizeof( ClpPel* ) + sizeof( ClpPel** ) * iNumberChannels;

    m_pppcInputPel = (ClpPel***)xMalloc( mem_size );

    ClpPel** pelPtrMem = (ClpPel**)( m_pppcInputPel + iNumberChannels );
    ClpPel* pelMem = (ClpPel*)( pelPtrMem + num_of_ptrs );
    for( int ch = 0; ch < iNumberChannels; ch++ )
    {
      int ratioW = ch > 0 ? m_pcPelFormat->log2ChromaWidth : 0;
      int ratioH = ch > 0 ? m_pcPelFormat->log2ChromaHeight : 0;
      m_pppcInputPel[ch] = pelPtrMem;
      for( unsigned int h = 0; h < CHROMASHIFT( m_uiHeight, ratioH ); h++ )
      {
        *pelPtrMem = pelMem;
        pelPtrMem++;
        pelMem += CHROMASHIFT( m_uiWidth, ratioW );
      }
    }

    /* Alloc ARGB memory */
    m_pcARGB32.resize( m_uiHeight * m_uiWidth * 4 );

    m_bHasHistogram = false;
    m_bHistogramRunning = false;

    m_uiHistoSegments = 1 << m_uiBitsPel;

    if( m_pcPelFormat->colorSpace == CLP_COLOR_RGB ||
        m_pcPelFormat->colorSpace == CLP_COLOR_RGBA )
      m_uiHistoChannels = m_pcPelFormat->numberChannels + 1;
    else
      m_uiHistoChannels = m_pcPelFormat->numberChannels;

    m_puiHistogram.resize( m_uiHistoSegments * m_uiHistoChannels );

    m_bInit = true;
  }

  int getRealHistogramChannel( int channel )
  {
    if( channel < 0 )
    {
      assert( false );
      return 0;
    }
    if( channel < HistogramChannels::HIST_LUMA || channel == HistogramChannels::HIST_ALL_CHANNELS )
    {
      return channel;
    }

    if( m_pcPelFormat->colorSpace == CLP_COLOR_GRAY && channel == HistogramChannels::HIST_LUMA )
    {
      return channel - HistogramChannels::HIST_LUMA;
    }

    if( m_pcPelFormat->colorSpace == CLP_COLOR_YUV && channel >= HistogramChannels::HIST_LUMA && channel < HistogramChannels::HIST_CHROMA_V )
    {
      return channel - HistogramChannels::HIST_LUMA;
    }

    if( m_pcPelFormat->colorSpace == CLP_COLOR_RGB || m_pcPelFormat->colorSpace == CLP_COLOR_RGBA )
    {
      if( channel == HistogramChannels::HIST_LUMA )
      {
        return m_uiHistoChannels - 1;
      }
      if( channel >= HistogramChannels::HIST_COLOR_R && channel <= HistogramChannels::HIST_COLOR_A )
      {
        return channel - CLP_COLOR_R;
      }
    }
    assert( false );
    return 0;
  }

  ~CalypFramePrivate()
  {
    while( m_bHistogramRunning )
      ;
    m_bHasHistogram = false;

    if( m_pppcInputPel )
      xFreeMem( m_pppcInputPel );
  }
};

/*!
 * \brief Constructors
 */
CalypFrame::CalypFrame( unsigned int width, unsigned int height, ClpPixelFormats pelFormat, unsigned bitsPixel )
    : d{ std::make_unique<CalypFramePrivate>() }
{
  d->init( width, height, pelFormat, bitsPixel );
}

CalypFrame::CalypFrame( unsigned int width, unsigned int height, ClpPixelFormats pelFormat, unsigned bitsPixel, bool has_negative_values )
    : d{ std::make_unique<CalypFramePrivate>() }
{
  d->init( width, height, pelFormat, bitsPixel, has_negative_values );
}

CalypFrame::CalypFrame( CalypFrame&& other ) noexcept
    : d{ std::move( other.d ) }
{
}

CalypFrame& CalypFrame::operator=( CalypFrame&& other ) noexcept
{
  // Self-assignment detection
  if( &other == this )
    return *this;

  // Transfer ownership of a.m_ptr to m_ptr
  d = std::move( other.d );

  return *this;
}

CalypFrame::CalypFrame( const CalypFrame& other )
    : d{ std::make_unique<CalypFramePrivate>() }
{
  d->init( other.getWidth(), other.getHeight(), other.getPelFormat(), other.getBitsPel(), other.getHasNegativeValues() );
  copyFrom( &other );
}

CalypFrame& CalypFrame::operator=( const CalypFrame& other )
{
  // Self-assignment detection
  if( &other == this )
    return *this;

  // Copy the resource
  d = std::make_unique<CalypFramePrivate>();
  d->init( other.getWidth(), other.getHeight(), other.getPelFormat(), other.getBitsPel(), other.getHasNegativeValues() );
  copyFrom( &other );

  return *this;
}

CalypFrame::CalypFrame( const CalypFrame& other, unsigned int x, unsigned int y, unsigned int width, unsigned int height )
    : d{ std::make_unique<CalypFramePrivate>() }
{
  const CalypPixelFormatDescriptor* pcPelFormat = &( g_CalypPixFmtDescriptorsMap.at( other.getPelFormat() ) );
  if( pcPelFormat->log2ChromaWidth )
  {
    if( x % ( 1 << pcPelFormat->log2ChromaWidth ) )
      x--;
    if( ( x + width ) % ( 1 << pcPelFormat->log2ChromaWidth ) )
      width++;
  }

  if( pcPelFormat->log2ChromaHeight )
  {
    if( y % ( 1 << pcPelFormat->log2ChromaHeight ) )
      y--;

    if( ( y + height ) % ( 1 << pcPelFormat->log2ChromaHeight ) )
      height++;
  }

  d->init( width, height, other.getPelFormat(), other.getBitsPel() );
  copyFrom( other, x, y );
}

CalypFrame::CalypFrame( const CalypFrame* other, unsigned int posX, unsigned int posY, unsigned int areaWidth, unsigned int areaHeight )
    : d{ std::make_unique<CalypFramePrivate>() }
{
  if( !other )
    return;

  const CalypPixelFormatDescriptor* pcPelFormat = &( g_CalypPixFmtDescriptorsMap.at( other->getPelFormat() ) );
  if( pcPelFormat->log2ChromaWidth )
  {
    if( posX % ( 1 << pcPelFormat->log2ChromaWidth ) )
      posX--;
    if( ( posX + areaWidth ) % ( 1 << pcPelFormat->log2ChromaWidth ) )
      areaWidth++;
  }

  if( pcPelFormat->log2ChromaHeight )
  {
    if( posY % ( 1 << pcPelFormat->log2ChromaHeight ) )
      posY--;

    if( ( posY + areaHeight ) % ( 1 << pcPelFormat->log2ChromaHeight ) )
      areaHeight++;
  }

  d->init( areaWidth, areaHeight, other->getPelFormat(), other->getBitsPel() );
  copyFrom( other, posX, posY );
}

CalypFrame::~CalypFrame() = default;

bool CalypFrame::haveSameFmt( const CalypFrame& other, unsigned int match ) const
{
  bool bRet = true;
  if( match & MATCH_COLOR_SPACE )
    bRet &= ( getColorSpace() == other.getColorSpace() );
  if( match & MATCH_RESOLUTION )
    bRet &= ( getWidth() == other.getWidth() ) && ( getHeight() == other.getHeight() );
  if( match & MATCH_PEL_FMT )
    bRet &= ( getPelFormat() == other.getPelFormat() );
  if( match & MATCH_BITS )
    bRet &= ( getBitsPel() == other.getBitsPel() );
  if( match & MATCH_COLOR_SPACE_IGNORE_GRAY )
    bRet &= ( getColorSpace() == CLP_COLOR_GRAY || getColorSpace() == other.getColorSpace() );
  if( match & MATCH_BYTES_PER_FRAME )
    bRet &= ( getBytesPerFrame() == other.getBytesPerFrame() );
  return bRet;
}

bool CalypFrame::haveSameFmt( const CalypFrame* other, unsigned int match ) const
{
  bool bRet = false;
  if( other )
  {
    bRet = haveSameFmt( *other, match );
  }
  return bRet;
}

auto CalypFrame::getPelFormat() const -> ClpPixelFormats
{
  return d->m_iPixelFormat;
}

auto CalypFrame::getPelFmtName() const -> std::string
{
  return std::string( d->m_pcPelFormat->name );
}

int CalypFrame::getColorSpace() const
{
  return d->m_pcPelFormat->colorSpace;
}

unsigned int CalypFrame::getNumberChannels() const
{
  return d->m_pcPelFormat->numberChannels;
}

unsigned CalypFrame::getWidth( unsigned channel ) const
{
  return CHROMASHIFT( d->m_uiWidth, channel > 0 ? d->m_pcPelFormat->log2ChromaWidth : 0 );
}
unsigned CalypFrame::getHeight( unsigned channel ) const
{
  return CHROMASHIFT( d->m_uiHeight, channel > 0 ? d->m_pcPelFormat->log2ChromaHeight : 0 );
}

bool CalypFrame::getHasNegativeValues() const
{
  return d->m_bHasNegativeValues;
}

std::uint64_t CalypFrame::getPixels( unsigned channel ) const
{
  return getWidth( channel ) * getHeight( channel );
}

std::uint64_t CalypFrame::getTotalNumberOfPixels() const
{
  std::uint64_t numberPixels = 0;
  for( unsigned i = 0; i < getNumberChannels(); i++ )
  {
    numberPixels += getPixels( i );
  }
  return numberPixels;
}

unsigned CalypFrame::getChromaWidthRatio() const
{
  return d->m_pcPelFormat->log2ChromaWidth;
}

unsigned CalypFrame::getChromaHeightRatio() const
{
  return d->m_pcPelFormat->log2ChromaHeight;
}

std::uint64_t CalypFrame::getChromaLength() const
{
  return getWidth( 1 ) * getHeight( 1 );
}

unsigned int CalypFrame::getBitsPel() const
{
  return d->m_uiBitsPel;
}

std::uint64_t CalypFrame::getBytesPerFrame() const
{
  return getBytesPerFrame( d->m_uiWidth, d->m_uiHeight, d->m_iPixelFormat, d->m_uiBitsPel );
}

std::uint64_t CalypFrame::getBytesPerFrame( unsigned int uiWidth, unsigned int uiHeight, ClpPixelFormats pelFormat, unsigned int bitsPixel )
{
  const auto& pcPelFormat = g_CalypPixFmtDescriptorsMap.at( pelFormat );
  unsigned int bytesPerPixel = ( bitsPixel - 1 ) / 8 + 1;
  std::uint64_t numberBytes = uiWidth * uiHeight;
  if( pcPelFormat.numberChannels > 1 )
  {
    std::uint64_t numberBytesChroma =
        CHROMASHIFT( uiWidth, pcPelFormat.log2ChromaWidth ) * CHROMASHIFT( uiHeight, pcPelFormat.log2ChromaHeight );
    numberBytes += ( pcPelFormat.numberChannels - 1 ) * numberBytesChroma;
  }
  return numberBytes * bytesPerPixel;
}

void CalypFrame::reset()
{
  ClpPel* pPel;
  ClpPel pelValue = 1 << ( d->m_uiBitsPel - 1 );
  int ratioH, ratioW;
  unsigned int i;
  for( unsigned int ch = 0; ch < d->m_pcPelFormat->numberChannels; ch++ )
  {
    ratioW = ch > 0 ? d->m_pcPelFormat->log2ChromaWidth : 0;
    ratioH = ch > 0 ? d->m_pcPelFormat->log2ChromaHeight : 0;
    pPel = d->m_pppcInputPel[ch][0];
    for( i = 0; i < CHROMASHIFT( d->m_uiHeight, ratioH ) * CHROMASHIFT( d->m_uiWidth, ratioW ); i++ )
    {
      *pPel++ = pelValue;
    }
  }
}

ClpPel*** CalypFrame::getPelBufferYUV() const
{
  return d->m_pppcInputPel;
}

ClpPel*** CalypFrame::getPelBufferYUV()
{
  d->m_bHasHistogram = false;
  d->m_bHasRGBPel = false;
  return d->m_pppcInputPel;
}

auto CalypFrame::getRGBBuffer() const -> const unsigned char*
{
  if( d->m_bHasRGBPel )
  {
    return d->m_pcARGB32.data();
  }
  return NULL;
}

ClpPel CalypFrame::operator()( unsigned int ch, unsigned int xPos, unsigned int yPos, bool absolute ) const
{
  int retValue = 0;
  if( ch < d->m_pcPelFormat->numberChannels )
  {
    retValue = d->m_pppcInputPel[ch][yPos][xPos];
    if( !absolute && d->m_bHasNegativeValues )
      retValue = retValue - int( d->m_uiHalfPelValue );
  }
  return ClpPel( retValue );
}

CalypPixel CalypFrame::operator()( unsigned int xPos, unsigned int yPos ) const
{
  return getPixel( xPos, yPos );
}

CalypPixel CalypFrame::getPixel( unsigned int xPos, unsigned int yPos ) const
{
  CalypPixel PixelValue( d->m_pcPelFormat->colorSpace );
  for( unsigned int ch = 0; ch < d->m_pcPelFormat->numberChannels; ch++ )
  {
    int ratioH = ch > 0 ? d->m_pcPelFormat->log2ChromaWidth : 0;
    int ratioW = ch > 0 ? d->m_pcPelFormat->log2ChromaHeight : 0;
    PixelValue[ch] = d->m_pppcInputPel[ch][( yPos >> ratioH )][( xPos >> ratioW )];
  }
  return PixelValue;
}

CalypPixel CalypFrame::getPixel( unsigned int xPos, unsigned int yPos, CalypColorSpace eColorSpace ) const
{
  return getPixel( xPos, yPos ).convertPixel( eColorSpace );
}

void CalypFrame::setPixel( unsigned int xPos, unsigned int yPos, CalypPixel pixel )
{
  for( unsigned int ch = 0; ch < d->m_pcPelFormat->numberChannels; ch++ )
  {
    int ratioH = ch > 0 ? d->m_pcPelFormat->log2ChromaWidth : 0;
    int ratioW = ch > 0 ? d->m_pcPelFormat->log2ChromaHeight : 0;
    d->m_pppcInputPel[ch][( yPos >> ratioH )][( xPos >> ratioW )] = pixel[ch];
  }
  d->m_bHasHistogram = false;
  d->m_bHasRGBPel = false;
}

void CalypFrame::copyFrom( const CalypFrame& other )
{
  if( !haveSameFmt( other, MATCH_COLOR_SPACE | MATCH_BYTES_PER_FRAME | MATCH_BITS ) )
    return;
  d->m_bHasRGBPel = false;
  d->m_bHasHistogram = false;
  memcpy( &( d->m_pppcInputPel[0][0][0] ), &( other.getPelBufferYUV()[0][0][0] ), getTotalNumberOfPixels() * sizeof( ClpPel ) );
}

void CalypFrame::copyFrom( const CalypFrame* other )
{
  if( other )
    copyFrom( *other );
}

void CalypFrame::copyFrom( const CalypFrame& other, unsigned x, unsigned y )
{
  if( !haveSameFmt( other, MATCH_COLOR_SPACE | MATCH_BITS ) )
    return;
  // TODO: Protect width and height
  ClpPel*** pInput = other.getPelBufferYUV();
  for( unsigned int ch = 0; ch < d->m_pcPelFormat->numberChannels; ch++ )
  {
    int ratioH = ch > 0 ? d->m_pcPelFormat->log2ChromaWidth : 0;
    int ratioW = ch > 0 ? d->m_pcPelFormat->log2ChromaHeight : 0;
    for( unsigned int i = 0; i < CHROMASHIFT( d->m_uiHeight, ratioH ); i++ )
    {
      memcpy( &( d->m_pppcInputPel[ch][i][0] ), &( pInput[ch][( y >> ratioH ) + i][( x >> ratioW )] ),
              ( d->m_uiWidth >> ratioW ) * sizeof( ClpPel ) );
    }
  }
  d->m_bHasRGBPel = false;
  d->m_bHasHistogram = false;
}

void CalypFrame::copyFrom( const CalypFrame* other, unsigned x, unsigned y )
{
  if( other )
    copyFrom( *other, x, y );
}

void CalypFrame::copyTo( const CalypFrame& other, unsigned x, unsigned y ) const
{
  if( !haveSameFmt( other, MATCH_COLOR_SPACE | MATCH_PEL_FMT | MATCH_BITS ) )
    return;
  ClpPel*** pInput = other.getPelBufferYUV();
  unsigned width = other.getWidth();
  // TODO: Protect width and height
  for( unsigned int ch = 0; ch < d->m_pcPelFormat->numberChannels; ch++ )
  {
    int ratioH = ch > 0 ? d->m_pcPelFormat->log2ChromaWidth : 0;
    int ratioW = ch > 0 ? d->m_pcPelFormat->log2ChromaHeight : 0;
    for( unsigned int i = 0; i < other.getHeight( ch ); i++ )
    {
      memcpy( &( d->m_pppcInputPel[ch][( y >> ratioH ) + i][( x >> ratioW )] ), &( pInput[ch][i][0] ),
              ( width >> ratioW ) * sizeof( ClpPel ) );
    }
  }
  d->m_bHasRGBPel = false;
  d->m_bHasHistogram = false;
}

void CalypFrame::copyTo( const CalypFrame* other, unsigned x, unsigned y ) const
{
  if( other )
    copyTo( *other, x, y );
}

void CalypFrame::frameFromBuffer( const std::vector<ClpByte>& Buff, int iEndianness, unsigned long uiBuffSize )
{
  if( uiBuffSize != getBytesPerFrame() )
    return;

  frameFromBuffer( Buff, iEndianness );
}

void CalypFrame::frameFromBuffer( const std::vector<ClpByte>& Buff, int iEndianness )
{
  const ClpByte* ppBuff[CalypPixel::getMaxNumberOfComponents()];
  unsigned int bytesPixel = ( d->m_uiBitsPel - 1 ) / 8 + 1;
  int startByte = 0;
  int endByte = bytesPixel;
  int incByte = 1;
  int maxval = ( 1 << d->m_uiBitsPel ) - 1;

  if( iEndianness == CLP_BIG_ENDIAN )
  {
    startByte = bytesPixel - 1;
    endByte = -1;
    incByte = -1;
  }

  ppBuff[0] = Buff.data();
  for( unsigned i = 1; i < CalypPixel::getMaxNumberOfComponents(); i++ )
  {
    int ratioW = i > 1 ? d->m_pcPelFormat->log2ChromaWidth : 0;
    int ratioH = i > 1 ? d->m_pcPelFormat->log2ChromaHeight : 0;
    ppBuff[i] = ppBuff[i - 1] + CHROMASHIFT( d->m_uiHeight, ratioH ) * CHROMASHIFT( d->m_uiWidth, ratioW ) * bytesPixel;
  }

  memset( d->m_pppcInputPel[0][0], 0, getTotalNumberOfPixels() * sizeof( ClpPel ) );

  for( unsigned ch = 0; ch < d->m_pcPelFormat->numberChannels; ch++ )
  {
    int ratioW = ch > 0 ? d->m_pcPelFormat->log2ChromaWidth : 0;
    int ratioH = ch > 0 ? d->m_pcPelFormat->log2ChromaHeight : 0;
    int step = ( d->m_pcPelFormat->comp[ch].step_minus1 ) * bytesPixel;

    ClpPel* pPel = d->m_pppcInputPel[ch][0];
    const ClpByte* pTmpBuff = ppBuff[d->m_pcPelFormat->comp[ch].plane] + ( d->m_pcPelFormat->comp[ch].offset_plus1 - 1 ) * bytesPixel;

    for( unsigned p = 0; p < CHROMASHIFT( d->m_uiHeight, ratioH ) * CHROMASHIFT( d->m_uiWidth, ratioW ); p++ )
    {
      for( int b = startByte; b != endByte; b += incByte )
      {
        pPel[p] += *pTmpBuff << ( b * 8 );
        pTmpBuff++;
      }
      // Check max value and bound it to "maxval" to prevent segfault when
      // calculating histogram
      if( pPel[p] > maxval )
        pPel[p] = 0;
      pTmpBuff += step;
    }
  }
  d->m_bHasRGBPel = false;
  d->m_bHasHistogram = false;
}

void CalypFrame::frameToBuffer( std::vector<ClpByte>& output_buffer, int iEndianness ) const
{
  unsigned int bytesPixel = ( d->m_uiBitsPel - 1 ) / 8 + 1;
  ClpByte* ppBuff[CalypPixel::getMaxNumberOfComponents()];
  ClpByte* pTmpBuff;
  ClpPel* pTmpPel;
  int ratioH, ratioW, step;
  unsigned int ch;
  int startByte = 0;
  int endByte = bytesPixel;
  int incByte = 1;
  int b;

  if( iEndianness == CLP_BIG_ENDIAN )
  {
    startByte = bytesPixel - 1;
    endByte = -1;
    incByte = -1;
  }

  ppBuff[0] = output_buffer.data();
  for( std::size_t i = 1; i < CalypPixel::getMaxNumberOfComponents(); i++ )
  {
    ratioW = i > 1 ? d->m_pcPelFormat->log2ChromaWidth : 0;
    ratioH = i > 1 ? d->m_pcPelFormat->log2ChromaHeight : 0;
    ppBuff[i] = ppBuff[i - 1] + CHROMASHIFT( d->m_uiHeight, ratioH ) * CHROMASHIFT( d->m_uiWidth, ratioW ) * bytesPixel;
  }

  for( ch = 0; ch < d->m_pcPelFormat->numberChannels; ch++ )
  {
    ratioW = ch > 0 ? d->m_pcPelFormat->log2ChromaWidth : 0;
    ratioH = ch > 0 ? d->m_pcPelFormat->log2ChromaHeight : 0;
    step = ( d->m_pcPelFormat->comp[ch].step_minus1 ) * bytesPixel;

    pTmpPel = d->m_pppcInputPel[ch][0];
    pTmpBuff = ppBuff[d->m_pcPelFormat->comp[ch].plane] + ( d->m_pcPelFormat->comp[ch].offset_plus1 - 1 ) * bytesPixel;

    for( std::size_t i = 0; i < CHROMASHIFT( d->m_uiHeight, ratioH ) * CHROMASHIFT( d->m_uiWidth, ratioW ); i++ )
    {
      for( b = startByte; b != endByte; b += incByte )
      {
        *pTmpBuff = *pTmpPel >> ( 8 * b );
        pTmpBuff++;
      }
      pTmpPel++;
      pTmpBuff += step;
    }
  }
}

#define PEL_ARGB( a, r, g, b ) ( ( a & 0xff ) << 24 ) | ( ( r & 0xff ) << 16 ) | ( ( g & 0xff ) << 8 ) | ( b & 0xff )
#define PEL_RGB( r, g, b ) PEL_ARGB( 0xffu, r, g, b )
#define CLAMP_YUV2RGB( X ) X = X < 0 ? 0 : X > 255 ? 255 : \
                                                     X;
#define YUV2RGB( iY, iU, iV, iR, iG, iB )                          \
  iR = iY + ( ( 1436 * ( iV - 128 ) ) >> 10 );                     \
  iG = iY - ( ( 352 * ( iU - 128 ) + 731 * ( iV - 128 ) ) >> 10 ); \
  iB = iY + ( ( 1812 * ( iU - 128 ) ) >> 10 );                     \
  CLAMP_YUV2RGB( iR )                                              \
  CLAMP_YUV2RGB( iG )                                              \
  CLAMP_YUV2RGB( iB )

void fillRGBBufferYUV420p( ClpPel*** pppInputPel, uint32_t* pARGB, unsigned uiWidth, unsigned uiHeight, int shiftBits )
{
  ClpPel* pY = pppInputPel[CLP_LUMA][0];
  ClpPel* pU = pppInputPel[CLP_CHROMA_U][0];
  ClpPel* pV = pppInputPel[CLP_CHROMA_V][0];

  for( unsigned y = 0; y < uiHeight; y += 2 )
  {
    for( unsigned x = 0; x < uiWidth; x += 2 )
    {
      int iY, iU, iV, iR, iG, iB;
      iY = pY[y * uiWidth + x] >> shiftBits;
      iU = pU[y * uiWidth / 4 + x / 2] >> shiftBits;
      iV = pV[y * uiWidth / 4 + x / 2] >> shiftBits;
      YUV2RGB( iY, iU, iV, iR, iG, iB );
      pARGB[y * uiWidth + x] = PEL_RGB( iR, iG, iB );

      iY = pY[y * uiWidth + x + 1] >> shiftBits;
      YUV2RGB( iY, iU, iV, iR, iG, iB );
      pARGB[y * uiWidth + x + 1] = PEL_RGB( iR, iG, iB );

      iY = pY[y * uiWidth + uiWidth + x] >> shiftBits;
      YUV2RGB( iY, iU, iV, iR, iG, iB );
      pARGB[y * uiWidth + uiWidth + x] = PEL_RGB( iR, iG, iB );

      iY = pY[y * uiWidth + uiWidth + x + 1] >> shiftBits;
      YUV2RGB( iY, iU, iV, iR, iG, iB );
      pARGB[y * uiWidth + uiWidth + x + 1] = PEL_RGB( iR, iG, iB );
    }
  }
}

void CalypFrame::fillRGBBuffer() const
{
  if( d->m_bHasRGBPel )
    return;
  int shiftBits = d->m_uiBitsPel - 8;

  d->m_bHasRGBPel = true;
  // 4 bytes for A, R, G and B
  uint32_t* pARGB = (uint32_t*)d->m_pcARGB32.data();
  if( d->m_pcPelFormat->colorSpace == CLP_COLOR_GRAY )
  {
    ClpPel* pY = d->m_pppcInputPel[CLP_LUMA][0];
    unsigned char finalPel{ 0 };
    for( unsigned int i = 0; i < d->m_uiHeight * d->m_uiWidth; i++ )
    {
      finalPel = ( *pY++ ) >> shiftBits;
      *pARGB++ = PEL_RGB( finalPel, finalPel, finalPel );
    }
  }
  else if( d->m_pcPelFormat->colorSpace == CLP_COLOR_RGB )
  {
    ClpPel* pR = d->m_pppcInputPel[CLP_COLOR_R][0];
    ClpPel* pG = d->m_pppcInputPel[CLP_COLOR_G][0];
    ClpPel* pB = d->m_pppcInputPel[CLP_COLOR_B][0];
    for( unsigned int i = 0; i < d->m_uiHeight * d->m_uiWidth; i++ )
      *pARGB++ = PEL_RGB( ( *pR++ ) >> shiftBits, ( *pG++ ) >> shiftBits, ( *pB++ ) >> shiftBits );
  }
  else if( d->m_pcPelFormat->colorSpace == CLP_COLOR_RGBA )
  {
    ClpPel* pR = d->m_pppcInputPel[CLP_COLOR_R][0];
    ClpPel* pG = d->m_pppcInputPel[CLP_COLOR_G][0];
    ClpPel* pB = d->m_pppcInputPel[CLP_COLOR_B][0];
    ClpPel* pA = d->m_pppcInputPel[CLP_COLOR_A][0];
    for( unsigned int i = 0; i < d->m_uiHeight * d->m_uiWidth; i++ )
      *pARGB++ = PEL_ARGB( ( *pA++ ) >> shiftBits, ( *pR++ ) >> shiftBits, ( *pG++ ) >> shiftBits, ( *pB++ ) >> shiftBits );
  }
  else if( d->m_pcPelFormat->colorSpace == CLP_COLOR_YUV )
  {
    if( d->m_iPixelFormat == ClpPixelFormats::YUV420p )
    {
      fillRGBBufferYUV420p( d->m_pppcInputPel, pARGB, d->m_uiWidth, d->m_uiHeight, shiftBits );
      return;
    }
    ClpPel* pLineY = d->m_pppcInputPel[CLP_LUMA][0];
    ClpPel* pLineU = d->m_pppcInputPel[CLP_CHROMA_U][0];
    ClpPel* pLineV = d->m_pppcInputPel[CLP_CHROMA_V][0];
    unsigned int uiChromaStride = CHROMASHIFT( d->m_uiWidth, d->m_pcPelFormat->log2ChromaWidth );

    int iR, iG, iB;
    uint32_t* pARGBLine = pARGB;
    uint32_t* pARGBAux;

    for( unsigned y = 0; y < CHROMASHIFT( d->m_uiHeight, d->m_pcPelFormat->log2ChromaHeight ); y++ )
    {
      for( int i = 0; i < 1 << d->m_pcPelFormat->log2ChromaHeight; i++ )
      {
        ClpPel* pY = pLineY;
        ClpPel* pU = pLineU;
        ClpPel* pV = pLineV;
        pARGBAux = pARGBLine;
        for( unsigned x = 0; x < CHROMASHIFT( d->m_uiWidth, d->m_pcPelFormat->log2ChromaWidth ); x++ )
        {
          int iU = *pU++;
          iU >>= shiftBits;
          int iV = *pV++;
          iV >>= shiftBits;
          for( int j = 0; j < ( 1 << d->m_pcPelFormat->log2ChromaWidth ); j++ )
          {
            int iY = *pY++;
            iY >>= shiftBits;
            YUV2RGB( iY, iU, iV, iR, iG, iB );
            *pARGBAux++ = PEL_RGB( iR, iG, iB );
          }
        }
        pLineY += d->m_uiWidth;
        pARGBLine += d->m_uiWidth;
      }
      pLineU += uiChromaStride;
      pLineV += uiChromaStride;
    }
  }
}

/**
 * Histogram
 */

void CalypFrame::calcHistogram()
{
  if( d->m_bHasHistogram || d->m_puiHistogram.empty() )
    return;

  d->m_bHistogramRunning = true;

  std::fill( d->m_puiHistogram.begin(), d->m_puiHistogram.end(), 0 );

  unsigned int numberChannels = d->m_pcPelFormat->numberChannels;
  for( unsigned int ch = 0; ch < numberChannels; ch++ )
  {
    unsigned int size = CHROMASHIFT( d->m_uiWidth, ch > 0 ? d->m_pcPelFormat->log2ChromaWidth : 0 ) *
                        CHROMASHIFT( d->m_uiHeight, ch > 0 ? d->m_pcPelFormat->log2ChromaHeight : 0 );

    ClpPel* chPel = &( d->m_pppcInputPel[ch][0][0] );
    for( unsigned int i = 0; i < size; i++ )
    {
      d->m_puiHistogram[*chPel + ch * d->m_uiHistoSegments]++;
      chPel++;
    }
  }

  if( d->m_pcPelFormat->colorSpace == CLP_COLOR_RGB ||
      d->m_pcPelFormat->colorSpace == CLP_COLOR_RGBA )
  {
    for( unsigned int y = 0; y < d->m_uiHeight; y++ )
      for( unsigned int x = 0; x < d->m_uiWidth; x++ )
      {
        ClpPel luma = getPixel( x, y ).convertPixel( CLP_COLOR_YUV )[0];
        d->m_puiHistogram[luma + ( d->m_uiHistoChannels - 1 ) * d->m_uiHistoSegments]++;
      }
  }
  d->m_bHasHistogram = true;
  d->m_bHistogramRunning = false;
}

int CalypFrame::getNumHistogramSegment() const
{
  return d->m_uiHistoSegments;
}

unsigned int CalypFrame::getMinimumPelValue( unsigned channel ) const
{
  if( !d->m_bHasHistogram )
    return 0;

  channel = d->getRealHistogramChannel( channel );
  if( channel < 0 )
    return 0;

  int indexStart;
  int indexEnd;
  if( channel == HIST_ALL_CHANNELS )
  {
    indexStart = 0;
    indexEnd = d->m_uiHistoChannels * d->m_uiHistoSegments;
  }
  else
  {
    indexStart = channel * d->m_uiHistoSegments;
    indexEnd = indexStart + d->m_uiHistoSegments;
  }

  for( int i = indexStart; i < indexEnd; i++ )
  {
    if( d->m_puiHistogram[i] > 0 )
    {
      return i - indexStart;
    }
  }
  return 0;
}

unsigned int CalypFrame::getMaximumPelValue( unsigned channel ) const
{
  if( !d->m_bHasHistogram )
    return 0;

  channel = d->getRealHistogramChannel( channel );
  if( channel < 0 )
    return 0;

  int indexStart;
  int indexEnd;
  if( channel == HIST_ALL_CHANNELS )
  {
    indexStart = d->m_uiHistoChannels * d->m_uiHistoSegments;
    indexEnd = 0;
  }
  else
  {
    indexStart = ( channel + 1 ) * d->m_uiHistoSegments - 1;
    indexEnd = indexStart - d->m_uiHistoSegments;
  }

  for( int i = indexStart; i > indexEnd; i-- )
  {
    if( d->m_puiHistogram[i] > 0 )
    {
      return i - indexEnd - 1;
    }
  }
  return 0;
}

unsigned int CalypFrame::getNEBins( unsigned channel ) const
{
  if( !d->m_bHasHistogram )
    return 0;

  channel = d->getRealHistogramChannel( channel );
  if( channel < 0 )
    return 0;

  int indexStart;
  int indexEnd;
  if( channel == HIST_ALL_CHANNELS )
  {
    indexStart = 0;
    indexEnd = d->m_uiHistoChannels * d->m_uiHistoSegments;
  }
  else
  {
    indexStart = channel * d->m_uiHistoSegments;
    indexEnd = indexStart + d->m_uiHistoSegments;
  }

  int nEBins = 0;
  for( int i = indexStart; i < indexEnd; i++ )
  {
    if( d->m_puiHistogram[i] > 0 )
    {
      nEBins++;
    }
  }
  return nEBins;
}

unsigned int CalypFrame::getMaximum( unsigned channel ) const
{
  if( !d->m_bHasHistogram )
    return 0;

  channel = d->getRealHistogramChannel( channel );
  if( channel < 0 )
    return 0;

  unsigned int maxValue = 0;
  int indexStart;
  int indexEnd;
  if( channel == HIST_ALL_CHANNELS )
  {
    indexStart = 0;
    indexEnd = d->m_uiHistoChannels * d->m_uiHistoSegments;
  }
  else
  {
    indexStart = channel * d->m_uiHistoSegments;
    indexEnd = indexStart + d->m_uiHistoSegments;
  }

  for( int x = indexStart; x < indexEnd; x++ )
  {
    if( d->m_puiHistogram[x] > maxValue )
    {
      maxValue = d->m_puiHistogram[x];
    }
  }
  return maxValue;
}

unsigned int CalypFrame::getNumPixelsRange( unsigned channel, unsigned int start, unsigned int end ) const
{
  if( !d->m_bHasHistogram || start < 0 || end > d->m_uiHistoSegments - 1 || start > end )
  {
    return 0;
  }
  channel = d->getRealHistogramChannel( channel );
  if( channel < 0 )
  {
    return 0;
  }

  unsigned int count = 0;
  int indexStart;
  indexStart = channel * d->m_uiHistoSegments;
  for( unsigned int i = start; i <= end; i++ )
  {
    count += d->m_puiHistogram[indexStart + i];
  }
  return count;
}

double CalypFrame::getMean( unsigned channel, unsigned int start, unsigned int end ) const
{
  if( !d->m_bHasHistogram || start < 0 || end > d->m_uiHistoSegments - 1 || start > end )
  {
    return 0.0;
  }
  channel = d->getRealHistogramChannel( channel );
  if( channel < 0 )
  {
    return 0.0;
  }

  double mean = 0.0;
  double count;
  int indexStart = channel * d->m_uiHistoSegments;
  for( unsigned int i = start; i <= end; i++ )
  {
    mean += i * d->m_puiHistogram[indexStart + i];
  }

  count = getNumPixelsRange( channel, start, end );

  if( count > 0.0 )
  {
    return mean / count;
  }

  return mean;
}

int CalypFrame::getMedian( unsigned channel, unsigned int start, unsigned int end ) const
{
  if( !d->m_bHasHistogram || start < 0 || end > d->m_uiHistoSegments - 1 || start > end )
  {
    return 0;
  }

  channel = d->getRealHistogramChannel( channel );
  if( channel < 0 )
  {
    return 0;
  }

  double sum = 0.0;
  int indexStart = channel * d->m_uiHistoSegments;
  double count = getNumPixelsRange( channel, start, end );
  for( unsigned int i = start; i <= end; i++ )
  {
    sum += d->m_puiHistogram[indexStart + i];
    if( sum * 2 > count )
      return i;
  }

  return 0;
}

double CalypFrame::getStdDev( unsigned channel, unsigned int start, unsigned int end ) const
{
  if( !d->m_bHasHistogram || start < 0 || end > d->m_uiHistoSegments - 1 || start > end )
  {
    return 0.0;
  }

  channel = d->getRealHistogramChannel( channel );
  if( channel < 0 )
  {
    return 0.0;
  }

  int indexStart = channel * d->m_uiHistoSegments;
  double mean = getMean( channel, start, end );
  double count = getNumPixelsRange( channel, start, end );
  double dev = 0.0;
  if( count == 0.0 )
    count = 1.0;

  /*------------ original

     for ( i = start ; i <= end ; i++ )
     {
     dev += ( i - mean ) * ( i - mean ) * d->m_puiHistogram[indexStart +i];
     }

     return sqrt( dev / count );

     -----------------------*/

  for( unsigned int i = start; i <= end; i++ )
  {
    dev += ( i * i ) * d->m_puiHistogram[indexStart + i];
  }

  return sqrt( ( dev - count * mean * mean ) / ( count - 1 ) );
}

double CalypFrame::getHistogramValue( unsigned channel, unsigned int bin ) const
{
  if( !d->m_bHasHistogram || bin < 0 || bin > d->m_uiHistoSegments - 1 )
    return 0.0;

  channel = d->getRealHistogramChannel( channel );
  if( channel < 0 )
  {
    return 0;
  }

  int indexStart = channel * d->m_uiHistoSegments;
  return d->m_puiHistogram[indexStart + bin];
}

double CalypFrame::getEntropy( unsigned channel, unsigned int start, unsigned int end ) const
{
  if( !d->m_bHasHistogram )
    return 0;

  channel = d->getRealHistogramChannel( channel );
  if( channel < 0 )
    return 0;

  int indexStart = channel * d->m_uiHistoSegments;
  double numValues = getNumPixelsRange( channel, start, end );
  double entropy = 0.0;

  for( unsigned b = start; b <= end; b++ )
  {
    if( d->m_puiHistogram[indexStart + b] == 0 )
      continue;

    double prob = d->m_puiHistogram[indexStart + b] / numValues;

    entropy -= prob * log2( prob );
  }

  return entropy;
}

/*
 **************************************************************
 * interface to other libs
 **************************************************************
 */

bool CalypFrame::toMat( cv::Mat& cvMat, bool convertToGray, bool scale, unsigned channel ) const
{
  bool bRet = false;
#ifdef USE_OPENCV
  if( convertToGray && !( d->m_pcPelFormat->colorSpace == CLP_COLOR_YUV ||
                          d->m_pcPelFormat->colorSpace == CLP_COLOR_GRAY ) )
  {
    return bRet;
  }
  unsigned int cvPrecision = getBitsPel() > 8 ? CV_16U : CV_8U;
  unsigned numBytes = getBitsPel() > sizeof( char ) ? 2 : 1;
  unsigned numChannels = getNumberChannels();
  unsigned scaleFactor = 1 << ( numBytes * 8 - getBitsPel() );
  if( !scale )
    scaleFactor = 1;
  if( convertToGray )
  {
    channel = channel >= numChannels ? 0 : channel;
    numChannels = 1;
  }

  unsigned imgWidth = getWidth( channel );
  unsigned imgHeight = getHeight( channel );

  cvMat.create( imgHeight, imgWidth, CV_MAKETYPE( cvPrecision, numChannels ) );
  if( numChannels > 1 )
  {
    cv::Mat tmpMat( imgHeight, imgWidth, CV_MAKETYPE( cvPrecision, numChannels ) );

    unsigned char* cv_data = tmpMat.data;
    for( unsigned y = 0; y < imgHeight; y++ )
    {
      for( unsigned x = 0; x < imgWidth; x++ )
      {
        CalypPixel currPel = getPixel( x, y );
        currPel *= scaleFactor;
        for( unsigned int ch = 0; ch < numChannels; ch++ )
          for( unsigned b = 0; b < numBytes; b++ )
          {
            unsigned char pel = currPel[ch] >> ( 8 * b );
            *cv_data++ = pel;
          }
      }
    }
    if( getNumberChannels() >= 1 )
    {
      // TODO: check for other formats
      switch( getColorSpace() )
      {
      case CLP_COLOR_YUV:
        cv::cvtColor( tmpMat, cvMat, cv::COLOR_YCrCb2RGB );
        break;
      case CLP_COLOR_RGB:
      case CLP_COLOR_RGBA:
        cvMat = tmpMat;
        break;
      default:
        assert( false );
      }
    }
  }
  else
  {
    unsigned char* cv_data = cvMat.data;
    ClpPel* pel = getPelBufferYUV()[channel][0];
    for( unsigned y = 0; y < imgHeight * imgWidth; y++ )
    {
      ClpPel currPel = ( *pel++ ) * scaleFactor;
      for( unsigned b = 0; b < numBytes; b++ )
        *cv_data++ = currPel >> ( 8 * b );
    }
  }
  bRet = true;
#endif
  return bRet;
}

bool CalypFrame::fromMat( cv::Mat& cvMat, int channel )
{
  bool bRet = false;
#ifdef USE_OPENCV
  unsigned numBytes = getBitsPel() > 8 ? 2 : 1;
  unsigned numChannels = getNumberChannels();
  unsigned int cvPrecision = getBitsPel() > 8 ? CV_16U : CV_8U;
  if( !d->m_bInit )
  {
    uchar depth = cvMat.type() & CV_MAT_DEPTH_MASK;
    if( d->m_iPixelFormat == ClpPixelFormats::Invalid )
    {
      switch( cvMat.channels() )
      {
      case 1:
        d->m_iPixelFormat = ClpPixelFormats::Gray;
        break;
      case 3:
        d->m_iPixelFormat = ClpPixelFormats::BGR24;
        break;
      default:
        assert( false );
        return false;
      }
    }
    d->m_uiBitsPel = depth == CV_8U ? 8 : 16;
    d->init( cvMat.cols, cvMat.rows, d->m_iPixelFormat, d->m_uiBitsPel );
  }

  d->m_bHasRGBPel = false;
  d->m_bHasHistogram = false;

  if( channel >= 0 )
    numChannels = 1;
  else
    channel = 0;

  if( cvMat.channels() != int( numChannels ) )
  {
    return false;
  }

  unsigned imgWidth = getWidth( channel );
  unsigned imgHeight = getHeight( channel );

  if( numChannels > 1 )
  {
    cv::Mat tmpMat( imgHeight, imgWidth, CV_MAKETYPE( cvPrecision, numChannels ) );
    switch( getColorSpace() )
    {
    case CLP_COLOR_YUV:
      cv::cvtColor( tmpMat, cvMat, cv::COLOR_RGB2YCrCb );
      break;
    }
    unsigned char* cv_data = tmpMat.data;
    unsigned int cv_step = tmpMat.step;
    CalypPixel currPel;
    for( unsigned int y = 0; y < imgHeight; y++ )
    {
      for( unsigned int x = 0; x < imgWidth; x++ )
      {
        for( unsigned int ch = 0; ch < numChannels; ch++ )
        {
          currPel[ch] = 0;
          for( unsigned b = 0; b < numBytes; b++ )
          {
            currPel[ch] = currPel[ch] + ( *( cv_data + y * cv_step + x * numChannels * numBytes + ch + b ) << ( 8 * b ) );
          }
        }
        setPixel( x, y, currPel );
      }
    }
  }
  else
  {
    unsigned char* cv_data = cvMat.data;
    ClpPel curr_pel;
    ClpPel* pel = getPelBufferYUV()[channel][0];
    for( unsigned int y = 0; y < imgHeight * imgWidth; y++ )
    {
      curr_pel = 0;
      for( unsigned b = 0; b < numBytes; b++ )
        curr_pel += ( *cv_data++ ) << ( 8 * b );
      *pel++ = curr_pel;
    }
  }

  bRet = true;
#endif
  return bRet;
}

/*
 **************************************************************
 * Quality Related Function API
 **************************************************************
 */

std::vector<std::string> CalypFrame::supportedQualityMetricsList()
{
  return std::vector<std::string>{
      "PSNR",
      "MSE",
      "SSIM",
      "WS-PSNR",
  };
}

std::vector<std::string> CalypFrame::supportedQualityMetricsUnitsList()
{
  return std::vector<std::string>{
      "dB",
      "",
      "",
      "dB",
  };
}

double CalypFrame::getQuality( int Metric, CalypFrame* Org, unsigned int component )
{
  if( component >= getNumberChannels() )
  {
    return 0;
  }
  switch( Metric )
  {
  case PSNR_METRIC:
    return getPSNR( Org, component );
    break;
  case MSE_METRIC:
    return getMSE( Org, component );
    break;
  case SSIM_METRIC:
    return getSSIM( Org, component );
    break;
  case WSPSNR_METRIC:
    return getWSPNR( Org, component );
    break;
  default:
    assert( 0 );
  };
  return 0;
}

double CalypFrame::getMSE( CalypFrame* Org, unsigned int component )
{
  ClpPel* pPelYUV = getPelBufferYUV()[component][0];
  ClpPel* pOrgPelYUV = Org->getPelBufferYUV()[component][0];
  std::uint64_t numberOfPixels = Org->getHeight( component ) * Org->getWidth( component );
  std::uint64_t ssd = 0;
  for( unsigned int i = 0; i < numberOfPixels; i++ )
  {
    int diff = int( *pPelYUV++ ) - int( *pOrgPelYUV++ );
    ssd += diff * diff;
  }
  if( ssd == 0.0 )
  {
    return 0.0;
  }
  return double( ssd ) / double( numberOfPixels );
}

double CalypFrame::getPSNR( CalypFrame* Org, unsigned int component )
{
  std::uint64_t uiMaxValue = ( 1 << Org->getBitsPel() ) - 1;
  double dPSNR = 100;
  double dMSE = getMSE( Org, component );
  if( dMSE != 0 )
    dPSNR = 10 * log10( double( uiMaxValue * uiMaxValue ) / dMSE );
  return dPSNR;
}

float compute_ssim( ClpPel** refImg, ClpPel** encImg, int width, int height, int win_width, int win_height,
                    int max_pel_value_comp, int overlapSize )
{
  static const float K1 = 0.01f, K2 = 0.03f;
  float max_pix_value_sqd;
  float C1, C2;
  float win_pixels = (float)( win_width * win_height );
#ifdef UNBIASED_VARIANCE
  float win_pixels_bias = win_pixels - 1;
#else
  float win_pixels_bias = win_pixels;
#endif
  float mb_ssim, meanOrg, meanEnc;
  float varOrg, varEnc, covOrgEnc;
  int imeanOrg, imeanEnc, ivarOrg, ivarEnc, icovOrgEnc;
  float cur_distortion = 0.0;
  int i, j, n, m, win_cnt = 0;

  max_pix_value_sqd = (float)( max_pel_value_comp * max_pel_value_comp );
  C1 = K1 * K1 * max_pix_value_sqd;
  C2 = K2 * K2 * max_pix_value_sqd;

  for( j = 0; j <= height - win_height; j += overlapSize )
  {
    for( i = 0; i <= width - win_width; i += overlapSize )
    {
      imeanOrg = 0;
      imeanEnc = 0;
      ivarOrg = 0;
      ivarEnc = 0;
      icovOrgEnc = 0;

      for( n = j; n < j + win_height; n++ )
      {
        for( m = i; m < i + win_width; m++ )
        {
          imeanOrg += refImg[n][m];
          imeanEnc += encImg[n][m];
          ivarOrg += refImg[n][m] * refImg[n][m];
          ivarEnc += encImg[n][m] * encImg[n][m];
          icovOrgEnc += refImg[n][m] * encImg[n][m];
        }
      }

      meanOrg = (float)imeanOrg / win_pixels;
      meanEnc = (float)imeanEnc / win_pixels;

      varOrg = ( (float)ivarOrg - ( (float)imeanOrg ) * meanOrg ) / win_pixels_bias;
      varEnc = ( (float)ivarEnc - ( (float)imeanEnc ) * meanEnc ) / win_pixels_bias;
      covOrgEnc = ( (float)icovOrgEnc - ( (float)imeanOrg ) * meanEnc ) / win_pixels_bias;

      mb_ssim = (float)( ( 2.0 * meanOrg * meanEnc + C1 ) * ( 2.0 * covOrgEnc + C2 ) );
      mb_ssim /= (float)( meanOrg * meanOrg + meanEnc * meanEnc + C1 ) * ( varOrg + varEnc + C2 );

      cur_distortion += mb_ssim;
      win_cnt++;
    }
  }

  cur_distortion /= (float)win_cnt;

  if( cur_distortion >= 1.0 && cur_distortion < 1.01 )  // avoid float accuracy problem at very low QP(e.g.2)
    cur_distortion = 1.0;

  return cur_distortion;
}

double CalypFrame::getSSIM( CalypFrame* Org, unsigned int component )
{
  double dSSIM = 1;
  if( component == CLP_LUMA )
  {
    dSSIM = compute_ssim( d->m_pppcInputPel[component], Org->getPelBufferYUV()[component], d->m_uiWidth, d->m_uiHeight,
                          8, 8, 255, 8 );
  }
  else
  {
    dSSIM = compute_ssim( d->m_pppcInputPel[component], Org->getPelBufferYUV()[component], getWidth( component ),
                          getHeight( component ), 4, 4, 255, 4 );
  }
  return dSSIM;
}

double CalypFrame::getWSPNR( CalypFrame* Org, unsigned int component )
{
  ClpPel* pPelYUV = getPelBufferYUV()[component][0];
  ClpPel* pOrgPelYUV = Org->getPelBufferYUV()[component][0];

  unsigned height = getHeight( component );
  unsigned width = getWidth( component );
  double ssd = 0;
  double weight_sum = 0;

  for( unsigned y = 0; y < height; y++ )
    for( unsigned x = 0; x < width; x++ )
    {
      int diff = int( *pPelYUV++ ) - int( *pOrgPelYUV++ );
      double weight = cos( double( ( y + 0.5 - height / 2.0 ) * S_PI / height ) );
      ssd += (double)( diff * diff * 1000 ) * weight;
      weight_sum += weight * 1000;
    }

  if( ssd == 0.0 )
  {
    return 100.00;
  }
  unsigned long uiMaxValue = ( 1 << Org->getBitsPel() ) - 1;
  return 10 * log10( double( uiMaxValue * uiMaxValue ) * weight_sum / ssd );
}

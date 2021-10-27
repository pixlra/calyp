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
 * \file     CalypFrame.h
 * \ingroup	 CalypFrameGrp
 * \brief    Video Frame handling
 */

#ifndef __CALYPFRAME_H__
#define __CALYPFRAME_H__

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace cv
{
class Mat;
}

using ClpPel = std::uint16_t;
using ClpByte = std::uint8_t;

/**
 * \enum CalypColorSpace
 * \brief List of supported color spaces
 * \ingroup CalypLibGrp
 */
enum CalypColorSpace
{
  CLP_COLOR_INVALID = -1,  //!< Invalid
  CLP_COLOR_YUV = 0,       //!< YUV
  CLP_COLOR_RGB = 1,       //!< RGB
  CLP_COLOR_GRAY = 2,      //!< Grayscale
  CLP_COLOR_RGBA = 3,      //!< RGB + Alpha
  CLP_COLOR_MAX = 255,     //!< Account for future formats
};

/**
 * \enum ClpPixelFormats
 * \brief List of supported pixel formats
 * \ingroup CalypLibGrp
 */
enum class ClpPixelFormats : int
{
  Invalid = -1,  //!< Invalid
  YUV420p = 0,   //!< YUV 420 progressive
  YUV422p,       //!< YUV 422 progressive
  YUV444p,       //!< YUV 444 progressive
  YUYV422,       //!< YUV 422 interleaved
  Gray,          //!< Grayscale
  RGB24p,        //!< RGB 32 bpp progressive
  RGB24,         //!< RGB 32 bpp
  BGR24,         //!< BGR 32 bpp
  RGBA32,        //!< RGBA 32 bpp
  BGRA32,        //!< BGRA 32 bpp
};

enum CLP_YUV_Components
{
  CLP_LUMA = 0,
  CLP_CHROMA_U,
  CLP_CHROMA_V,
};

enum CLP_RGB_Components
{
  CLP_COLOR_R = 0,
  CLP_COLOR_G,
  CLP_COLOR_B,
  CLP_COLOR_A,
};

enum CLP_Endianness
{
  CLP_INVALID_ENDIANESS = -1,
  CLP_BIG_ENDIAN = 0,
  CLP_LITTLE_ENDIAN = 1,
};

#define CHROMASHIFT( SIZE, SHIFT ) (unsigned int)( -( ( -( (int)( SIZE ) ) ) >> ( SHIFT ) ) )

/**
 * \class    CalypPixel
 * \ingroup  CalypLibGrp CalypFrameGrp
 * \brief    Pixel handling class
 */
class CalypPixel
{
private:
  static constexpr std::size_t kMaxNumberOfComponents{ 4 };
  int m_colorSpace{ CLP_COLOR_INVALID };
  std::array<ClpPel, kMaxNumberOfComponents> m_pelComp{ 0, 0, 0, 0 };

public:
  static constexpr std::size_t getMaxNumberOfComponents() { return kMaxNumberOfComponents; }

  CalypPixel() = default;
  CalypPixel( const int CalypColorSpace, const ClpPel c0 = 0 );
  CalypPixel( const int CalypColorSpace, const ClpPel c0, const ClpPel c1, const ClpPel c2 );
  CalypPixel( const int CalypColorSpace, const ClpPel c0, const ClpPel c1, const ClpPel c2, const ClpPel c3 );
  CalypPixel( const CalypPixel& other ) noexcept = default;
  CalypPixel( CalypPixel&& other ) noexcept = default;
  auto operator=( const CalypPixel& other ) -> CalypPixel& = default;
  auto operator=( CalypPixel&& other ) noexcept -> CalypPixel& = default;
  ~CalypPixel() = default;

  int colorSpace() const { return m_colorSpace; };

  auto components() const -> const std::array<ClpPel, kMaxNumberOfComponents>& { return m_pelComp; };
  auto components() -> std::array<ClpPel, kMaxNumberOfComponents>& { return m_pelComp; };

  ClpPel operator[]( const std::size_t channel ) const { return m_pelComp[channel]; }
  ClpPel& operator[]( const std::size_t channel ) { return m_pelComp[channel]; }

  CalypPixel& operator+=( const CalypPixel& );
  CalypPixel& operator-=( const CalypPixel& );
  CalypPixel& operator*=( const double );

  CalypPixel operator+( const CalypPixel& ) const;
  CalypPixel operator-( const CalypPixel& ) const;
  CalypPixel operator*( const double ) const;

  auto operator==( const CalypPixel& other ) const -> bool;

  friend std::ostream& operator<<( std::ostream& os, const CalypPixel& p );

  /**
   * Convert a Pixel to a new color space
   * @param inputPixel input pixel (CalypPixel)
   * @param eOutputSpace output color space
   * @return converted pixel
   */
  CalypPixel convertPixel( CalypColorSpace eOutputSpace ) const;
};

std::ostream& operator<<( std::ostream& os, const CalypPixel& p );

/**
 * \class    CalypPlane
 * \ingroup	 CalypLibGrp CalypPlaneGrp
 * \brief    Frame handling class
 */
template <typename T>
class CalypPlane
{
public:
  CalypPlane() noexcept = default;
  CalypPlane( std::size_t width, std::size_t height ) noexcept { resize( width, height ); }

  ~CalypPlane() noexcept = default;
  CalypPlane( const CalypPlane& buffer ) noexcept = default;
  CalypPlane( CalypPlane&& buffer ) noexcept = default;
  auto operator=( const CalypPlane& buffer ) -> CalypPlane& = default;
  auto operator=( CalypPlane&& buffer ) noexcept -> CalypPlane& = default;

  auto operator[]( std::size_t row ) const& noexcept -> const std::span<T>& { return m_rows[row]; }
  auto operator[]( std::size_t row ) & noexcept -> std::span<T>& { return m_rows[row]; }

  auto operator[]( std::size_t row ) const&& noexcept { return m_rows[row]; }
  auto operator[]( std::size_t row ) && noexcept { return m_rows[row]; }

  auto data() noexcept { return std::span<T>( m_data ); }
  auto data() const noexcept { return std::span<const T>( m_data ); }

  void resize( std::size_t width, std::size_t height ) noexcept
  {
    m_data.resize( width * height );
    m_rows.resize( height );
    for( std::size_t y = 0; y < height; y++ )
    {
      m_rows[y] = std::span<T>( m_data ).subspan( y * width, width );
    }
  }

private:
  std::vector<T> m_data;
  std::vector<std::span<T>> m_rows;
};

/**
 * \class    CalypFrame
 * \ingroup	 CalypLibGrp CalypFrameGrp
 * \brief    Frame handling class
 */
class CalypFrame
{
public:
  /**
   * Function that handles the supported color space
   * of CalypFrame
   * @return vector of strings with pixel formats names
   */
  static std::vector<std::string> supportedColorSpacesListNames();

  /**
   * Function that handles the supported pixel formats
   * of CalypFrame
   * @return vector of strings with pixel formats names
   */
  static constexpr auto numberOfFormats() -> std::size_t;
  static auto findPixelFormat( const std::string& name ) -> std::optional<ClpPixelFormats>;
  static auto findPixelFormat( const std::string_view name ) -> std::optional<ClpPixelFormats>;
  static auto pelformatColorSpace( ClpPixelFormats idx ) -> int;
  static auto supportedPixelFormatListNames() -> std::map<ClpPixelFormats, std::string_view>;
  static auto supportedPixelFormatListNames( int colorSpace ) -> std::map<ClpPixelFormats, std::string_view>;
  static auto pixelFormatName( ClpPixelFormats idx ) -> const std::string_view;

  /**
   * Creates a new frame using the following configuration
   *
   * @param width width of the frame
   * @param height height of the frame
   * @param pel_format pixel format index (always use PixelFormats enum)
   *
   * @note this function might misbehave if the pixel format enum is not correct
   */
  CalypFrame( unsigned int width, unsigned int height, ClpPixelFormats pelFormat, unsigned bitsPixel = 8 );

  /**
   * Creates a new frame using the following configuration
   *
   * @param width width of the frame
   * @param height height of the frame
   * @param pel_format pixel format index (always use PixelFormats enum)
   *
   * @note this function might misbehave if the pixel format enum is not correct
   */
  CalypFrame( unsigned int width, unsigned int height, ClpPixelFormats pelFormat, unsigned bitsPixel, bool has_negative_values );

  /**
   * Move contructor
   *
   * @param other existing frame to copy from
   */
  CalypFrame( CalypFrame&& other ) noexcept;

  /**
   * Move assignement contructor
   *
   * @param other existing frame to copy from
   */
  CalypFrame& operator=( CalypFrame&& other ) noexcept;

  /**
   * Copy contructor
   *
   * @param other existing frame to copy from
   */
  CalypFrame( const CalypFrame& other );

  /**
   * Copy-assignement contructor
   *
   * @param other existing frame to copy from
   */
  CalypFrame& operator=( const CalypFrame& other );

  /**
   * Creates and new frame with the configuration of an existing one and copy
   * its contents. This function only copies a specific region from the existing
   * frame
   *
   * @param other existing frame to copy from
   * @param posX position X to crop from
   * @param posY position Y to crop from
   * @param areaWidth crop width
   * @param areaHeight crop height
   */
  CalypFrame( const CalypFrame& other, unsigned int x, unsigned int y, unsigned int width, unsigned int height );
  CalypFrame( const CalypFrame* other, unsigned int x, unsigned int y, unsigned int width, unsigned int height );

  ~CalypFrame();

  /** Format match opts
   */
  enum FormatMatching
  {
    MATCH_COLOR_SPACE = 1,
    MATCH_RESOLUTION = 2,
    MATCH_PEL_FMT = 4,
    MATCH_BITS = 8,
    MATCH_COLOR_SPACE_IGNORE_GRAY = 16,
    MATCH_BYTES_PER_FRAME = 32,
    MATCH_ALL = 0xFFFF,
  };

  /**
   * Check if two CalypFrames have the same fmt
   * @param other frame to compare with
   * @param match matching conditions (use enum FormatMatching)
   * @return true if format matches
   */
  bool haveSameFmt( const CalypFrame& other, unsigned int match = MATCH_ALL ) const;
  bool haveSameFmt( const CalypFrame* other, unsigned int match = MATCH_ALL ) const;

  /**
   * Get pixel format information
   * @return pixel format index
   */
  ClpPixelFormats getPelFormat() const;

  /**
   * Get pixel format information
   * @return pixel format name
   */
  auto getPelFmtName() const -> std::string;

  /**
   * Get color space information
   * @return get color space index
   */
  int getColorSpace() const;

  /**
   * Get the number of channels
   * @return number of channels
   */
  unsigned getNumberChannels() const;

  /**
   * Get width of the frame
   * @param channel/component
   * @return number of pixels
   */
  unsigned int getWidth( unsigned channel = 0 ) const;

  /**
   * Get height of the frame
   * @param channel/component
   * @return number of pixels
   */
  unsigned int getHeight( unsigned channel = 0 ) const;

  /**
   * Check if frame has negative values
   * @return Boolean with the flag value
   */
  bool getHasNegativeValues() const;

  /**
   * Get number of pixels of the frame
   * @param channel/component
   * @return number of pixels
   */
  std::uint64_t getPixels( unsigned channel = 0 ) const;

  /**
   * Get the total number of pixels of the frame
   * @return number of pixels
   */
  std::uint64_t getTotalNumberOfPixels() const;

  /**
   * Get chroma width ratio
   * @return ratio multiple of 2
   */
  unsigned getChromaWidthRatio() const;

  /**
   * Get chroma height ratio
   * @return ratio multiple of 2
   */
  unsigned getChromaHeightRatio() const;

  /**
   * Get number of pixels in each chroma channel
   * @return number of pixels
   */
  std::uint64_t getChromaLength() const;

  /**
   * Get number of bits per pixel
   * @return number of bits
   */
  unsigned int getBitsPel() const;

  /**
   * Get number of bytes per frame of an existing frame
   * @return number of bytes per frame
   */
  std::uint64_t getBytesPerFrame() const;

  /**
   * Get number of bytes per frame of a specific pixel format
   * @return number of bytes per frame
   */
  static std::uint64_t getBytesPerFrame( unsigned int uiWidth, unsigned int uiHeight, ClpPixelFormats pelFormat, unsigned int bitsPixel );

  /**
   * Reset frame pixels to zero
   */
  void reset();

  ClpPel*** getPelBufferYUV() const;
  ClpPel*** getPelBufferYUV();

  auto getRGBBuffer() const -> const unsigned char*;

  /**
   * Get pixel value at coordinates
   * @param ch frame channel
   * @param xPos position in X axis
   * @param yPos position in Y axis
   * @param absolute whether it should return positive/negative values
   * @return pixel value
   */
  ClpPel operator()( unsigned int ch, unsigned int xPos, unsigned int yPos, bool absolute = true ) const;

  /**
   * Get pixel value at coordinates
   * @param xPos position in X axis
   * @param yPos position in Y axis
   * @return pixel value
   */
  CalypPixel operator()( unsigned int xPos, unsigned int yPos ) const;

  /**
   * Get pixel value at coordinates
   * @param xPos position in X axis
   * @param yPos position in Y axis
   * @return pixel value
   */
  CalypPixel getPixel( unsigned int xPos, unsigned int yPos ) const;

  /**
   * Get pixel value at coordinates
   * in the desired color space
   * @param xPos position in X axis
   * @param yPos position in Y axis
   * @param eColorSpace desired color space
   * @return pixel value
   */
  CalypPixel getPixel( unsigned int xPos, unsigned int yPos, CalypColorSpace eColorSpace ) const;

  /**
   * Set pixel value at coordinates to a given value
   * @param xPos position in X axis
   * @param yPos position in Y axis
   */
  void setPixel( unsigned int xPos, unsigned int yPos, CalypPixel pixel );

  /**
   * Copy a frame into the current buffer
   * @param other frame to be copied
   */
  void copyFrom( const CalypFrame& other );
  void copyFrom( const CalypFrame* other );

  /**
   * Copy a region from a reference corresponding to the size
   * of the size of the current frame
   * @param other frame to be copied
   * @param x x position on the reference frame
   * @param y y position on the reference frame
   */
  void copyFrom( const CalypFrame& other, unsigned x, unsigned y );
  void copyFrom( const CalypFrame* other, unsigned x, unsigned y );

  /**
   * Copy a reference frame to a specific location on the current frame
   * The whole reference frame is copied
   * @param other frame to be copied
   * @param x x position on the current frame
   * @param y y position on the current frame
   */
  void copyTo( const CalypFrame& other, unsigned x, unsigned y ) const;
  void copyTo( const CalypFrame* other, unsigned x, unsigned y ) const;

  void frameFromBuffer( const std::vector<ClpByte>&, int, unsigned long );
  void frameFromBuffer( const std::vector<ClpByte>&, int );
  void frameToBuffer( std::vector<ClpByte>&, int ) const;

  void fillRGBBuffer() const;

  /**
   * Histogram
   */
  enum HistogramChannels
  {
    HIST_CHAN_ONE = 0,
    HIST_CHAN_TWO,
    HIST_CHAN_THREE,
    HIST_CHAN_FOUR,
    HIST_LUMA = 10,
    HIST_CHROMA_U,
    HIST_CHROMA_V,
    HIST_COLOR_R = 20,
    HIST_COLOR_G,
    HIST_COLOR_B,
    HIST_COLOR_A,
    HIST_ALL_CHANNELS = 254,
    HISTOGRAM_MAX = 255,
  };
  void calcHistogram();

  unsigned int getMinimumPelValue( unsigned channel ) const;
  unsigned int getMaximumPelValue( unsigned channel ) const;

  unsigned int getMaximum( unsigned channel ) const;
  unsigned int getNumPixelsRange( unsigned channel, unsigned int start, unsigned int end ) const;
  double getMean( unsigned channel, unsigned int start, unsigned int end ) const;
  int getMedian( unsigned channel, unsigned int start, unsigned int end ) const;
  double getStdDev( unsigned channel, unsigned int start, unsigned int end ) const;
  double getHistogramValue( unsigned channel, unsigned int bin ) const;
  unsigned int getNEBins( unsigned channel ) const;
  int getNumHistogramSegment() const;
  double getEntropy( unsigned channel, unsigned int start, unsigned int end ) const;

  /**
   * interface with OpenCV lib
   */
  bool toMat( cv::Mat& cvMat, bool convertToGray = false, bool scale = true, unsigned channel = 0 ) const;
  bool fromMat( cv::Mat& cvMat, int iChannel = -1 );

  /**
   * \ingroup	 CalypFrameGrp
   * @defgroup CalypFrameQualityMetricsGrp Calyp Frame Quality Metrics interface
   * @{
   * Quality metrics interface
   *
   */

  enum QualityMetrics
  {
    NO_METRIC = -1,
    PSNR_METRIC = 0,
    MSE_METRIC,
    SSIM_METRIC,
    WSPSNR_METRIC,
    NUMBER_METRICS,
  };

  static std::vector<std::string> supportedQualityMetricsList();
  static std::vector<std::string> supportedQualityMetricsUnitsList();
  double getQuality( int Metric, CalypFrame* Org, unsigned int component );
  double getMSE( CalypFrame* Org, unsigned int component );
  double getPSNR( CalypFrame* Org, unsigned int component );
  double getSSIM( CalypFrame* Org, unsigned int component );
  double getWSPNR( CalypFrame* Org, unsigned int component );
  /** @} */

private:
  class CalypFramePrivate;
  std::unique_ptr<CalypFramePrivate> d;
};

#endif  // __CALYPFRAME_H__

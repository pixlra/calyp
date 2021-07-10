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

#include <span>

#include "CalypDefs.h"

namespace cv
{
class Mat;
}

#define CHROMASHIFT( SIZE, SHIFT ) (unsigned int)( -( ( -( (int)( SIZE ) ) ) >> ( SHIFT ) ) )

/**
 * \class    CalypPixel
 * \ingroup  CalypLibGrp CalypFrameGrp
 * \brief    Pixel handling class
 */
class CalypPixel
{
public:
  static int getMaxNumberOfComponents();

  CalypPixel( const int& CalypColorSpace = CLP_COLOR_INVALID );
  CalypPixel( const int& CalypColorSpace, const ClpPel& c0 );
  CalypPixel( const int& CalypColorSpace, const ClpPel& c0, const ClpPel& c1, const ClpPel& c2 );
  CalypPixel( const int& CalypColorSpace, const ClpPel& c0, const ClpPel& c1, const ClpPel& c2, const ClpPel& c3 );
  CalypPixel( const CalypPixel& other );
  ~CalypPixel();

  int colorSpace() const;

  ClpPel operator[]( const int& channel ) const;
  ClpPel& operator[]( const int& channel );

  CalypPixel operator=( const CalypPixel& );
  CalypPixel operator+( const CalypPixel& );
  CalypPixel operator+=( const CalypPixel& );
  CalypPixel operator-( const CalypPixel& );
  CalypPixel operator-=( const CalypPixel& );
  CalypPixel operator*( const double& );

  /**
	 * Convert a Pixel to a new color space
	 * @param inputPixel input pixel (CalypPixel)
	 * @param eOutputSpace output color space
	 * @return converted pixel
	 */
  CalypPixel convertPixel( CalypColorSpace eOutputSpace );

private:
  class CalypPixelPrivate;
  std::unique_ptr<CalypPixelPrivate> d;
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
  static std::vector<ClpString> supportedColorSpacesListNames();

  /**
	 * Function that handles the supported pixel formats
	 * of CalypFrame
	 * @return vector of strings with pixel formats names
	 */
  static std::vector<ClpString> supportedPixelFormatListNames();
  static std::vector<ClpString> supportedPixelFormatListNames( int colorSpace );
  static int numberOfFormats();
  static int findPixelFormat( const ClpString& name );
  static int pelformatColorSpace( const int idx );

  /**
   * Default constructor. Frame is not valid
   */
  CalypFrame();

  /**
	 * Creates a new frame using the following configuration
	 *
	 * @param width width of the frame
	 * @param height height of the frame
	 * @param pel_format pixel format index (always use PixelFormats enum)
	 *
	 * @note this function might misbehave if the pixel format enum is not correct
	 */
  CalypFrame( unsigned int width, unsigned int height, int pelFormat, unsigned bitsPixel = 8 );

  /**
   * Creates a new frame using the following configuration
   *
   * @param width width of the frame
   * @param height height of the frame
   * @param pel_format pixel format index (always use PixelFormats enum)
   *
   * @note this function might misbehave if the pixel format enum is not correct
   */
  CalypFrame( unsigned int width, unsigned int height, int pelFormat, unsigned bitsPixel, bool has_negative_values );

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
  CalypFrame( const CalypFrame* other );

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
  int getPelFormat() const;

  /**
	 * Get pixel format information
	 * @return pixel format name
	 */
  ClpString getPelFmtName();

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
  ClpULong getPixels( unsigned channel = 0 ) const;

  /**
   * Get the total number of pixels of the frame
   * @return number of pixels
   */
  ClpULong getTotalNumberOfPixels() const;

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
  ClpULong getChromaLength() const;

  /**
	 * Get number of bits per pixel
	 * @return number of bits
	 */
  unsigned int getBitsPel() const;

  /**
	 * Get number of bytes per frame of an existing frame
	 * @return number of bytes per frame
	 */
  ClpULong getBytesPerFrame() const;

  /**
	 * Get number of bytes per frame of a specific pixel format
	 * @return number of bytes per frame
	 */
  static ClpULong getBytesPerFrame( unsigned int uiWidth, unsigned int uiHeight, int iPixelFormat, unsigned int bitsPixel );

  /**
	 * Reset frame pixels to zero
	 */
  void reset();

  ClpPel*** getPelBufferYUV() const;
  ClpPel*** getPelBufferYUV();

  unsigned char* getRGBBuffer() const;

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
  CalypPixel getPixel( unsigned int xPos, unsigned int yPos, CalypColorSpace eColorSpace );

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
  void copyTo( const CalypFrame& other, unsigned x, unsigned y );
  void copyTo( const CalypFrame* other, unsigned x, unsigned y );

  void frameFromBuffer( const std::vector<ClpByte>&, int, unsigned long );
  void frameFromBuffer( const std::vector<ClpByte>&, int );
  void frameToBuffer( std::vector<ClpByte>&, int );

  void fillRGBBuffer();

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

  unsigned int getMinimumPelValue( unsigned channel );
  unsigned int getMaximumPelValue( unsigned channel );

  unsigned int getMaximum( unsigned channel );
  unsigned int getNumPixelsRange( unsigned channel, unsigned int start, unsigned int end );
  double getMean( unsigned channel, unsigned int start, unsigned int end );
  int getMedian( unsigned channel, unsigned int start, unsigned int end );
  double getStdDev( unsigned channel, unsigned int start, unsigned int end );
  double getHistogramValue( unsigned channel, unsigned int bin );
  unsigned int getNEBins( unsigned channel );
  int getNumHistogramSegment();
  double getEntropy( unsigned channel, unsigned int start, unsigned int end );

  /**
	 * interface with OpenCV lib
	 */
  bool toMat( cv::Mat& cvMat, bool convertToGray = false, bool scale = true, unsigned channel = 0 );
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

  static std::vector<ClpString> supportedQualityMetricsList();
  static std::vector<ClpString> supportedQualityMetricsUnitsList();
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

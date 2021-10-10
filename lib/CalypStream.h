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
 * \file     CalypStream.h
 * \ingroup  CalypStreamGrp
 * \brief    Input stream handling
 */

#ifndef __CALYPSTREAM_H__
#define __CALYPSTREAM_H__

#include <memory>
#include <vector>

#include "CalypFrame.h"

class CalypStreamHandlerIf;

struct CalypStandardResolution
{
  std::string shortName;
  unsigned int uiWidth;
  unsigned int uiHeight;
};

/**
 * \class CalypStreamFormat
 * \ingroup CalypLibGrp CalypStreamGrp
 * \brief  Stream format handling class
 */

class CalypStreamFormat
{
public:
  using CreateStreamHandlerFn = auto ( * )( void ) -> std::unique_ptr<CalypStreamHandlerIf>;

  CalypStreamFormat() = default;
  std::vector<std::string> getExts();

  std::string formatName;
  std::string formatExt;
  std::string formatPattern;
  CreateStreamHandlerFn formatFct;
};

/**
 * \class CalypStream
 * \ingroup CalypLibGrp CalypStreamGrp
 * \brief  Stream handling class
 */
class CalypStream
{
public:
  static std::vector<CalypStreamFormat> supportedReadFormats();
  static std::vector<CalypStreamFormat> supportedWriteFormats();
  static std::vector<CalypStandardResolution> stdResolutionSizes();

  CalypStream();
  CalypStream( CalypStream&& other ) noexcept = delete;
  CalypStream& operator=( CalypStream&& other ) noexcept = delete;
  CalypStream( const CalypStream& other ) = delete;
  CalypStream& operator=( const CalypStream& other ) = delete;
  ~CalypStream();

  std::string getFormatName() const;
  std::string getCodecName() const;

  bool open( std::string filename,
             std::string resolution,
             std::string input_format,
             unsigned int bitsPel,
             int endianness,
             bool hasNegative,
             unsigned int frame_rate,
             bool bInput );
  bool open( std::string filename,
             unsigned int width,
             unsigned int height,
             int input_format,
             unsigned int bitsPel,
             int endianness,
             unsigned int frame_rate,
             bool bInput );
  bool open( std::string filename,
             unsigned int width,
             unsigned int height,
             int input_format,
             unsigned int bitsPel,
             int endianness,
             bool hasNegative,
             unsigned int frame_rate,
             bool bInput );
  bool open( std::string filename,
             unsigned int width,
             unsigned int height,
             int input_format,
             unsigned int bitsPel,
             int endianness,
             unsigned int frame_rate,
             bool bInput,
             bool forceRaw );

  bool supportsFormatConfiguration();
  bool reload();

  bool isNative() const;
  std::string getFileName() const;
  std::uint64_t getFrameNum() const;
  unsigned int getWidth() const;
  unsigned int getHeight() const;
  unsigned int getBitsPerPixel() const;
  int getEndianess() const;
  long getCurrFrameNum() const;
  double getFrameRate() const;
  void getFormat( unsigned int& rWidth, unsigned int& rHeight, int& rInputFormat, unsigned int& rBitsPerPel, int& rEndianness,
                  unsigned int& rFrameRate ) const;

  auto hasNextFrame() -> bool;
  auto hasWritingSlot() -> bool;

  void loadAll();
  std::unique_ptr<CalypFrame> getCurrFrame( std::unique_ptr<CalypFrame> buffer );
  auto getCurrFrameAsset() -> std::shared_ptr<CalypFrame>;
  auto getCurrFrame() -> CalypFrame*;
  bool isEof();
  bool setNextFrame();
  void readNextFrame();
  void readNextFrameFillRGBBuffer();

  void writeFrame( const CalypFrame& pcFrame );

  bool saveFrame( const std::string& filename );
  static bool saveFrame( const std::string& filename, const CalypFrame& saveFrame );

  bool seekInputRelative( bool bIsFoward );
  bool seekInput( unsigned long new_frame_num );

private:
  class CalypStreamPrivate;
  std::unique_ptr<CalypStreamPrivate> d;
};

#endif  // __CALYPSTREAM_H__

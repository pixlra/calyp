/*    This file is a part of plaYUVer project
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
 * \file     CalypStreamTests.cpp
 * \brief    CalypStream general tests
 */

#include <catch2/catch_all.hpp>
#include <filesystem>
#include <iostream>
#include <variant>

#include "CalypFrame.h"
#include "CalypStream.h"

constexpr int kFrameRate{ 30 };
constexpr auto kStreamType = CalypStream::Type::Input;

auto getFileTestFilename( const char* test_sequence ) -> std::string
{
  return std::string( CALYP_TEST_DATA_DIR ) + std::string( "/" ) + std::string( test_sequence );
}

TEST_CASE( "Can open a YUV 420p file", "CalypStream" )
{
  constexpr int kWidth{ 352 };
  constexpr int kHeight{ 288 };
  constexpr auto kInputFormat{ ClpPixelFormats::YUV420p };
  constexpr int kBitsPel{ 8 };
  constexpr auto KEndianness{ CLP_INVALID_ENDIANESS };

  const auto kFilename = getFileTestFilename( "Foreman.yuv" );

  REQUIRE( std::filesystem::exists( std::filesystem::path( kFilename ) ) );

  CalypStream test_stream;
  auto result =
      test_stream.open( kFilename, kWidth, kHeight, kInputFormat, kBitsPel, KEndianness, kFrameRate, kStreamType );

  REQUIRE( result );

  CHECK( test_stream.getWidth() == kWidth );
  CHECK( test_stream.getHeight() == kHeight );
  CHECK( test_stream.getBitsPerPixel() == kBitsPel );
  CHECK( test_stream.getFormatName() == "YUV" );
  CHECK( test_stream.getCodecName() == "Raw Video" );
  CHECK( test_stream.getEndianess() == CLP_BIG_ENDIAN );

  const auto* frame = test_stream.getCurrFrame();

  CHECK( frame->getPixel( 2, 0 ) == CalypPixel{ CLP_COLOR_YUV, 201, 129, 125 } );
  CHECK( frame->getPixel( 336, 278 ) == CalypPixel{ CLP_COLOR_YUV, 99, 111, 142 } );
}

TEST_CASE( "Can open a YUV file", "CalypStream" )
{
  constexpr int kWidth{ 352 };
  constexpr int kHeight{ 288 };
  constexpr int kBitsPel{ 8 };
  constexpr auto KEndianness{ CLP_INVALID_ENDIANESS };

  auto common_checks = []( auto& stream ) {
    return stream.getWidth() == kWidth && stream.getHeight() == kHeight && stream.getBitsPerPixel() == kBitsPel &&
           stream.getFormatName() == "YUV" && stream.getCodecName() == "Raw Video" &&
           stream.getEndianess() == CLP_BIG_ENDIAN;
  };
  SECTION( "YUV420p" )
  {
    constexpr auto kInputFormat{ ClpPixelFormats::YUV420p };
    const auto kFilename = getFileTestFilename( "Foreman.yuv" );
    REQUIRE( std::filesystem::exists( std::filesystem::path( kFilename ) ) );

    CalypStream test_stream;
    auto result =
        test_stream.open( kFilename, kWidth, kHeight, kInputFormat, kBitsPel, KEndianness, kFrameRate, kStreamType );
    REQUIRE( result );
    CHECK( common_checks( test_stream ) );
    const auto* frame = test_stream.getCurrFrame();

    CHECK( frame->getColorSpace() == CLP_COLOR_YUV );
    CHECK( frame->getPelFmtName() == "YUV420p" );
    CHECK( frame->getPelFormat() == kInputFormat );
  }
  SECTION( "YUV422p" )
  {
    constexpr auto kInputFormat{ ClpPixelFormats::YUV422p };
    const auto kFilename = getFileTestFilename( "Foreman_352x288_yuv422p.yuv" );
    REQUIRE( std::filesystem::exists( std::filesystem::path( kFilename ) ) );

    CalypStream test_stream;
    auto result =
        test_stream.open( kFilename, kWidth, kHeight, kInputFormat, kBitsPel, KEndianness, kFrameRate, kStreamType );
    REQUIRE( result );
    CHECK( common_checks( test_stream ) );
    const auto* frame = test_stream.getCurrFrame();

    CHECK( frame->getColorSpace() == CLP_COLOR_YUV );
    CHECK( frame->getPelFmtName() == "YUV422p" );
    CHECK( frame->getPelFormat() == kInputFormat );
  }
  SECTION( "YUV444p" )
  {
    constexpr auto kInputFormat{ ClpPixelFormats::YUV444p };
    const auto kFilename = getFileTestFilename( "Foreman_352x288_yuv444p.yuv" );
    REQUIRE( std::filesystem::exists( std::filesystem::path( kFilename ) ) );

    CalypStream test_stream;
    auto result =
        test_stream.open( kFilename, kWidth, kHeight, kInputFormat, kBitsPel, KEndianness, kFrameRate, kStreamType );
    REQUIRE( result );
    CHECK( common_checks( test_stream ) );
    const auto* frame = test_stream.getCurrFrame();

    CHECK( frame->getColorSpace() == CLP_COLOR_YUV );
    CHECK( frame->getPelFmtName() == "YUV444p" );
    CHECK( frame->getPelFormat() == kInputFormat );
  }
  SECTION( "YUYV422" )
  {
    constexpr auto kInputFormat{ ClpPixelFormats::YUYV422 };
    const auto kFilename = getFileTestFilename( "Foreman_352x288_yuyv422.yuv" );
    REQUIRE( std::filesystem::exists( std::filesystem::path( kFilename ) ) );

    CalypStream test_stream;
    auto result =
        test_stream.open( kFilename, kWidth, kHeight, kInputFormat, kBitsPel, KEndianness, kFrameRate, kStreamType );
    REQUIRE( result );
    CHECK( common_checks( test_stream ) );
    const auto* frame = test_stream.getCurrFrame();

    CHECK( frame->getColorSpace() == CLP_COLOR_YUV );
    CHECK( frame->getPelFmtName() == "YUYV422" );
    CHECK( frame->getPelFormat() == kInputFormat );
  }
}

TEST_CASE( "Can open a RGB file", "CalypStream" )
{
  constexpr int kWidth{ 352 };
  constexpr int kHeight{ 288 };
  constexpr int kBitsPel{ 8 };
  constexpr auto KEndianness{ CLP_INVALID_ENDIANESS };

  auto common_checks = []( auto& stream ) {
    return stream.getWidth() == kWidth && stream.getHeight() == kHeight && stream.getBitsPerPixel() == kBitsPel &&
           stream.getFormatName() == "YUV" && stream.getCodecName() == "Raw Video" &&
           stream.getEndianess() == CLP_BIG_ENDIAN;
  };
  SECTION( "RGB24" )
  {
    constexpr auto kInputFormat{ ClpPixelFormats::RGB24 };
    const auto kFilename = getFileTestFilename( "Foreman_352x288_rgb24.yuv" );
    REQUIRE( std::filesystem::exists( std::filesystem::path( kFilename ) ) );
    CalypStream test_stream;
    auto result =
        test_stream.open( kFilename, kWidth, kHeight, kInputFormat, kBitsPel, KEndianness, kFrameRate, kStreamType );
    REQUIRE( result );
    CHECK( common_checks( test_stream ) );
    const auto* frame = test_stream.getCurrFrame();
    CHECK( frame->getColorSpace() == CLP_COLOR_RGB );
    CHECK( frame->getPelFmtName() == "RGB" );
    CHECK( frame->getPelFormat() == kInputFormat );

    std::cout << test_stream.getFormatName();
  }

  SECTION( "BGR24" )
  {
    constexpr auto kInputFormat{ ClpPixelFormats::BGR24 };
    const auto kFilename = getFileTestFilename( "Foreman_352x288_bgr24.yuv" );
    REQUIRE( std::filesystem::exists( std::filesystem::path( kFilename ) ) );
    CalypStream test_stream;
    auto result =
        test_stream.open( kFilename, kWidth, kHeight, kInputFormat, kBitsPel, KEndianness, kFrameRate, kStreamType );
    REQUIRE( result );
    CHECK( common_checks( test_stream ) );
    const auto* frame = test_stream.getCurrFrame();
    CHECK( frame->getColorSpace() == CLP_COLOR_RGB );
    CHECK( frame->getPelFmtName() == "BGR" );
    CHECK( frame->getPelFormat() == kInputFormat );
  }
}

TEST_CASE( "Can open a GRAY file", "CalypStream" )
{
  constexpr int kWidth{ 352 };
  constexpr int kHeight{ 288 };
  constexpr int kBitsPel{ 8 };
  constexpr auto KEndianness{ CLP_INVALID_ENDIANESS };

  auto common_checks = []( auto& stream ) {
    return stream.getWidth() == kWidth && stream.getHeight() == kHeight && stream.getBitsPerPixel() == kBitsPel &&
           stream.getFormatName() == "YUV" && stream.getCodecName() == "Raw Video" &&
           stream.getEndianess() == CLP_BIG_ENDIAN;
  };
  constexpr auto kInputFormat{ ClpPixelFormats::Gray };
  const auto kFilename = getFileTestFilename( "Foreman_352x288_gray.yuv" );
  REQUIRE( std::filesystem::exists( std::filesystem::path( kFilename ) ) );
  CalypStream test_stream;
  auto result =
      test_stream.open( kFilename, kWidth, kHeight, kInputFormat, kBitsPel, KEndianness, kFrameRate, kStreamType );
  REQUIRE( result );
  CHECK( common_checks( test_stream ) );
  const auto* frame = test_stream.getCurrFrame();
  CHECK( frame->getColorSpace() == CLP_COLOR_GRAY );
  CHECK( frame->getPelFmtName() == "GRAY" );
  CHECK( frame->getPelFormat() == kInputFormat );
}

TEST_CASE( "Can open .batatas file as RAW video", "CalypStream" )
{
  constexpr int kWidth{ 352 };
  constexpr int kHeight{ 288 };
  constexpr auto kInputFormat{ ClpPixelFormats::YUV420p };
  constexpr int kBitsPel{ 8 };
  constexpr auto KEndianness{ CLP_INVALID_ENDIANESS };

  const auto kFilename = getFileTestFilename( "Foreman.batatas" );

  REQUIRE( std::filesystem::exists( std::filesystem::path( kFilename ) ) );

  bool force_raw{ false };

  SECTION( "Without forcing raw video" )
  {
    CalypStream test_stream;
    bool result{ false };
    try
    {
      result = test_stream.open( kFilename, kWidth, kHeight, kInputFormat, kBitsPel, KEndianness, kFrameRate, force_raw,
                                 kStreamType );
    }
    catch( ... )
    {
      // NOLINT
    }

    CHECK( !result );
  }

  SECTION( "Forcing raw video" )
  {
    force_raw = true;

    CalypStream test_stream;
    auto result = test_stream.open( kFilename, kWidth, kHeight, kInputFormat, kBitsPel, KEndianness, kFrameRate,
                                    force_raw, kStreamType );

    REQUIRE( result );

    CHECK( test_stream.getWidth() == kWidth );
    CHECK( test_stream.getHeight() == kHeight );
    CHECK( test_stream.getBitsPerPixel() == kBitsPel );
    CHECK( test_stream.getFormatName() == "BATATAS" );
    CHECK( test_stream.getCodecName() == "Raw Video" );
    CHECK( test_stream.getEndianess() == CLP_BIG_ENDIAN );

    const auto* frame = test_stream.getCurrFrame();

    CHECK( frame->getPixel( 2, 0 ) == CalypPixel{ CLP_COLOR_YUV, 201, 129, 125 } );
    CHECK( frame->getPixel( 336, 278 ) == CalypPixel{ CLP_COLOR_YUV, 99, 111, 142 } );
  }
}
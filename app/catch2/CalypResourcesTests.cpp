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
 * \file     CalypResourcesTests.cpp
 * \brief    Resouce handling tests
 */

#include <QString>
#include <catch2/catch.hpp>
#include <filesystem>
#include <iostream>
#include <variant>

#include "ResourceHandle.h"
#include "lib/CalypFrame.h"
#include "lib/CalypStream.h"
#include "models/VideostreamResource.h"

using namespace std::chrono_literals;

constexpr int kFrameRate{ 30 };
constexpr auto kStreamType = CalypStream::Type::Input;

auto getFileTestFilename( const char* test_sequence ) -> std::string
{
  return std::string( CALYP_TEST_DATA_DIR ) + std::string( "/" ) + std::string( test_sequence );
}

TEST_CASE( "Can construct and destruct a resource", "VideoResource" )
{
  auto videoResource = std::make_unique<VideostreamResource>();
  CHECK( videoResource != nullptr );
}

TEST_CASE( "Can load a YUV file", "VideoResource" )
{
  constexpr unsigned int kWidth{ 352 };
  constexpr unsigned int kHeight{ 288 };
  constexpr auto kInputFormat{ ClpPixelFormats::YUV420p };
  constexpr unsigned int kBitsPel{ 8 };
  constexpr auto KEndianness{ CLP_LITTLE_ENDIAN };

  const auto kFilename = getFileTestFilename( "Foreman.yuv" );

  REQUIRE( std::filesystem::exists( std::filesystem::path( kFilename ) ) );

  auto resourceHandle = std::make_unique<ResourceHandle>( nullptr );

  auto videoResource = std::make_unique<VideostreamResource>();

  CalypFileInfo streamInfo{ .m_cFilename = QString::fromStdString( kFilename ) };
  CHECK_FALSE( videoResource->loadFile( streamInfo, false ) );

  SECTION( "Now use a valid config" )
  {
    streamInfo = CalypFileInfo{ .m_cFilename = QString::fromStdString( kFilename ),
                                .m_uiWidth = kWidth,
                                .m_uiHeight = kHeight,
                                .m_iPelFormat = kInputFormat,
                                .m_uiBitsPelPixel = kBitsPel,
                                .m_iEndianness = KEndianness,
                                .m_uiFrameRate = kFrameRate };

    REQUIRE( videoResource->loadFile( streamInfo ) );

    const auto& test_stream = videoResource->getStream();

    CHECK( test_stream.getWidth() == kWidth );
    CHECK( test_stream.getHeight() == kHeight );
    CHECK( test_stream.getBitsPerPixel() == kBitsPel );
    CHECK( test_stream.getFormatName() == "YUV" );
    CHECK( test_stream.getCodecName() == "Raw Video" );
    CHECK( test_stream.getEndianess() == CLP_BIG_ENDIAN );

    auto frame = videoResource->getCurrFrame();

    CHECK( frame->getPixel( 2, 0 ) == CalypPixel{ CLP_COLOR_YUV, 201, 129, 125 } );
    CHECK( frame->getPixel( 336, 278 ) == CalypPixel{ CLP_COLOR_YUV, 99, 111, 142 } );
  }
}

TEST_CASE( "Can load a MKV file", "VideoResource" )
{
  constexpr unsigned int kWidth{ 352 };
  constexpr unsigned int kHeight{ 288 };
  constexpr unsigned int kBitsPel{ 8 };

  const auto kFilename = getFileTestFilename( "Foreman.mkv" );

  REQUIRE( std::filesystem::exists( std::filesystem::path( kFilename ) ) );

  auto resourceHandle = std::make_unique<ResourceHandle>( nullptr );

  auto videoResource = std::make_unique<VideostreamResource>( nullptr );

  CalypFileInfo streamInfo{ .m_cFilename = QString::fromStdString( kFilename ) };
  REQUIRE( videoResource->loadFile( streamInfo ) );

  const auto& test_stream = videoResource->getStream();

  CHECK( test_stream.getWidth() == kWidth );
  CHECK( test_stream.getHeight() == kHeight );
  CHECK( test_stream.getBitsPerPixel() == kBitsPel );
  CHECK( test_stream.getFormatName() == "MKV" );
  CHECK( test_stream.getCodecName() == "h264" );
  CHECK( test_stream.getEndianess() == CLP_BIG_ENDIAN );

  auto frame = videoResource->getCurrFrame();

  CHECK( frame->getPixel( 311, 255 ) == CalypPixel{ CLP_COLOR_YUV, 83, 120, 132 } );
}

TEST_CASE( "Can load a PNG file", "VideoResource" )
{
  constexpr unsigned int kWidth{ 352 };
  constexpr unsigned int kHeight{ 288 };
  constexpr unsigned int kBitsPel{ 8 };

  const auto kFilename = getFileTestFilename( "Foreman.png" );

  REQUIRE( std::filesystem::exists( std::filesystem::path( kFilename ) ) );

  auto resourceHandle = std::make_unique<ResourceHandle>( nullptr );

  auto videoResource = std::make_unique<VideostreamResource>( nullptr );

  CalypFileInfo streamInfo{ .m_cFilename = QString::fromStdString( kFilename ) };

  REQUIRE( videoResource->loadFile( streamInfo ) );

  const auto& test_stream = videoResource->getStream();

  CHECK( test_stream.getWidth() == kWidth );
  CHECK( test_stream.getHeight() == kHeight );
  CHECK( test_stream.getBitsPerPixel() == kBitsPel );
  CHECK( test_stream.getFormatName() == "PNG" );
  CHECK( test_stream.getCodecName() == "png" );
  CHECK( test_stream.getEndianess() == CLP_BIG_ENDIAN );

  auto frame = videoResource->getCurrFrame();

  CHECK( frame->getPixel( 292, 263 ) == CalypPixel{ CLP_COLOR_RGB, 92, 83, 54 } );
}

TEST_CASE( "Resource handle can store a video resource", "VideoResource" )
{
  constexpr unsigned int kWidth{ 352 };
  constexpr unsigned int kHeight{ 288 };
  constexpr auto kInputFormat{ ClpPixelFormats::YUV420p };
  constexpr unsigned int kBitsPel{ 8 };
  constexpr auto KEndianness{ CLP_LITTLE_ENDIAN };

  const auto kFilename = getFileTestFilename( "Foreman.yuv" );

  REQUIRE( std::filesystem::exists( std::filesystem::path( kFilename ) ) );

  auto resourceHandle = std::make_unique<ResourceHandle>( nullptr );

  CalypFileInfo streamInfo{ .m_cFilename = QString::fromStdString( kFilename ),
                            .m_uiWidth = kWidth,
                            .m_uiHeight = kHeight,
                            .m_iPelFormat = kInputFormat,
                            .m_uiBitsPelPixel = kBitsPel,
                            .m_iEndianness = KEndianness,
                            .m_uiFrameRate = kFrameRate };

  {
    auto videoResource = std::make_unique<VideostreamResource>( nullptr );
    REQUIRE( videoResource != nullptr );
    CHECK( videoResource->loadFile( streamInfo ) );
    CHECK( resourceHandle->appendResource( std::move( videoResource ) ) == 0 );
  }

  auto videoResource = std::make_unique<VideostreamResource>( nullptr );
  REQUIRE( videoResource != nullptr );
  CHECK( videoResource->loadFile( streamInfo ) );
  auto resource_id = resourceHandle->appendResource( std::move( videoResource ) );
  CHECK( resource_id == 1 );

  SECTION( "Can start and stop resource" )
  {
    resourceHandle->startResourceWorker( resource_id );
    std::this_thread::sleep_for( 20ms );
    resourceHandle->stopResourceWorker( resource_id );
    CHECK( true );
  }

  SECTION( "Can start without stopping (RAII will clear resource)" )
  {
    resourceHandle->startResourceWorker( resource_id );
    CHECK( true );
  }

  resourceHandle->removeResource( resource_id );
}

TEST_CASE( "Resource handle can play a video resource", "VideoResource" )
{
  constexpr unsigned int kWidth{ 352 };
  constexpr unsigned int kHeight{ 288 };
  constexpr auto kInputFormat{ ClpPixelFormats::YUV420p };
  constexpr unsigned int kBitsPel{ 8 };
  constexpr auto KEndianness{ CLP_LITTLE_ENDIAN };

  const auto kFilename = getFileTestFilename( "Foreman.yuv" );

  REQUIRE( std::filesystem::exists( std::filesystem::path( kFilename ) ) );

  auto resourceHandle = std::make_unique<ResourceHandle>( nullptr );

  CalypFileInfo streamInfo{ .m_cFilename = QString::fromStdString( kFilename ),
                            .m_uiWidth = kWidth,
                            .m_uiHeight = kHeight,
                            .m_iPelFormat = kInputFormat,
                            .m_uiBitsPelPixel = kBitsPel,
                            .m_iEndianness = KEndianness,
                            .m_uiFrameRate = kFrameRate };

  auto videoResource = std::make_unique<VideostreamResource>( nullptr );
  REQUIRE( videoResource != nullptr );
  CHECK( videoResource->loadFile( streamInfo ) );

  auto frame = videoResource->getCurrFrame();
  CHECK( frame->getPixel( 2, 0 ) == CalypPixel{ CLP_COLOR_YUV, 201, 129, 125 } );
  CHECK( frame->getPixel( 336, 278 ) == CalypPixel{ CLP_COLOR_YUV, 99, 111, 142 } );

  auto resource_id = resourceHandle->appendResource( std::move( videoResource ) );

  resourceHandle->executeResourceAction<VideostreamResource>( resource_id, []( auto* resource ) {
    resource->play();
    return true;
  } );

  resourceHandle->startResourceWorker( resource_id );

  resourceHandle->executeResourceAction<VideostreamResource>( resource_id, [&frame]( auto* resource ) {
    resource->playEvent();
    frame = resource->getCurrFrame();
    return true;
  } );
  CHECK( frame->getPixel( 298, 237 ) == CalypPixel{ CLP_COLOR_YUV, 82, 115, 134 } );

  resourceHandle->executeResourceAction<VideostreamResource>( resource_id, [&frame]( auto* resource ) {
    resource->playEvent();
    frame = resource->getCurrFrame();
    return true;
  } );
  CHECK_FALSE( frame->getPixel( 298, 237 ) == CalypPixel{ CLP_COLOR_YUV, 82, 115, 134 } );

  SECTION( "Can Stop" )
  {
    resourceHandle->executeResourceAction<VideostreamResource>( resource_id, [&frame]( auto* resource ) {
      resource->playEvent();
      frame = resource->getCurrFrame();
      return true;
    } );

    resourceHandle->executeResourceAction<VideostreamResource>( resource_id, [&frame]( auto* resource ) {
      resource->stop();
      frame = resource->getCurrFrame();
      return true;
    } );
    CHECK( frame->getPixel( 2, 0 ) == CalypPixel{ CLP_COLOR_YUV, 201, 129, 125 } );
    CHECK( frame->getPixel( 336, 278 ) == CalypPixel{ CLP_COLOR_YUV, 99, 111, 142 } );
  }

  resourceHandle->stopResourceWorker( resource_id );
}
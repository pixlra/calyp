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
 * \file     CalypFrameQualityTests.cpp
 * \brief    CalypFrame quality metrics tests
 */

#include "CalypFrame.h"
#include "CalypStream.h"
#include "gtest/gtest.h"

class CalypFrameQualityTest : public ::testing::Test
{
protected:
  // Per-test-case set-up.
  // Called before the first test in this test case.
  // Can be omitted if not needed.
  static void SetUpTestCase()
  {
    pcStreamPast = std::make_unique<CalypStream>();
    pcStreamPast->open( std::string( CALYP_TEST_DATA_DIR ) + std::string( "/BasketballDrill_F10_832x480_yuv420p.yuv" ), 832, 480, ClpPixelFormats::YUV420p, 8,
                        CLP_BIG_ENDIAN, 1, CalypStream::Type::Input );
    pcFramePast = pcStreamPast->getCurrFrame();

    pcStreamFuture = std::make_unique<CalypStream>();
    pcStreamFuture->open( std::string( CALYP_TEST_DATA_DIR ) + std::string( "/BasketballDrill_F15_832x480_yuv420p.yuv" ), 832, 480, ClpPixelFormats::YUV420p, 8,
                          CLP_BIG_ENDIAN, 1, CalypStream::Type::Input );
    pcFrameFuture = pcStreamFuture->getCurrFrame();
  }

  // Per-test-case tear-down.
  // Called after the last test in this test case.
  // Can be omitted if not needed.
  static void TearDownTestCase()
  {
    pcStreamPast = nullptr;
    pcStreamFuture = nullptr;
  }

  // Some expensive resource shared by all tests.
  static std::unique_ptr<CalypStream> pcStreamPast;
  static CalypFrame* pcFramePast;

  static std::unique_ptr<CalypStream> pcStreamFuture;
  static CalypFrame* pcFrameFuture;
};

std::unique_ptr<CalypStream> CalypFrameQualityTest::pcStreamPast{ nullptr };
std::unique_ptr<CalypStream> CalypFrameQualityTest::pcStreamFuture{ nullptr };
CalypFrame* CalypFrameQualityTest::pcFramePast{ nullptr };
CalypFrame* CalypFrameQualityTest::pcFrameFuture{ nullptr };

TEST_F( CalypFrameQualityTest, MSE )
{
  for( int c = 0; c < 3; c++ )
    EXPECT_EQ( pcFramePast->getQuality( CalypFrame::MSE_METRIC, pcFramePast, c ), 0 );

  EXPECT_NE( pcFrameFuture->getQuality( CalypFrame::MSE_METRIC, pcFramePast, 0 ), 0 );
}

TEST_F( CalypFrameQualityTest, PSNR )
{
  for( int c = 0; c < 3; c++ )
    EXPECT_EQ( pcFramePast->getQuality( CalypFrame::PSNR_METRIC, pcFramePast, c ), 100 );
}

TEST_F( CalypFrameQualityTest, SSIM )
{
  for( int c = 0; c < 3; c++ )
    EXPECT_EQ( pcFramePast->getQuality( CalypFrame::SSIM_METRIC, pcFramePast, c ), 1.0 );
}

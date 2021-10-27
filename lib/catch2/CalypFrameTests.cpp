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
 * \file     CalypFrameTests.cpp
 * \brief    CalypFrame general tests
 */

#include <catch2/catch.hpp>

#include "CalypFrame.h"

TEST_CASE( "create a 256x128 frame with 8 bits in YUV420 format", "CalypFrame" )
{
  // Constructor: CalypFrame( unsigned int width, unsigned int height, int pelFormat, unsigned bitsPixel = 8 );
  CalypFrame testFrame( 256, 128, ClpPixelFormats::CLP_YUV420P, 8 );

  CHECK( testFrame.getWidth() == 256 );
  CHECK( testFrame.getHeight() == 128 );
  CHECK( testFrame.getPelFormat() == ClpPixelFormats::CLP_YUV420P );
  CHECK( testFrame.getBitsPel() == 8 );
}

TEST_CASE( "create a 256x128 frame with 10 bits in YUV420 format", "CalypFrame" )
{
  // Constructor: CalypFrame( unsigned int width, unsigned int height, int pelFormat, unsigned bitsPixel = 8 );
  CalypFrame testFrame( 256, 128, ClpPixelFormats::CLP_YUV420P, 10 );

  CHECK( testFrame.getWidth() == 256 );
  CHECK( testFrame.getHeight() == 128 );
  CHECK( testFrame.getPelFormat() == ClpPixelFormats::CLP_YUV420P );
  CHECK( testFrame.getBitsPel() == 10 );
}

TEST_CASE( "create a 256x128 frame with 16 bits in YUV420 format", "CalypFrame" )
{
  // Constructor: CalypFrame( unsigned int width, unsigned int height, int pelFormat, unsigned bitsPixel = 8 );
  CalypFrame testFrame( 256, 128, ClpPixelFormats::CLP_YUV420P, 16 );

  CHECK( testFrame.getWidth() == 256 );
  CHECK( testFrame.getHeight() == 128 );
  CHECK( testFrame.getPelFormat() == ClpPixelFormats::CLP_YUV420P );
  CHECK( testFrame.getBitsPel() == 16 );
}

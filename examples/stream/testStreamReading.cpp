/*    This file is a part of Calyp project
 *    Copyright (C) 2014-2021  by Joao Carreira   (jfmcarreira@gmail.com)
 *
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
 * \file     testStreamReading.cpp
 * \brief    Example of open and reading from a stream
 */

#include <chrono>
#include <iostream>

#include "CalypFrame.h"
#include "CalypStream.h"

int main( int argc, char* argv[] )
{
  // Use auto keyword to avoid typing long
  // type definitions to get the timepoint
  // at this instant use function now()
  auto start = std::chrono::high_resolution_clock::now();

  double n = 0;
  CalypStream stream;
  //stream.open( ClpString( CALYP_TEST_DATA_DIR ) + ClpString( "/Kendo_c4_1024x768_yuv420p.yuv" ), 1024, 768, 0, 8, CLP_BIG_ENDIAN, 1, true );
  stream.open( ClpString( CALYP_TEST_DATA_DIR ) + ClpString( "/SteamLocomotiveTrain_2560x1600_60_10bit_crop.yuv" ), 2560, 1600, 0, 10, CLP_LITTLE_ENDIAN, 1, true );

  while( 1 )
  {
    CalypFrame* frame = stream.getCurrFrame();
    frame->fillRGBBuffer();
    n++;
    if( stream.setNextFrame() )
      break;
    stream.readNextFrame();
  }

  // After function call
  auto stop = std::chrono::high_resolution_clock::now();

  // Subtract stop and start timepoints and
  // cast it to required unit. Predefined units
  // are nanoseconds, microseconds, milliseconds,
  // seconds, minutes, hours. Use duration_cast()
  // function.
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>( stop - start );

  // To get the value of duration use the count()
  // member function on the duration object
  std::cout << duration.count() / 1000.0 / n << std::endl;

  return 0;
}

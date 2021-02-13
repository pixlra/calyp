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
 * \file     main.cpp
 * \brief    main file
 */

#include "CalypTools.h"

int main( int argc, char* argv[] )
{
  int iRet = 0;
  CalypTools CalypToolsApp;

  iRet = CalypToolsApp.Open( argc, argv );
  if( iRet == 1 )
  {
    return 0;
  }
  if( iRet < 0 )
  {
    printf( "Exiting with error \n" );
    return 1;
  }

  if( CalypToolsApp.Process() < 0 )
  {
    printf( "Exiting with error \n" );
    return 1;
  }

  if( CalypToolsApp.Close() < 0 )
  {
    printf( "Exiting with error \n" );
    return 1;
  }
}

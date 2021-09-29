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
 * \file     TestModuleCreate.cpp
 * \brief    CalypModuleIf testing for YUV420p sequences
 */

#include "gtest/gtest.h"
#include "lib/CalypFrame.h"
#include "lib/CalypModuleIf.h"
#include "lib/CalypStream.h"
#include "modules/CalypModulesFactory.h"

class TestModuleProcessNumberOfFrames : public ::testing::Test
{
protected:
  // Per-test-case set-up.
  // Called before the first test in this test case.
  // Can be omitted if not needed.
  static void SetUpTestCase()
  {
  }

  // Per-test-case tear-down.
  // Called after the last test in this test case.
  // Can be omitted if not needed.
  static void TearDownTestCase()
  {
    pcStream = nullptr;
  }

  // Called before each test
  void SetUp()
  {
    ClpString moduleName = CALYP_TEST_MODULE_NAME;

    pcStream = std::make_unique<CalypStream>();
    pcStream->open( ClpString( CALYP_TEST_DATA_DIR ) + ClpString( "/BasketballDrill_F10_832x480_yuv420p.yuv" ), 832, 480, 0, 8,
                    CLP_BIG_ENDIAN, 1, true );

    CalypModulesFactoryMap& moduleFactoryMap = CalypModulesFactory::Get()->getMap();
    CalypModulesFactoryMap::iterator it = moduleFactoryMap.begin();
    for( unsigned int i = 0; it != moduleFactoryMap.end(); ++it, i++ )
    {
      if( strcmp( it->first, moduleName.c_str() ) == 0 )
      {
        pcModule = it->second();
        break;
      }
    }
  }

  void TearDown()
  {
  }

  std::unique_ptr<CalypModuleIf> pcModule;

  // Some expensive resource shared by all tests.
  static std::unique_ptr<CalypStream> pcStream;
};

std::unique_ptr<CalypStream> TestModuleProcessNumberOfFrames::pcStream{ nullptr };

TEST_F( TestModuleProcessNumberOfFrames, PROCESS_OK )
{
  if( pcModule->m_iModuleAPI >= CLP_MODULE_API_2 )
  {
    std::vector<CalypFrame*> apcFrameList;
    apcFrameList.clear();
    for( unsigned int i = 0; i < pcModule->m_uiNumberOfFrames; i++ )
    {
      apcFrameList.push_back( pcStream->getCurrFrame() );
      pcStream->setNextFrame();
      pcStream->readNextFrame();
    }

    // Let's not test the modules that are not able to be created
    if( pcModule->create( apcFrameList ) )
    {
      CalypFrame* frame = pcModule->process( apcFrameList );
      EXPECT_NE( frame, nullptr );
    }
  }
}

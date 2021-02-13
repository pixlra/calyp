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

#include "CalypFrame.h"
#include "CalypModulesFactory.h"
#include "CalypStream.h"
#include "gtest/gtest.h"

class TestModuleCreate : public ::testing::Test
{
protected:
  // Per-test-case set-up.
  // Called before the first test in this test case.
  // Can be omitted if not needed.
  static void SetUpTestCase()
  {
    pcStream = new CalypStream;
    pcStream->open( ClpString( CALYP_TEST_DATA_DIR ) + ClpString( "/BasketballDrill_F10_832x480_yuv420p.yuv" ), 832, 480, 0, 8,
                    CLP_BIG_ENDIAN, 1, true );
  }

  // Per-test-case tear-down.
  // Called after the last test in this test case.
  // Can be omitted if not needed.
  static void TearDownTestCase()
  {
    pcStream->close();
    delete pcStream;
    pcStream = NULL;
  }

  // Some expensive resource shared by all tests.
  static CalypStream* pcStream;
  static CalypFrame* pcFrame;
  static CalypModuleIf* pcModule;
  static ClpString sModuleName;
};

CalypStream* TestModuleCreate::pcStream = NULL;
CalypFrame* TestModuleCreate::pcFrame = NULL;
ClpString TestModuleCreate::sModuleName = CALYP_TEST_MODULE_NAME;

TEST_F( TestModuleCreate, FIND_MODULE )
{
  CalypModuleIf* module;
  CalypModulesFactoryMap& moduleFactoryMap = CalypModulesFactory::Get()->getMap();
  CalypModulesFactoryMap::iterator it = moduleFactoryMap.begin();
  for( unsigned int i = 0; it != moduleFactoryMap.end(); ++it, i++ )
  {
    if( strcmp( it->first, sModuleName.c_str() ) == 0 )
    {
      module = it->second();
      break;
    }
  }

  EXPECT_NE( module, nullptr );
  module->Delete();
}

TEST_F( TestModuleCreate, CREATE_OK )
{
  CalypModuleIf* module;
  CalypModulesFactoryMap& moduleFactoryMap = CalypModulesFactory::Get()->getMap();
  CalypModulesFactoryMap::iterator it = moduleFactoryMap.begin();
  for( unsigned int i = 0; it != moduleFactoryMap.end(); ++it, i++ )
  {
    if( strcmp( it->first, sModuleName.c_str() ) == 0 )
    {
      module = it->second();
      break;
    }
  }
  EXPECT_NE( module, nullptr );

  bool moduleCreated = false;
  std::vector<CalypFrame*> apcFrameList;
  apcFrameList.clear();
  for( unsigned int i = 0; i < module->m_uiNumberOfFrames; i++ )
  {
    apcFrameList.push_back( pcStream->getCurrFrame() );
    pcStream->setNextFrame();
    pcStream->readNextFrame();
  }
  if( module->m_iModuleAPI >= CLP_MODULE_API_2 )
  {
    moduleCreated = module->create( apcFrameList );
  }
  else if( module->m_iModuleAPI == CLP_MODULE_API_1 )
  {
    module->create( pcStream->getCurrFrame() );
    moduleCreated = true;
  }
  EXPECT_EQ( moduleCreated, true );
}

TEST_F( TestModuleCreate, CREATE_FAIL_LESS_ONE )
{
  CalypModuleIf* module;
  CalypModulesFactoryMap& moduleFactoryMap = CalypModulesFactory::Get()->getMap();
  CalypModulesFactoryMap::iterator it = moduleFactoryMap.begin();
  for( unsigned int i = 0; it != moduleFactoryMap.end(); ++it, i++ )
  {
    if( strcmp( it->first, sModuleName.c_str() ) == 0 )
    {
      module = it->second();
      break;
    }
  }
  EXPECT_NE( module, nullptr );

  bool moduleCreated = false;
  std::vector<CalypFrame*> apcFrameList;
  apcFrameList.clear();
  for( unsigned int i = 0; i < module->m_uiNumberOfFrames - 1; i++ )
  {
    apcFrameList.push_back( pcStream->getCurrFrame() );
    pcStream->setNextFrame();
    pcStream->readNextFrame();
  }
  if( module->m_iModuleAPI >= CLP_MODULE_API_2 )
  {
    moduleCreated = module->create( apcFrameList );
  }
  else if( module->m_iModuleAPI == CLP_MODULE_API_1 )
  {
    module->create( pcStream->getCurrFrame() );
    moduleCreated = true;
  }
  EXPECT_NE( moduleCreated, true );
}

TEST_F( TestModuleCreate, CREATE_FAIL_PLUS_ONE )
{
  CalypModuleIf* module;
  CalypModulesFactoryMap& moduleFactoryMap = CalypModulesFactory::Get()->getMap();
  CalypModulesFactoryMap::iterator it = moduleFactoryMap.begin();
  for( unsigned int i = 0; it != moduleFactoryMap.end(); ++it, i++ )
  {
    if( strcmp( it->first, sModuleName.c_str() ) == 0 )
    {
      module = it->second();
      break;
    }
  }
  EXPECT_NE( module, nullptr );

  bool moduleCreated = false;
  std::vector<CalypFrame*> apcFrameList;
  apcFrameList.clear();
  for( unsigned int i = 0; i < module->m_uiNumberOfFrames + 1; i++ )
  {
    apcFrameList.push_back( pcStream->getCurrFrame() );
    pcStream->setNextFrame();
    pcStream->readNextFrame();
  }
  if( module->m_iModuleAPI >= CLP_MODULE_API_2 )
  {
    moduleCreated = module->create( apcFrameList );
  }
  else if( module->m_iModuleAPI == CLP_MODULE_API_1 )
  {
    module->create( pcStream->getCurrFrame() );
    moduleCreated = true;
  }
  EXPECT_NE( moduleCreated, true );
}
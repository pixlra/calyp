/*    This file is a part of Calyp project
 *    Copyright (C) 2014-202   by Joao Carreira   (jfmcarreira@gmail.com)
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
 * \file     CalypToolsCmdParser.cpp
 * \brief    Handle for command line
 */

#include "CalypToolsCmdParser.h"

#include <cassert>
#include <cstdarg>
#include <cstring>
#include <iostream>

#include "lib/CalypFrame.h"
#include "lib/CalypModuleIf.h"
#include "lib/CalypStream.h"
#include "modules/CalypModulesFactory.h"

CalypToolsCmdParser::CalypToolsCmdParser()
{
  m_uiLogLevel = 0;
  m_bQuiet = false;
  m_iFrames = -1;

  m_cOptions.addOptions()                                     /**/
      ( "help", "produce help message" )                      /**/
      ( "version", "show version and exit" )                  /**/
      ( "pel_fmts", "list pixel formats" )                    /**/
      ( "quality_metrics", "list supported quality metrics" ) /**/
      ( "module_list", "list supported modules" )             /**/
      ( "module_list_full", "detailed list supported modules" );
  ;
}

CalypToolsCmdParser::~CalypToolsCmdParser() {}

void CalypToolsCmdParser::log( unsigned int level, const char* fmt, ... )
{
  if( level >= m_uiLogLevel )
  {
    std::va_list args;
    va_start( args, fmt );
    vfprintf( stdout, fmt, args );
    va_end( args );
  }
}

void CalypToolsCmdParser::log( unsigned int level, std::string log_msg )
{
  if( level < m_uiLogLevel )
  {
    return;
  }
  std::cout << log_msg;
}

int CalypToolsCmdParser::parseToolsArgs( int argc, char* argv[] )
{
  int iRet = 0;

  m_cOptions.addOptions()                                                              /**/
      ( "quiet,q", m_bQuiet, "disable verbose" )                                       /**/
      ( "input,i", m_apcInputs, "input file" )                                         /**/
      ( "output,o", m_strOutput, "output file" )                                       /**/
      ( "size,s", m_strResolution, "size (WxH)" )                                      /**/
      ( "pel_fmt,p", m_strPelFmt, "pixel format" )                                     /**/
      ( "bits_pel", m_strBitsPerPixel, "bits per pixel" )                              /**/
      ( "endianness", m_strEndianness, "File endianness (big, little)" )               /**/
      ( "has_negative", m_strHasNegativeValues, "Flag for files with negatie values" ) /**/
      ( "frames,f", m_iFrames, "number of frames to parse" )                           /**/
      ( "quality", m_strQualityMetric, "select a quality metric" )                     /**/
      ( "module", m_strModule, "select a module (use internal name)" )                 /**/
      ( "save", "save a specific frame" )                                              /**/
      ( "rate-reduction", m_iRateReductionFactor, "reduce the frame rate" );           /**/

  iRet = m_cOptions.parse( argc, argv );
  if( iRet < 0 || iRet > 0 )
  {
    return iRet;
  }

  if( m_bQuiet )
  {
    m_uiLogLevel = CLP_LOG_RESULT;
  }

  if( m_cOptions.hasOpt( "module_list" ) || m_cOptions.hasOpt( "module_list_full" ) )
  {
    listModules();
    iRet = 1;
  }

  if( m_cOptions.hasOpt( "module" ) && m_cOptions.hasOpt( "help" ) )
  {
    listModuleHelp();
    iRet = 1;
  }
  else if( m_cOptions.hasOpt( "help" ) )
  {
    printf( "Usage: %s module/quality/save [options] --input=input_file [--output=output_file]\n", argv[0] );
    printf( "       %s --module=module_name [options] --input=input_file [--output=output_file]\n", argv[0] );
    printf( "       %s --quality=quality_metric [options] --input=input_file1 --input=input_file2\n", argv[0] );
    m_cOptions.doHelp( std::cout );
    iRet = 1;
  }
  return iRet;
}

void CalypToolsCmdParser::listModuleHelp()
{
  std::string moduleName = m_strModule;
  auto pcCurrModuleIf = CalypModulesFactory::Get()->CreateModule( moduleName );
  if( pcCurrModuleIf )
  {
    printf( "Usage: calypTools --module=%s options:\n", moduleName.c_str() );
    pcCurrModuleIf->m_cModuleOptions.doHelp( std::cout );
  }
}

void CalypToolsCmdParser::listModules()
{
  bool bDetailed = false;

  if( m_cOptions.hasOpt( "module_list_full" ) )
    bDetailed = true;

  const auto& moduleFactoryMap = CalypModulesFactory::Get()->getMap();

  std::cout << "Calyp available modules:" << std::endl;
  std::cout << "   [Internal Name]                  ";
  if( bDetailed )
  {
    std::cout << "   [Full Name]                                ";
    std::cout << "   [Type]        ";
    std::cout << "   [Description]";
  }
  std::cout << std::endl;

  for( const auto& [name, moduleFct] : moduleFactoryMap )
  {
    printf( "   " );
    printf( "%-33s", name.c_str() );
    if( bDetailed )
    {
      std::string ModuleNameString;
      auto pcCurrModuleIf = moduleFct();

      if( pcCurrModuleIf->m_pchModuleCategory )
      {
        ModuleNameString += pcCurrModuleIf->m_pchModuleCategory;
        ModuleNameString += "/";
      }
      ModuleNameString += pcCurrModuleIf->m_pchModuleName;
      printf( "   %-43s", ModuleNameString.c_str() );
      switch( pcCurrModuleIf->m_iModuleType )
      {
      case ClpModuleType::Invalid:
        assert( false );
        break;
      case ClpModuleType::FrameProcessing:
        printf( "   Processing    " );
        break;
      case ClpModuleType::FrameMeasurement:
        printf( "   Measurement   " );
        break;
      }
      printf( "   %s", pcCurrModuleIf->m_pchModuleTooltip );
    }
    printf( "\n" );
  }
}

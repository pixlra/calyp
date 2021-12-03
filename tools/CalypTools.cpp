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
 * \file     CalypTools.cpp
 * \brief    Main definition of the CalypTools APp
 */

#include "CalypTools.h"

#include <climits>
#include <cstring>
#include <iostream>

#include "config.h"
#include "lib/CalypFrame.h"
#include "lib/CalypModuleIf.h"
#include "lib/CalypStream.h"
#include "modules/CalypModulesFactory.h"

CalypTools::CalypTools()
{
  m_bVerbose = true;
  m_uiOperation = INVALID_OPERATION;
  m_uiNumberOfFrames = -1;

  m_uiQualityMetric = -1;

  m_pcCurrModuleIf = NULL;
}

CalypTools::~CalypTools() = default;

#define GET_PARAM( X, i ) X[X.size() > i ? i : X.size() - 1]

void CalypTools::reportStreamInfo( const CalypStream* stream, std::string strPrefix )
{
  log( CLP_LOG_INFO, "%sStream name: %s \n", strPrefix.c_str(), stream->getFileName().c_str() );
  log( CLP_LOG_INFO, "%sResolution: %dx%d@%d \n", strPrefix.c_str(), stream->getWidth(), stream->getHeight(), stream->getFrameRate() );
  log( CLP_LOG_INFO, "%sBits/pel: %d (%s)\n", strPrefix.c_str(), stream->getBitsPerPixel(), stream->getEndianess() == CLP_BIG_ENDIAN ? "BE" : "LE" );
}

int CalypTools::openInputs()
{
  /**
   * Create input streams
   */
  if( Opts().hasOpt( "input" ) )
  {
    std::vector<std::string> inputFileNames = m_apcInputs;

    std::string resolutionString( "" );
    std::string fmtString( "yuv420p" );
    unsigned int uiBitsPerPixel = 8;
    int uiEndianness = 0;
    bool hasNegativeValues = false;

    CalypStream* pcStream;
    for( unsigned int i = 0; i < inputFileNames.size() && i < MAX_NUMBER_INPUTS; i++ )
    {
      if( Opts().hasOpt( "size" ) )
      {
        resolutionString = GET_PARAM( m_strResolution, i );
      }
      if( Opts().hasOpt( "pel_fmt" ) )
      {
        fmtString = GET_PARAM( m_strPelFmt, i );
      }
      if( Opts().hasOpt( "bits_pel" ) )
      {
        uiBitsPerPixel = std::stoi( GET_PARAM( m_strBitsPerPixel, i ).c_str() );
      }
      if( Opts().hasOpt( "endianness" ) )
      {
        if( GET_PARAM( m_strEndianness, i ) == "big" )
        {
          uiEndianness = 0;
        }
        else if( GET_PARAM( m_strEndianness, i ) == "little" )
        {
          uiEndianness = 1;
        }
      }
      if( Opts().hasOpt( "has_negative" ) )
      {
        hasNegativeValues = std::stoi( GET_PARAM( m_strHasNegativeValues, i ).c_str() ) == 0 ? false : true;
      }
      pcStream = new CalypStream;
      try
      {
        if( !pcStream->open( inputFileNames[i], resolutionString, fmtString, uiBitsPerPixel, uiEndianness, hasNegativeValues, 1, CalypStream::Type::Input ) )
        {
          log( CLP_LOG_ERROR, "Cannot open input stream %s! ", inputFileNames[i].c_str() );
          return -1;
        }
        m_apcInputStreams.push_back( pcStream );
        log( CLP_LOG_INFO, "Found input %d \n", m_apcInputStreams.size() );
        reportStreamInfo( pcStream );
      }
      catch( const char* msg )
      {
        log( CLP_LOG_ERROR, "Cannot open input stream %s with the following error: \n%s\n", inputFileNames[i].c_str(), msg );
        return -1;
      }
    }
  }

  m_uiNumberOfFrames = -1;
  if( Opts().hasOpt( "frames" ) )
  {
    m_uiNumberOfFrames = m_iFrames;
  }

  m_uiNumberOfComponents = -1;
  for( unsigned int i = 0; i < m_apcInputStreams.size(); i++ )
  {
    m_uiNumberOfFrames = std::min( m_uiNumberOfFrames, m_apcInputStreams[i]->getFrameNum() );
    m_uiNumberOfComponents =
        std::min( m_uiNumberOfComponents, m_apcInputStreams[i]->getCurrFrame()->getNumberChannels() );
  }

  return 0;
}

int CalypTools::Open( int argc, char* argv[] )
{
  int iRet = 0;

  log( CLP_LOG_ERROR, "calypTools - The command line interface for Calyp modules! \n" );

  // check requirements
  if( CalypPixel::getMaxNumberOfComponents() > MAX_NUMBER_CHANNELS )
  {
    log( CLP_LOG_ERROR, "Cannot parse the maximum number of components!" );
    return -1;
  }
  if( ( iRet = parseToolsArgs( argc, argv ) ) < 0 )
  {
    return iRet;
  }
  if( iRet == 1 )
  {
    return iRet;
  }

  if( ( iRet = openInputs() ) < 0 )
  {
    return iRet;
  }

  m_uiOutEndianness = 0;
  if( Opts().hasOpt( "endianness" ) )
  {
    if( GET_PARAM( m_strEndianness, 9999 ) == "big" )
    {
      m_uiOutEndianness = 0;
    }
    else if( GET_PARAM( m_strEndianness, 9999 ) == "little" )
    {
      m_uiOutEndianness = 1;
    }
  }

  if( Opts().hasOpt( "save" ) )
  {
    if( m_apcInputStreams.size() == 0 )
    {
      log( CLP_LOG_ERROR, "Invalid number of input streams! " );
      return -1;
    }
    long long int currFrames = 0;
    long long int numberOfFrames = LONG_MAX;
    for( unsigned int i = 0; i < m_apcInputStreams.size(); i++ )
    {
      currFrames = m_apcInputStreams[i]->getFrameNum();
      if( currFrames < numberOfFrames )
        numberOfFrames = currFrames;
    }
    m_iFrameNum = m_iFrames;
    if( !( m_iFrameNum >= 0 && m_iFrameNum < numberOfFrames ) )
    {
      log( CLP_LOG_ERROR, "Invalid frame number! Use --frame option " );
      return -1;
    }
    if( Opts().hasOpt( "output" ) )
      m_pcOutputFileNames.push_back( m_strOutput );
    if( m_pcOutputFileNames.size() != m_apcInputStreams.size() )
    {
      log( CLP_LOG_ERROR, "Invalid number of outputs! Each input must have an "
                          "output filename. " );
      return -1;
    }

    m_uiOperation = SAVE_OPERATION;
    m_fpProcess = &CalypTools::SaveOperation;
    log( CLP_LOG_INFO, "Calyp Save Frame\n" );
  }

  if( Opts().hasOpt( "rate-reduction" ) )
  {
    if( m_apcInputStreams.size() == 0 )
    {
      log( CLP_LOG_ERROR, "Invalid number of input streams! " );
      return -1;
    }
    long long int currFrames = 0;
    long long int numberOfFrames = LONG_MAX;
    for( unsigned int i = 0; i < m_apcInputStreams.size(); i++ )
    {
      currFrames = m_apcInputStreams[i]->getFrameNum();
      if( currFrames < numberOfFrames )
        numberOfFrames = currFrames;
    }
    if( m_iRateReductionFactor <= 0 )
    {
      log( CLP_LOG_ERROR, "Invalid frame rate reduction value!" );
      return -1;
    }

    if( Opts().hasOpt( "output" ) )
      m_pcOutputFileNames.push_back( m_strOutput );

    const CalypFrame* pcInputFrame = m_apcInputStreams[0]->getCurrFrame();
    CalypStream* pcOutputStream = new CalypStream;
    try
    {
      pcOutputStream->open( m_pcOutputFileNames[0], pcInputFrame->getWidth(), pcInputFrame->getHeight(),
                            pcInputFrame->getPelFormat(), pcInputFrame->getBitsPel(), m_uiOutEndianness, 1,
                            CalypStream::Type::Output );
      log( CLP_LOG_INFO, "Output stream from rate-reduction!\n" );
      reportStreamInfo( pcOutputStream, "Output " );
    }
    catch( const char* msg )
    {
      log( CLP_LOG_ERROR, "Cannot open input stream %s with the following error %s!\n", m_pcOutputFileNames[0].c_str(),
           msg );
      delete pcOutputStream;
      pcOutputStream = NULL;
      return -1;
    }
    m_apcOutputStreams.push_back( pcOutputStream );

    if( m_apcOutputStreams.size() != m_apcInputStreams.size() )
    {
      log( CLP_LOG_ERROR, "Invalid number of outputs! Each input must have an "
                          "output filename. " );
      return -1;
    }

    m_uiOperation = RATE_REDUCTION_OPERATION;
    m_fpProcess = &CalypTools::RateReductionOperation;
    log( CLP_LOG_INFO, "Calyp Frame Rate Reduction\n" );
  }

  /**
   * Check Quality operation
   */
  if( Opts().hasOpt( "quality" ) )
  {
    std::string qualityMetric = m_strQualityMetric;
    if( m_apcInputStreams.size() < 2 )
    {
      log( CLP_LOG_ERROR, "Invalid number of inputs! " );
      return -1;
    }
    for( unsigned int i = 0; i < CalypFrame::supportedQualityMetricsList().size(); i++ )
    {
      if( clpLowercase( CalypFrame::supportedQualityMetricsList()[i] ) == clpLowercase( qualityMetric ) )
      {
        m_uiQualityMetric = i;
      }
    }
    if( m_uiQualityMetric == -1 )
    {
      log( CLP_LOG_ERROR, "Invalid quality metric! " );
      return -1;
    }
    m_uiOperation = QUALITY_OPERATION;
    m_fpProcess = &CalypTools::QualityOperation;
    log( CLP_LOG_INFO, "Calyp Quality\n" );
  }

  /**
   * Check Module operation
   */
  if( Opts().hasOpt( "module" ) )
  {
    std::string moduleName = m_strModule;

    CalypModulesFactoryMap& moduleFactoryMap = CalypModulesFactory::Get()->getMap();
    CalypModulesFactoryMap::iterator it = moduleFactoryMap.begin();
    for( unsigned int i = 0; it != moduleFactoryMap.end(); ++it, i++ )
    {
      if( strcmp( it->first, moduleName.c_str() ) == 0 )
      {
        m_pcCurrModuleIf = it->second();
        break;
      }
    }
    if( !m_pcCurrModuleIf )
    {
      log( CLP_LOG_ERROR, "Invalid module! " );
      return -1;
    }

    if( m_pcCurrModuleIf->has( ClpModuleFeature::VariableNumOfFrames ) )
    {
      m_pcCurrModuleIf->m_uiNumberOfFrames = m_apcInputStreams.size();
    }
    else
    {
      if( m_apcInputStreams.size() != m_pcCurrModuleIf->m_uiNumberOfFrames )
      {
        log( CLP_LOG_ERROR, "Invalid number of inputs! " );
        return -1;
      }
    }

    m_pcCurrModuleIf->m_cModuleOptions.parse( argc, argv );

    // Create Module
    bool moduleCreated = false;
    std::vector<CalypFrame*> apcFrameList;
    for( unsigned int i = 0; i < m_pcCurrModuleIf->m_uiNumberOfFrames; i++ )
    {
      apcFrameList.push_back( m_apcInputStreams[i]->getCurrFrame() );
    }
    if( m_pcCurrModuleIf->m_iModuleAPI >= CLP_MODULE_API_2 )
    {
      moduleCreated = m_pcCurrModuleIf->create( apcFrameList );
    }
    else if( m_pcCurrModuleIf->m_iModuleAPI == CLP_MODULE_API_1 )
    {
      m_pcCurrModuleIf->create( m_apcInputStreams[0]->getCurrFrame() );
      moduleCreated = true;
    }
    if( !moduleCreated )
    {
      log( CLP_LOG_ERROR, "Module is not supported with the selected inputs! " );
      return -1;
    }

    if( m_pcCurrModuleIf->m_iModuleType == ClpModuleType::FrameProcessing )
    {
      // Check outputs
      std::vector<std::string> outputFileNames;
      if( Opts().hasOpt( "output" ) )
        outputFileNames.push_back( m_strOutput );

      if( outputFileNames.size() == 1 )
      {
        CalypFrame* pcModFrame;
        if( m_pcCurrModuleIf->m_iModuleAPI >= CLP_MODULE_API_2 )
        {
          pcModFrame = m_pcCurrModuleIf->getProcessedFrame();
          if( !pcModFrame )
          {
            pcModFrame = m_pcCurrModuleIf->process( apcFrameList );
            m_pcCurrModuleIf->flush();
          }
        }
        else
        {
          pcModFrame = m_pcCurrModuleIf->process( m_apcInputStreams[0]->getCurrFrame() );
        }
        CalypStream* pcModStream = new CalypStream;
        try
        {
          pcModStream->open( outputFileNames[0], pcModFrame->getWidth(), pcModFrame->getHeight(),
                             pcModFrame->getPelFormat(), pcModFrame->getBitsPel(), m_uiOutEndianness, 1, CalypStream::Type::Output );
          log( CLP_LOG_INFO, "Output stream from module!\n" );
          reportStreamInfo( pcModStream, "Module Output " );
        }
        catch( const char* msg )
        {
          log( CLP_LOG_ERROR, "Cannot open input stream %s with the following error %s!\n", outputFileNames[0].c_str(),
               msg );
          delete pcModStream;
          pcModStream = NULL;
          return -1;
        }
        m_apcOutputStreams.push_back( pcModStream );
      }
      else
      {
        log( CLP_LOG_ERROR, "One output is required! " );
        return -1;
      }
    }

    m_uiOperation = MODULE_OPERATION;
    m_fpProcess = &CalypTools::ModuleOperation;
    log( CLP_LOG_INFO, "Calyp Module\n" );
  }

  /**
   * Check Statistics operation
   */
  if( Opts().hasOpt( "statistics" ) )
  {
    if( m_apcInputStreams.empty() )
    {
      log( CLP_LOG_ERROR, "Invalid number of inputs! " );
      return -1;
    }

    m_uiOperation = STATISTICS_OPERATION;
    m_fpProcess = &CalypTools::ListStatistics;
  }

  if( m_uiOperation == INVALID_OPERATION )
  {
    log( CLP_LOG_ERROR, "No operation was selected! Use --help to see usage.\n" );
    return 1;
  }
  return iRet;
}

int CalypTools::Process()
{
  return ( this->*m_fpProcess )();
}

int CalypTools::Close()
{
  // Finish
  return 0;
}

int CalypTools::SaveOperation()
{
  bool bRet = true;
  for( unsigned int s = 0; s < m_apcInputStreams.size(); s++ )
  {
    bRet = m_apcInputStreams[s]->seekInput( m_iFrameNum );
    if( bRet == false )
    {
      log( CLP_LOG_INFO, "Cannot seek input file to frame %d! ", m_iFrameNum );
      return -1;
    }
    m_apcInputStreams[s]->saveFrame( m_pcOutputFileNames[s] );
  }
  return 0;
}

int CalypTools::RateReductionOperation()
{
  bool abEOF;
  log( CLP_LOG_INFO, "\n Reducing frame rate by a factor of %d ... ", m_iRateReductionFactor );
  for( unsigned int frame = 0; frame < m_uiNumberOfFrames; frame++ )
  {
    log( CLP_LOG_INFO, "\n Reading frame %d ... ", frame );
    if( ( frame % m_iRateReductionFactor ) == 0 )
    {
      log( CLP_LOG_INFO, "Writing", frame );
      m_apcOutputStreams[0]->writeFrame( *m_apcInputStreams[0]->getCurrFrame() );
    }
    abEOF = m_apcInputStreams[0]->setNextFrame();
    if( !abEOF )
    {
      m_apcInputStreams[0]->readNextFrame();
    }
  }
  return 0;
}

int CalypTools::QualityOperation()
{
  const char* pchQualityMetricName = CalypFrame::supportedQualityMetricsList()[m_uiQualityMetric].c_str();
  CalypFrame* apcCurrFrame[MAX_NUMBER_INPUTS];
  bool abEOF[MAX_NUMBER_INPUTS];
  double adAverageQuality[MAX_NUMBER_INPUTS - 1][MAX_NUMBER_CHANNELS];
  double dQuality;

  std::string metric_fmt = " ";
  switch( m_uiQualityMetric )
  {
  case CalypFrame::PSNR_METRIC:
    //"PSNR_0_0"
    metric_fmt += " %6.3f ";
    break;
  case CalypFrame::SSIM_METRIC:
    //"SSIM_0_0"
    metric_fmt += " %6.4f ";
    break;
  case CalypFrame::MSE_METRIC:
    //"MSE_0_0"
    metric_fmt += "%7.2f";
    break;
  default:
    metric_fmt += " %6.3f ";
  }
  metric_fmt += " ";

  log( CLP_LOG_INFO, "  Measuring Quality using %s ... \n", pchQualityMetricName );
  log( CLP_LOG_INFO, "# Frame   ", pchQualityMetricName );

  for( unsigned int s = 1; s < m_apcInputStreams.size(); s++ )
  {
    for( unsigned int c = 0; c < m_uiNumberOfComponents; c++ )
    {
      log( CLP_LOG_INFO, "%s_%d_%d  ", pchQualityMetricName, s, c );
    }
    log( CLP_LOG_INFO, "   " );
  }

  log( CLP_LOG_INFO, "\n" );

  for( unsigned int s = 0; s < m_apcInputStreams.size(); s++ )
  {
    abEOF[s] = false;
    for( unsigned int c = 0; c < m_uiNumberOfComponents; c++ )
    {
      adAverageQuality[s][c] = 0;
    }
  }
  for( unsigned int frame = 0; frame < m_uiNumberOfFrames; frame++ )
  {
    log( CLP_LOG_INFO, "  %3d  ", frame );
    for( unsigned int s = 0; s < m_apcInputStreams.size(); s++ )
      apcCurrFrame[s] = m_apcInputStreams[s]->getCurrFrame();

    for( unsigned int s = 1; s < m_apcInputStreams.size(); s++ )
    {
      log( CLP_LOG_RESULT, "  " );
      for( unsigned int c = 0; c < m_uiNumberOfComponents; c++ )
      {
        dQuality = apcCurrFrame[s]->getQuality( m_uiQualityMetric, apcCurrFrame[0], c );
        adAverageQuality[s - 1][c] = ( adAverageQuality[s - 1][c] * double( frame ) + dQuality ) / double( frame + 1 );
        log( CLP_LOG_RESULT, metric_fmt.c_str(), dQuality );
      }
      log( CLP_LOG_RESULT, " " );
    }
    log( CLP_LOG_RESULT, "\n" );
    for( unsigned int s = 0; s < m_apcInputStreams.size(); s++ )
    {
      abEOF[s] = m_apcInputStreams[s]->setNextFrame();
      if( !abEOF[s] )
      {
        m_apcInputStreams[s]->readNextFrame();
      }
    }
  }
  log( CLP_LOG_INFO, "\n  Mean Values: \n         " );
  for( unsigned int s = 0; s < m_apcInputStreams.size() - 1; s++ )
  {
    for( unsigned int c = 0; c < m_uiNumberOfComponents; c++ )
    {
      log( CLP_LOG_INFO, metric_fmt.c_str(), adAverageQuality[s][c] );
    }
    log( CLP_LOG_RESULT, "   " );
  }
  log( CLP_LOG_INFO, "\n" );
  return 0;
}

// CalypFrame* CalypTools::applyFrameModule()
//{
//   CalypFrame* pcProcessedFrame = NULL;
//   if( m_pcCurrModuleIf->m_iModuleType == ClpModuleType::FrameProcessing )
//   {

//  }
//  return pcProcessedFrame;
//}

std::vector<CalypFrame*> CalypTools::readInput()
{
  std::vector<CalypFrame*> apcFrameList;
  apcFrameList.clear();
  // Check EOF and read next frame
  bool abEOF[MAX_NUMBER_INPUTS];
  for( unsigned int s = 0; s < m_apcInputStreams.size(); s++ )
  {
    abEOF[s] = m_apcInputStreams[s]->setNextFrame();
    if( abEOF[s] )
    {
      return apcFrameList;
    }
    m_apcInputStreams[s]->readNextFrame();
  }
  for( unsigned int i = 0; i < m_pcCurrModuleIf->m_uiNumberOfFrames; i++ )
  {
    apcFrameList.push_back( m_apcInputStreams[i]->getCurrFrame() );
  }
  return apcFrameList;
}

int CalypTools::ModuleOperation()
{
  log( CLP_LOG_INFO, "  Applying Module %s/%s ...\n", m_pcCurrModuleIf->m_pchModuleCategory,
       m_pcCurrModuleIf->m_pchModuleName );

  CalypFrame* pcProcessedFrame = NULL;
  double dMeasurementResult = 0.0;
  double dAveragedMeasurementResult = 0;

  std::vector<CalypFrame*> apcFrameList;
  for( unsigned int i = 0; i < m_pcCurrModuleIf->m_uiNumberOfFrames; i++ )
  {
    apcFrameList.push_back( m_apcInputStreams[i]->getCurrFrame() );
  }

  for( unsigned int frame = 0; frame < m_uiNumberOfFrames; )
  {
    log( CLP_LOG_INFO, "  Processing frame %3d\n", frame );
    bool bReadFrame = true;
    if( m_pcCurrModuleIf->m_iModuleType == ClpModuleType::FrameProcessing )
    {
      if( m_pcCurrModuleIf->m_iModuleAPI >= CLP_MODULE_API_2 )
        pcProcessedFrame = m_pcCurrModuleIf->process( apcFrameList );
      else
        pcProcessedFrame = m_pcCurrModuleIf->process( m_apcInputStreams[0]->getCurrFrame() );

      if( pcProcessedFrame )
      {
        m_apcOutputStreams[0]->writeFrame( *pcProcessedFrame );
      }
    }
    else if( m_pcCurrModuleIf->m_iModuleType == ClpModuleType::FrameMeasurement )
    {
      if( m_pcCurrModuleIf->m_iModuleAPI >= CLP_MODULE_API_2 )
        dMeasurementResult = m_pcCurrModuleIf->measure( apcFrameList );
      else
        dMeasurementResult = m_pcCurrModuleIf->measure( m_apcInputStreams[0]->getCurrFrame() );
      log( CLP_LOG_INFO, "   %3d", frame );
      log( CLP_LOG_RESULT, "  %8.3f \n", dMeasurementResult );
      dAveragedMeasurementResult =
          ( dAveragedMeasurementResult * double( frame ) + dMeasurementResult ) / double( frame + 1 );
    }
    apcFrameList.clear();
    if( m_pcCurrModuleIf->m_iModuleAPI == CLP_MODULE_API_3 )
    {
      bReadFrame = m_pcCurrModuleIf->needFrame();
    }
    if( bReadFrame )
    {
      apcFrameList = readInput();
      frame++;
    }
  }

  if( m_pcCurrModuleIf->m_iModuleAPI >= CLP_MODULE_API_3 )
  {
    while( true )
    {
      if( m_pcCurrModuleIf->m_iModuleType == ClpModuleType::FrameProcessing )
      {
        pcProcessedFrame = m_pcCurrModuleIf->process( apcFrameList );
        if( !pcProcessedFrame )
          break;
        m_apcOutputStreams[0]->writeFrame( *pcProcessedFrame );
      }
    }
  }

  if( m_pcCurrModuleIf->m_iModuleType == ClpModuleType::FrameMeasurement )
  {
    log( CLP_LOG_INFO, "\n  Mean Value: \n        %8.3f\n", dAveragedMeasurementResult );
  }

  return 0;
}

int CalypTools::ListStatistics()
{
  log( CLP_LOG_RESULT, "\n\x1B[35mCalyp Statistics:\x1B[0m\n\n" );

  bool abEOF;
  unsigned min[MAX_NUMBER_CHANNELS], max[MAX_NUMBER_CHANNELS];

  for( unsigned input = 0; input < m_apcInputStreams.size(); input++ )
  {
    log( CLP_LOG_RESULT, "\x1B[32mInput:\t\t\t%d\x1B[0m\n", input );
    log( CLP_LOG_RESULT, "No. Frames:\t\t%d\n", m_apcInputStreams[input]->getFrameNum() );
    log( CLP_LOG_RESULT, "Pixels:\t\t\t%d\n", m_apcInputStreams[input]->getHeight() * m_apcInputStreams[input]->getWidth() );

    for( unsigned int frame = 0; frame < m_apcInputStreams[input]->getFrameNum(); frame++ )
    {
      log( CLP_LOG_RESULT, "\x1B[34m  Frame: %d\x1B[0m\n", frame );

      auto currFrame = m_apcInputStreams[input]->getCurrFrame();
      abEOF = m_apcInputStreams[input]->setNextFrame();
      if( !abEOF )
      {
        m_apcInputStreams[input]->readNextFrame();
      }

      currFrame->calcHistogram();

      for( unsigned channel = 0; channel < currFrame->getNumberChannels(); channel++ )
      {
        min[channel] = currFrame->getMinimumPelValue( channel );
        max[channel] = currFrame->getMaximumPelValue( channel );
      }

      log( CLP_LOG_RESULT, "    Channel:        " );
      for( unsigned channel = 0; channel < currFrame->getNumberChannels(); channel++ )
        log( CLP_LOG_RESULT, "| %13d ", channel );
      log( CLP_LOG_RESULT, "|\n" );

      log( CLP_LOG_RESULT, "    ----------------" );
      for( unsigned channel = 0; channel < currFrame->getNumberChannels(); channel++ )
        log( CLP_LOG_RESULT, "----------------" );
      log( CLP_LOG_RESULT, "-\n" );

      log( CLP_LOG_RESULT, "    Range:          " );
      for( unsigned channel = 0; channel < currFrame->getNumberChannels(); channel++ )
      {
        char buffer[14];
        sprintf( buffer, "[%d:%d]", min[channel], max[channel] );

        log( CLP_LOG_RESULT, "| %13s ", buffer );
      }
      log( CLP_LOG_RESULT, "|\n" );

      log( CLP_LOG_RESULT, "    Non empty bins: " );
      for( unsigned channel = 0; channel < currFrame->getNumberChannels(); channel++ )
        log( CLP_LOG_RESULT, "| %13d ", currFrame->getNEBins( channel ) );
      log( CLP_LOG_RESULT, "|\n" );

      log( CLP_LOG_RESULT, "    Mean:           " );
      for( unsigned channel = 0; channel < currFrame->getNumberChannels(); channel++ )
        log( CLP_LOG_RESULT, "| %13.1f ", currFrame->getMean( channel, min[channel], max[channel] ) );
      log( CLP_LOG_RESULT, "|\n" );

      log( CLP_LOG_RESULT, "    Std. deviation: " );
      for( unsigned channel = 0; channel < currFrame->getNumberChannels(); channel++ )
        log( CLP_LOG_RESULT, "| %13.1f ", currFrame->getStdDev( channel, min[channel], max[channel] ) );
      log( CLP_LOG_RESULT, "|\n" );

      log( CLP_LOG_RESULT, "    Median:         " );
      for( unsigned channel = 0; channel < currFrame->getNumberChannels(); channel++ )
        log( CLP_LOG_RESULT, "| %13d ", currFrame->getMedian( channel, min[channel], max[channel] ) );
      log( CLP_LOG_RESULT, "|\n" );

      log( CLP_LOG_RESULT, "    Entropy:        " );
      for( unsigned channel = 0; channel < currFrame->getNumberChannels(); channel++ )
        log( CLP_LOG_RESULT, "| %13.2f ", currFrame->getEntropy( channel, min[channel], max[channel] ) );
      log( CLP_LOG_RESULT, "|\n" );
    }
  }

  return 0;
}

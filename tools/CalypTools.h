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
 * \file     CalypTools.h
 * \brief    Main definition of the CalypTools app
 *
 * @defgroup CalypTools Calyp Tools
 * @{
 *
 * CalypTools is a command line interface
 * for the modules and quality metrics
 * included in Calyp
 *
 * @}
 */

#ifndef __CALYPTOOLS_H__
#define __CALYPTOOLS_H__

/**
 * @defgroup CalypTools Calyp Tools
 * @{
 * CalypTools is a command line interface
 * for the modules and quality metrics
 * included in Calyp
 * @}
 *
 */

#include <vector>

#include "CalypToolsCmdParser.h"
#include "lib/CalypModuleIf.h"

class CalypFrame;
class CalypStream;

constexpr std::size_t MAX_NUMBER_INPUTS = 255;
constexpr std::size_t MAX_NUMBER_CHANNELS = 4;

class CalypTools : public CalypToolsCmdParser
{
public:
  CalypTools();
  CalypTools( const CalypTools& ) = delete;
  CalypTools( CalypTools&& ) = delete;
  auto operator=( const CalypTools& ) -> CalypTools& = delete;
  auto operator=( CalypTools&& ) -> CalypTools& = delete;
  ~CalypTools();

  auto Open( int argc, char* argv[] ) -> int;
  auto Process() -> int;
  auto Close() -> int;

private:
  bool m_bVerbose;

  unsigned int m_uiOperation;
  enum TOOLS_OPERATIONS_LIST
  {
    INVALID_OPERATION,
    SAVE_OPERATION,
    RATE_REDUCTION_OPERATION,
    QUALITY_OPERATION,
    MODULE_OPERATION,
    STATISTICS_OPERATION,
  };

  std::uint64_t m_uiNumberOfFrames{ 0 };
  unsigned int m_uiNumberOfComponents{ 0 };
  std::vector<CalypStream*> m_apcInputStreams;
  std::vector<CalypStream*> m_apcOutputStreams;

  void reportStreamInfo( const CalypStream* stream, std::string strPrefix = "" );
  auto openInputs() -> int;
  auto readInput() -> std::vector<CalypFrame*>;

  using FpProcess = int ( CalypTools::* )();
  FpProcess m_fpProcess;

  std::int64_t m_iFrameNum{ 0 };
  std::vector<std::string> m_pcOutputFileNames;
  auto SaveOperation() -> int;

  auto RateReductionOperation() -> int;

  int m_uiQualityMetric;
  auto QualityOperation() -> int;

  CalypModulePtr m_pcCurrModuleIf;
  auto ModuleOperation() -> int;
  auto ListStatistics() -> int;
};

#endif  // __CALYPTOOLS_H__

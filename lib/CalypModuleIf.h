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
 * \file     CalypModuleIf.h
 * \ingroup  CalypLibGrp
 * \brief    Calyp modules interface
 */

#ifndef __CALYPMODULESIF_H__
#define __CALYPMODULESIF_H__

/**
 * @defgroup Calyp_Modules Calyp Modules
 * @{
 * CalypLib supports the creation of independent
 * frame processing modules
 * @}
 */

#include <memory>
#include <type_safe/flag_set.hpp>
#include <vector>

#include "CalypFrame.h"
#include "CalypOptions.h"

#define REGISTER_CLASS_MAKER( X ) \
  extern "C" CalypModulePtr Maker() { return std::make_unique<X>(); }
#define REGISTER_CLASS_FACTORY( X )                   \
public:                                               \
  X( const X& buffer )                                \
  noexcept = delete;                                  \
  X( X&& buffer )                                     \
  noexcept = delete;                                  \
  auto operator=( const X& buffer )->X& = delete;     \
  auto operator=( X&& buffer ) noexcept->X& = delete; \
  static CalypModulePtr Create() { return std::make_unique<X>(); }

#define _BASIC_MODULE_API_2_CHECK_                        \
  if( apcFrameList.size() != m_uiNumberOfFrames )         \
    return false;                                         \
  for( unsigned int i = 0; i < apcFrameList.size(); i++ ) \
    if( !apcFrameList[i] )                                \
      return false;

#define _BASIC_MODULE_API_3_BREAK_CONDITION_ \
  if( m_iFrameBufferCount == 0 )             \
    if( apcFrameList.size() <= 0 )           \
      return NULL;

/** Module_API_Version Enum
 * \ingroup Calyp_Modules
 * Version of the modules API required
 * @see m_iModuleAPI
 */
enum Module_API_Version
{
  CLP_MODULE_API_INVALID = -1,
  CLP_MODULE_API_1 = 0,
  CLP_MODULE_API_2 = 1,
  CLP_MODULE_API_3,
};

/** Module_Type Enum
 * \ingroup Calyp_Modules
 * Type of module
 * @see m_iModuleType
 */
enum class ClpModuleType : std::uint8_t
{
  Invalid,
  FrameProcessing,
  FrameMeasurement,
};

/** Module_Features Enum
 * \ingroup Calyp_Modules
 * Features/Requirements of a module
 * @see m_uiModuleRequirements
 */
enum class ClpModuleFeature
{
  None,
  SkipWhilePlaying,
  Options,
  NewWindow,
  KeysShortcuts,
  VariableNumOfFrames,
  HasInfo,
  _flag_set_size
};

using ClpModuleFeatures = type_safe::flag_set<ClpModuleFeature>;

enum Module_Key_Supported
{
  MODULE_KEY_LEFT,
  MODULE_KEY_RIGHT,
  MODULE_KEY_UP,
  MODULE_KEY_DOWN,
};

/**
 * \class    CalypModuleIf
 * \ingroup  CalypLib Calyp_Modules
 * \brief    Abstract class for modules
 */
class CalypModuleIf
{
public:
  CalypModuleIf() = default;
  CalypModuleIf( const CalypModuleIf& buffer ) noexcept = delete;
  CalypModuleIf( CalypModuleIf&& buffer ) noexcept = delete;
  auto operator=( const CalypModuleIf& buffer ) -> CalypModuleIf& = delete;
  auto operator=( CalypModuleIf&& buffer ) noexcept -> CalypModuleIf& = delete;

  virtual ~CalypModuleIf() = default;
  virtual void destroy(){};

  const char* getModuleLongName()
  {
    return m_pchModuleLongName ? m_pchModuleLongName : m_pchModuleName;
  }

  /**
   * Module API version 1
   */
  virtual void create( CalypFrame* ) {}
  virtual CalypFrame* process( CalypFrame* ) { return NULL; }
  virtual double measure( CalypFrame* ) { return 0; }
  /**
   * Module API version 2
   */
  virtual bool create( std::vector<CalypFrame*> apcFrameList ) { return false; }
  virtual CalypFrame* process( std::vector<CalypFrame*> ) { return NULL; }
  virtual double measure( std::vector<CalypFrame*> ) { return 0; }
  virtual bool keyPressed( enum Module_Key_Supported ) { return false; }
  virtual std::string moduleInfo() { return std::string(); }
  /**
   * Module API version 3
   */
  virtual CalypFrame* getProcessedFrame() { return nullptr; }
  virtual bool needFrame() { return m_iFrameBufferCount == 0 ? true : false; };
  virtual bool flush()
  {
    m_iFrameBufferCount = 0;
    return true;
  };

public:
  int m_iModuleAPI{ CLP_MODULE_API_1 };
  ClpModuleType m_iModuleType{ ClpModuleType::Invalid };
  const char* m_pchModuleCategory{ "" };
  const char* m_pchModuleName{ "" };
  const char* m_pchModuleTooltip{ "" };
  const char* m_pchModuleLongName{ nullptr };

  //! Number of frames
  unsigned int m_uiNumberOfFrames{ 1 };
  //! Features/Requirements
  ClpModuleFeatures m_uiModuleRequirements{ ClpModuleFeature::None };

  unsigned int m_iFrameBufferCount{ 0 };

  CalypOptions m_cModuleOptions;
};

namespace cv
{
class Mat;
}

/**
 * \class    CalypOpenCVModuleIf
 * \ingroup  CalypLib Calyp_Modules
 * \brief    Abstract class for modules using OpenCV library
 */
class CalypOpenCVModuleIf : public CalypModuleIf
{
  using Mat = cv::Mat;

public:
  CalypOpenCVModuleIf() { m_bConvertToGray = false; };
  virtual ~CalypOpenCVModuleIf() {}

  // Common API
  bool create( std::vector<CalypFrame*> apcFrameList );
  CalypFrame* process( std::vector<CalypFrame*> apcFrameList );
  void destroy();

  // API using OpenCV
  virtual cv::Mat* create_using_opencv( const std::vector<Mat>& apcFrameList )
  {
    return nullptr;
  };
  virtual Mat* process_using_opencv( const std::vector<Mat>& apcFrameList ) = 0;
  virtual void destroy_using_opencv(){};

protected:
  // const char* m_pchPythonFunctionName;
  std::unique_ptr<CalypFrame> m_pcOutputFrame;
  bool m_bConvertToGray;
};

using CalypModulePtr = std::unique_ptr<CalypModuleIf>;

template <typename T>
class CalypModuleInstace
{
public:
  static CalypModulePtr Create() { return std::make_unique<T>(); }
};

#endif  // __CALYPMODULESIF_H__

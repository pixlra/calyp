/*    This file is a part of Calyp project
 *    Copyright (C) 2014-2019  by Joao Carreira   (jfmcarreira@gmail.com)
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
 * \file     CalypOpenCVModuleIf.h
 * \ingroup  CalypLibGroup
 * \brief    Calyp modules interface for OpenCV
 */

#ifndef __CALYPOPENCVMODULEIF_H__
#define __CALYPOPENCVMODULEIF_H__

#include "CalypModuleIf.h"

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
protected:
  const char* m_pchPythonFunctionName;
  bool m_bConvertToGray;

public:
  CalypOpenCVModuleIf()
  {
    m_bConvertToGray = false;
  };
  virtual ~CalypOpenCVModuleIf() {}

  // Common API
  bool create( std::vector<CalypFrame*> apcFrameList );
  CalypFrame* process( std::vector<CalypFrame*> apcFrameList );
  void destroy();

  // API using OpenCV
  virtual bool create_using_opencv( std::vector<cv::Mat*> apcFrameList ) { return true; };
  virtual cv::Mat* process_using_opencv( std::vector<cv::Mat*> apcFrameList ) = 0;
  virtual void destroy_using_opencv(){};
};

#endif  // __CALYPOPENCVMODULEIF_H__

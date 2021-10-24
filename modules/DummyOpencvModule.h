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
 * \file     DummyOpencvModule.h
 * \brief    Modules to detect curves shape
 */

#ifndef __DUMMYOPENCVMODULE_H__
#define __DUMMYOPENCVMODULE_H__

// CalypLib
#include "lib/CalypModuleIf.h"

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/saliency.hpp>

class DummyOpencvModule : public CalypOpenCVModuleIf,
                          public CalypModuleInstace<DummyOpencvModule>
{
  using Mat = cv::Mat;

private:
  std::unique_ptr<Mat> m_resultImage;

public:
  DummyOpencvModule();
  virtual ~DummyOpencvModule() = default;
  Mat* create_using_opencv( const std::vector<Mat>& apcMatList ) override;
  Mat* process_using_opencv( const std::vector<Mat>& apcMatList ) override;
};

#endif  // __DUMMYOPENCVMODULE_H__

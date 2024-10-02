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
 * \file     ThreeSixtyNxN.h
 * \brief    Downsampling ERP to NxN
 */

#ifndef __THREESIXTYNXN_H__
#define __THREESIXTYNXN_H__

// CalypLib
#include "lib/CalypModuleIf.h"
#include "opencv2/core/mat.hpp"
#include "opencv2/core/types.hpp"

using cv::Mat;
using cv::Mat_;
using cv::Point;

class ThreeSixtyNxN : public CalypModuleIf, public CalypModuleInstance<ThreeSixtyNxN>
{
private:
  unsigned m_uiDownsampling;
  CalypFrame* m_pcOutputFrame;
  void downsamplingOperation( const CalypFrame* pcInputFrame );
  void upsamplingOperation( const CalypFrame* pcInputFrame );

public:
  ThreeSixtyNxN();
  virtual ~ThreeSixtyNxN() {}
  bool create( std::vector<CalypFrame*> apcFrameList );
  CalypFrame* process( std::vector<CalypFrame*> apcFrameList );
  void destroy();
};

#endif  // __THREESIXTYNXN_H__

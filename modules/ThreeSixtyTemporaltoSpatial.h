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
 * \file     ThreeSixtyTemporaltoSpatial.h
 * \brief    Convert 360 videos from temporal to spatial
 */

#ifndef __THREESIXTYTEMPORALTOSPATIAL_H__
#define __THREESIXTYTEMPORALTOSPATIAL_H__

// CalypLib
#include "lib/CalypModuleIf.h"

class ThreeSixtyTemporaltoSpatial : public CalypModuleIf
{
  REGISTER_CLASS_FACTORY( ThreeSixtyTemporaltoSpatial )

private:
  unsigned int m_uiFacesX;
  unsigned int m_uiFacesY;
  unsigned int m_uiCopyX;
  unsigned int m_uiCopyY;
  unsigned int m_uiFacesCount;

public:
  ThreeSixtyTemporaltoSpatial();
  virtual ~ThreeSixtyTemporaltoSpatial() {}
  bool create( std::vector<CalypFrame*> apcFrameList );
  bool needFrame();
  CalypFrame* process( std::vector<CalypFrame*> apcFrameList );
  bool flush();
  void destroy();
};

#endif  // __THREESIXTYTEMPORALTOSPATIAL_H__

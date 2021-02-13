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
 * \file     FrameShift.h
 * \brief    Shift image
 */

#ifndef __FRAMESHIFT_H__
#define __FRAMESHIFT_H__

// CalypLib
#include "lib/CalypModuleIf.h"

class FrameShift : public CalypModuleIf
{
  REGISTER_CLASS_FACTORY( FrameShift )

private:
  CalypFrame* m_pcProcessedFrame;
  int m_iShiftHor;
  int m_iShiftVer;

public:
  FrameShift();
  virtual ~FrameShift() {}
  bool create( std::vector<CalypFrame*> apcFrameList );
  CalypFrame* process( std::vector<CalypFrame*> apcFrameList );
  bool keyPressed( enum Module_Key_Supported value );
  void destroy();
};

#endif  // __FrameShift_H__

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
 * \file     FrameBinarization.h
 * \brief    Binarize frame module
 */

#ifndef __FRAMEBINARIZATION_H__
#define __FRAMEBINARIZATION_H__

// CalypLib
#include "lib/CalypModuleIf.h"

class FrameBinarization : public CalypModuleIf
{
  REGISTER_CLASS_FACTORY( FrameBinarization )

private:
  CalypFrame* m_pcBinFrame;
  unsigned int m_uiThreshold;

public:
  FrameBinarization();
  virtual ~FrameBinarization() {}
  void create( CalypFrame* );
  CalypFrame* process( CalypFrame* );
  void destroy();
};

#endif  // __FRAMEBINARIZATION_H__

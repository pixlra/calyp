/*    This file is a part of plaYUVer project
 *    Copyright (C) 2014-2015  by Luis Lucas      (luisfrlucas@gmail.com)
 *                                Joao Carreira   (jfmcarreira@gmail.com)
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
 * \file     SaliencyBasedFiltering.h
 */

#ifndef __SALIENCYBASEDFILTERING_H__
#define __SALIENCYBASEDFILTERING_H__

#include <cassert>

#include "lib/PlaYUVerModuleIf.h"

class SaliencyBasedFiltering : public PlaYUVerModuleIf
{
  REGISTER_CLASS_FACTORY( SaliencyBasedFiltering )

private:
  PlaYUVerFrame* m_pcProcessedFrame;

public:
  SaliencyBasedFiltering();
  virtual ~SaliencyBasedFiltering()
  {
  }
  Bool create( std::vector<PlaYUVerFrame*> apcFrameList );
  PlaYUVerFrame* process( std::vector<PlaYUVerFrame*> apcFrameList );
  Void destroy();
};

#endif  // __SALIENCYBASEDFILTERING_H__

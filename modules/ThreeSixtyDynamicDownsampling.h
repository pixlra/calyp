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
 * \file     ThreeSixtyDynamicDownsampling.h
 */

#ifndef __ThreeSixtyDynamicDownsampling_H__
#define __ThreeSixtyDynamicDownsampling_H__

// CalypLib
#include "lib/CalypModuleIf.h"

struct ClpFaceProperty
{
  unsigned id;
  unsigned x;
  unsigned y;
  int rot;
  unsigned ratio;
};
struct ClpVideoFPStruct
{
  unsigned rows;
  unsigned cols;
  int numFaces;
  ClpFaceProperty faces[60];
};

class ThreeSixtyDynamicDownsampling : public CalypModuleIf, public CalypModuleInstance<ThreeSixtyDynamicDownsampling>
{
private:
  std::string m_arrayInputDefinitionString;
  std::string m_arrayOutputDefinitionString;
  ClpVideoFPStruct m_cInputFPSStruct;
  ClpVideoFPStruct m_cOutputFPSStruct;
  unsigned m_uiFaceWidth;
  unsigned m_uiFaceHeight;

  CalypFrame* m_pcOutputFrame;

public:
  ThreeSixtyDynamicDownsampling();
  virtual ~ThreeSixtyDynamicDownsampling() {}
  bool create( std::vector<CalypFrame*> apcFrameList );
  CalypFrame* process( std::vector<CalypFrame*> apcFrameList );
  void destroy();
};

#endif  // __ThreeSixtyDynamicDownsampling_H__

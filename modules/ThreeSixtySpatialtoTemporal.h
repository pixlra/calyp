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
 * \file     ThreeSixtySpatialtoTemporal.h
 * \brief    Convert 360 videos from spatial to temporal
 */

#ifndef __THREESIXTYSPATIALTOTEMPORAL_H__
#define __THREESIXTYSPATIALTOTEMPORAL_H__

// CalypLib
#include "lib/CalypModuleIf.h"

class ThreeSixtySpatialtoTemporal : public CalypModuleIf
{
  REGISTER_CLASS_FACTORY( ThreeSixtySpatialtoTemporal )

private:
  std::unique_ptr<CalypFrame> m_pcTempSpatialFrame;
  std::unique_ptr<CalypFrame> m_pcTmpInputFrame;
  std::unique_ptr<CalypFrame> m_pcTmpFace;
  bool m_uiSpatial2Temporal;
  unsigned int m_uiFacesX;
  unsigned int m_uiFacesY;
  unsigned int m_uiFacesPerFrame;
  unsigned int m_uiFacesCount;
  unsigned int m_uiCopyX;
  unsigned int m_uiCopyY;

public:
  ThreeSixtySpatialtoTemporal();
  virtual ~ThreeSixtySpatialtoTemporal() {}
  bool create( std::vector<CalypFrame*> apcFrameList ) override;
  CalypFrame* getProcessedFrame() override;
  bool needFrame() override;
  CalypFrame* process( std::vector<CalypFrame*> apcFrameList ) override;
  bool flush() override;
  void destroy() override;
};

#endif  // __THREESIXTYSPATIALTOTEMPORAL_H__

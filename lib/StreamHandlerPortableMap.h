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
 * \file     StreamHandlerPortableMap.h
 * \ingroup  CalypStreamGrp
 * \brief    Handling portable pixmap formats
 */

#ifndef __STREAMHANDLERPORTABLEMAP_H__
#define __STREAMHANDLERPORTABLEMAP_H__

#include "CalypStreamHandlerIf.h"

class StreamHandlerPortableMap : public CalypStreamHandlerIf
{
  REGISTER_CALYP_STREAM_HANDLER( StreamHandlerPortableMap )

public:
  StreamHandlerPortableMap() { m_pchHandlerName = "PortableMaps"; }
  ~StreamHandlerPortableMap() {}
  bool openHandler( ClpString strFilename, bool bInput );
  void closeHandler();
  bool configureBuffer( const CalypFrame& pcFrame );
  bool seek( ClpULong iFrameNum );
  bool read( CalypFrame& pcFrame );
  bool write( const CalypFrame& pcFrame );

private:
  FILE* m_pFile; /**< The input file pointer >*/
  int m_iMagicNumber;
  int m_iMaxValue;
};

#endif  // __STREAMHANDLERPORTABLEMAP_H__

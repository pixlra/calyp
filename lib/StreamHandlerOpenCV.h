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
 * \file     StreamHandlerOpenCV.h
 * \ingroup  CalypStreamGrp
 * \brief    interface with opencv lib
 */

#ifndef __STREAMHANDLEROPENCV_H__
#define __STREAMHANDLEROPENCV_H__

#include "CalypStreamHandlerIf.h"

namespace cv
{
class VideoCapture;
}

class StreamHandlerOpenCV : public CalypStreamHandlerIf
{
  REGISTER_CALYP_STREAM_HANDLER( StreamHandlerOpenCV )

public:
  StreamHandlerOpenCV();
  ~StreamHandlerOpenCV() {}
  bool openHandler( std::string strFilename, bool bInput );
  void closeHandler();
  bool configureBuffer( const CalypFrame& pcFrame );
  bool seek( std::uint64_t iFrameNum );
  bool read( CalypFrame& pcFrame );
  bool write( const CalypFrame& pcFrame );

private:
  std::string m_cFilename;
  cv::VideoCapture* m_pcVideoCapture;
};

#endif  // __STREAMHANDLEROPENCV_H__

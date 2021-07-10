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
 * \file     CalypDefs.h
 * \ingroup  CalypLibGrp
 * \brief    Define basic types, new types and enumerations
 */

#ifndef __CALYPDEFS_H__
#define __CALYPDEFS_H__

#include <iterator>
#include <memory>
#include <string>

using ClpByte = std::uint8_t;
using ClpPel = std::uint16_t;
using ClpULong = std::uint64_t;
using ClpString = std::string;

enum CLP_LOG_LEVEL
{
  CLP_LOG_INFO = 1,
  CLP_LOG_WARNINGS = 2,
  CLP_LOG_RESULT = 3,
  CLP_LOG_ERROR = 4,
};

/**
 * \enum CalypColorSpace
 * \brief List of supported color spaces
 * \ingroup CalypLibGrp
 */
enum CalypColorSpace
{
  CLP_COLOR_INVALID = -1,  //!< Invalid
  CLP_COLOR_YUV = 0,       //!< YUV
  CLP_COLOR_RGB = 1,       //!< RGB
  CLP_COLOR_GRAY = 2,      //!< Grayscale
  CLP_COLOR_RGBA = 3,      //!< RGB + Alpha
  CLP_COLOR_MAX = 255,     //!< Account for future formats
};

/**
 * \enum CalypPixelFormats
 * \brief List of supported pixel formats
 * \ingroup CalypLibGrp
 */
enum CalypPixelFormats
{
  CLP_INVALID_FMT = -1,  //!< Invalid
  CLP_YUV420P = 0,       //!< YUV 420 progressive
  CLP_YUV422P,           //!< YUV 422 progressive
  CLP_YUV444P,           //!< YUV 444 progressive
  CLP_YUYV422,           //!< YUV 422 interleaved
  CLP_GRAY,              //!< Grayscale
  CLP_RGB24P,            //!< RGB 32 bpp progressive
  CLP_RGB24,             //!< RGB 32 bpp
  CLP_BGR24,             //!< BGR 32 bpp
  CLP_RGBA32,            //!< RGBA 32 bpp
  CLP_BGRA32,            //!< BGRA 32 bpp
  CLP_MAX_FMT = 255,     //!< Account for future formats
};

enum CLP_YUV_Components
{
  CLP_LUMA = 0,
  CLP_CHROMA_U,
  CLP_CHROMA_V,
};

enum CLP_RGB_Components
{
  CLP_COLOR_R = 0,
  CLP_COLOR_G,
  CLP_COLOR_B,
  CLP_COLOR_A,
};

enum CLP_Endianness
{
  CLP_BIG_ENDIAN = 0,
  CLP_LITTLE_ENDIAN = 1,
};

static const double S_PI = 3.14159265358979323846;
static const double S_PI_2 = 1.57079632679489661923;

inline ClpString clpLowercase( const ClpString& in )
{
  ClpString out;
  transform( in.begin(), in.end(), std::back_inserter( out ), tolower );
  return out;
}

inline ClpString clpUppercase( const ClpString& in )
{
  ClpString out;
  transform( in.begin(), in.end(), std::back_inserter( out ), toupper );
  return out;
}

struct CalypFailure : public std::exception
{
  ClpString m_class_name;
  ClpString m_error_msg;
  CalypFailure( ClpString error_msg ) throw()
      : m_error_msg( error_msg ) {}
  CalypFailure( ClpString class_name, ClpString error_msg ) throw()
      : m_class_name( class_name ), m_error_msg( error_msg )
  {
  }
  ~CalypFailure() throw() {}
  const char* what() const throw()
  {
    ClpString* msg = new ClpString( "[" + m_class_name + "] " + m_error_msg );
    return msg->c_str();
  }
};

#endif  // __CALYPDEFS_H__

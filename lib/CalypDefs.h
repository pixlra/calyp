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

#include <algorithm>
#include <iterator>
#include <memory>
#include <string>

enum CLP_LOG_LEVEL
{
  CLP_LOG_INFO = 1,
  CLP_LOG_WARNINGS = 2,
  CLP_LOG_RESULT = 3,
  CLP_LOG_ERROR = 4,
};

static const double S_PI = 3.14159265358979323846;
static const double S_PI_2 = 1.57079632679489661923;

inline std::string clpLowercase( const std::string& in )
{
  std::string out;
  std::transform( in.begin(), in.end(), std::back_inserter( out ), tolower );
  return out;
}

inline std::string clpUppercase( const std::string& in )
{
  std::string out;
  std::transform( in.begin(), in.end(), std::back_inserter( out ), toupper );
  return out;
}

struct CalypFailure : public std::exception
{
  std::string m_class_name;
  std::string m_error_msg;
  CalypFailure( std::string error_msg ) throw()
      : m_error_msg( error_msg ) {}
  CalypFailure( std::string class_name, std::string error_msg ) throw()
      : m_class_name( class_name ), m_error_msg( error_msg )
  {
  }
  ~CalypFailure() throw() {}
  const char* what() const throw()
  {
    std::string* msg = new std::string( "[" + m_class_name + "] " + m_error_msg );
    return msg->c_str();
  }
};

#endif  // __CALYPDEFS_H__

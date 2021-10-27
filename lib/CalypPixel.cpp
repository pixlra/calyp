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
 * \file     CalypFrame.cpp
 * \brief    Video Frame handling
 */

#include <algorithm>
#include <cassert>
#include <memory>
#include <numeric>

#include "CalypFrame.h"
#include "PixelFormats.h"
#include "config.h"

constexpr auto kMinPixelValue{ 0 };
constexpr auto kMaxPixelValue{ 255 };

static inline void yuvToRgb( const int& iY, const int& iU, const int& iV, int& iR, int& iG, int& iB )
{
  iR = std::clamp( iY + ( ( 1436 * ( iV - 128 ) ) >> 10 ), kMinPixelValue, kMaxPixelValue );                      // NOLINT
  iG = std::clamp( iY - ( ( 352 * ( iU - 128 ) + 731 * ( iV - 128 ) ) >> 10 ), kMinPixelValue, kMaxPixelValue );  // NOLINT
  iB = std::clamp( iY + ( ( 1812 * ( iU - 128 ) ) >> 10 ), kMinPixelValue, kMaxPixelValue );                      // NOLINT
}

static inline void rgbToYuv( const int& iR, const int& iG, const int& iB, int& iY, int& iU, int& iV )
{
  iY = ( 299 * iR + 587 * iG + 114 * iB + 500 ) / 1000;  // NOLINT
  iU = ( 1000 * ( iB - iY ) + 226816 ) / 1772;           // NOLINT
  iV = ( 1000 * ( iR - iY ) + 179456 ) / 1402;           // NOLINT
}

CalypPixel::CalypPixel( const int ColorSpace, const ClpPel c0 )
{
  m_colorSpace = ColorSpace;
  m_pelComp[0] = c0;
}

CalypPixel::CalypPixel( const int ColorSpace, const ClpPel c0, const ClpPel c1, const ClpPel c2 )
{
  m_colorSpace = ColorSpace;
  m_pelComp[0] = c0;
  m_pelComp[1] = c1;
  m_pelComp[2] = c2;
}

CalypPixel::CalypPixel( const int ColorSpace, const ClpPel c0, const ClpPel c1, const ClpPel c2, const ClpPel c3 )
{
  m_colorSpace = ColorSpace;
  m_pelComp[0] = c0;
  m_pelComp[1] = c1;
  m_pelComp[2] = c2;
  m_pelComp[3] = c3;
}

CalypPixel& CalypPixel::operator+=( const CalypPixel& in )
{
  assert( in.colorSpace() == m_colorSpace );
  const auto& other_comp = in.components();
  std::transform( other_comp.begin(),
                  other_comp.end(),
                  m_pelComp.begin(),
                  m_pelComp.begin(),
                  []( auto& p1, auto& p2 ) { return p1 + p2; } );
  return *this;
}

CalypPixel& CalypPixel::operator-=( const CalypPixel& in )
{
  assert( in.colorSpace() == m_colorSpace );
  const auto& other_comp = in.components();
  std::transform( other_comp.begin(),
                  other_comp.end(),
                  m_pelComp.begin(),
                  m_pelComp.begin(),
                  []( auto& p1, auto& p2 ) { return p1 - p2; } );
  return *this;
}

CalypPixel& CalypPixel::operator*=( const double op )
{
  std::transform( m_pelComp.begin(),
                  m_pelComp.end(),
                  m_pelComp.begin(),
                  [&op]( auto& p1 ) { return p1 * op; } );
  return *this;
}

CalypPixel CalypPixel::operator+( const CalypPixel& in ) const
{
  assert( in.colorSpace() == m_colorSpace );
  const auto& other_comp = in.components();
  CalypPixel result{ m_colorSpace };
  std::transform( other_comp.begin(),
                  other_comp.end(),
                  m_pelComp.begin(),
                  result.components().begin(),
                  []( auto& p1, auto& p2 ) { return p1 + p2; } );
  return result;
}

CalypPixel CalypPixel::operator-( const CalypPixel& in ) const
{
  assert( in.colorSpace() == m_colorSpace );
  const auto& other_comp = in.components();
  CalypPixel result{ m_colorSpace };
  std::transform( other_comp.begin(),
                  other_comp.end(),
                  m_pelComp.begin(),
                  result.components().begin(),
                  []( auto& p1, auto& p2 ) { return p1 - p2; } );
  return result;
}

CalypPixel CalypPixel::operator*( const double op ) const
{
  CalypPixel result{ m_colorSpace };
  auto& comp = result.components();
  std::transform( comp.begin(),
                  comp.end(),
                  comp.begin(),
                  [&op]( auto& p1 ) { return p1 * op; } );
  return result;
}

auto CalypPixel::operator==( const CalypPixel& other ) const -> bool
{
  return colorSpace() == other.colorSpace() &&
         std::equal( m_pelComp.begin(), m_pelComp.end(), other.components().begin() );
}

std::ostream& operator<<( std::ostream& os, const CalypPixel& p )
{
  if( p.colorSpace() == CLP_COLOR_RGB || p.colorSpace() == CLP_COLOR_YUV )
  {
    os << "(" << p[0] << ", " << p[1] << ", " << p[2] << ")";
  }
  else
  {
    os << "(" << p[0] << ", " << p[1] << ", " << p[2] << ", " << p[3] << ")";
  }
  return os;
}

CalypPixel CalypPixel::convertPixel( CalypColorSpace eOutputSpace ) const
{
  if( m_colorSpace == eOutputSpace )
    return *this;

  int outA = 0;
  int outB = 0;
  int outC = 0;
  int outD = 0;
  if( m_colorSpace == CLP_COLOR_YUV )
  {
    switch( eOutputSpace )
    {
    case CLP_COLOR_GRAY:
      outA = m_pelComp[0];
      break;
    case CLP_COLOR_RGBA:
      outD = kMaxPixelValue;
    case CLP_COLOR_RGB:
      yuvToRgb( m_pelComp[0], m_pelComp[1], m_pelComp[2], outA, outB, outC );
      break;
    default:
      break;
    }
  }
  if( m_colorSpace == CLP_COLOR_RGB )
  {
    switch( eOutputSpace )
    {
    case CLP_COLOR_GRAY:
      rgbToYuv( m_pelComp[0], m_pelComp[1], m_pelComp[2], outA, outB, outC );
    case CLP_COLOR_YUV:
      rgbToYuv( m_pelComp[0], m_pelComp[1], m_pelComp[2], outA, outB, outC );
      break;
    case CLP_COLOR_RGBA:
      outD = kMaxPixelValue;
      break;
    default:
      break;
    }
  }
  return CalypPixel( eOutputSpace, outA, outB, outC, outD );
}

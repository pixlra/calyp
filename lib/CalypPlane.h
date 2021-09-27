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
 * \file     CalypPlane.h
 * \ingroup	 CalypFrameGrp
 * \brief    Video Frame Plane handling
 */

#ifndef __CALYPPLANE_H__
#define __CALYPPLANE_H__

#include <span>
#include <vector>

#include "CalypDefs.h"

/**
 * \class    CalypPlane
 * \ingroup	 CalypLibGrp CalypPlaneGrp
 * \brief    Frame handling class
 */
template <typename T>
class CalypPlane
{
public:
  CalypPlane() noexcept = default;
  CalypPlane( std::size_t width, std::size_t height ) noexcept { resize( width, height ); }

  ~CalypPlane() noexcept = default;
  CalypPlane( const CalypPlane& buffer ) noexcept = default;
  CalypPlane( CalypPlane&& buffer ) noexcept = default;
  auto operator=( const CalypPlane& buffer ) -> CalypPlane& = default;
  auto operator=( CalypPlane&& buffer ) noexcept -> CalypPlane& = default;

  auto operator[]( std::size_t row ) const& noexcept -> const std::span<T>& { return m_rows[row]; }
  auto operator[]( std::size_t row ) & noexcept -> std::span<T>& { return m_rows[row]; }

  auto operator[]( std::size_t row ) const&& noexcept { return m_rows[row]; }
  auto operator[]( std::size_t row ) && noexcept { return m_rows[row]; }

  auto data() noexcept { return std::span<T>( m_data ); }
  auto data() const noexcept { return std::span<const T>( m_data ); }

  void resize( std::size_t width, std::size_t height ) noexcept
  {
    m_data.resize( width * height );
    m_rows.resize( height );
    for( std::size_t y = 0; y < height; y++ )
    {
      m_rows[y] = std::span<T>( m_data ).subspan( y * width, width );
    }
  }

private:
  std::vector<T> m_data;
  std::vector<std::span<T>> m_rows;
};

#endif  // __CALYPPLANE_H__

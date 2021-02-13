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
 * \file     CalypLib.h
 * \brief    Inlcude all modules of Calyp Lib
 */

#ifndef __CALYPLIB_H_
#define __CALYPLIB_H_

/**
 * @defgroup CalypLibGrp Calyp Lib
 * @{
 *
 * CalypLib is an independent module that provide the low level interface
 * for dealing with streams and frames.
 *
 * Two main interfaces are defining CalypFrame and CalypStream.
 * Moreover handler for external libs, as FFmpeg and OpenCv are provided
 * which enables Calyp to boost its functionality using external libs.
 *
 * This guaratees that the top-level applications does not need to worry
 * about dealig with different stream formats as well as different pixel formats
 *
 * @defgroup CalypFrameGrp Calyp Frame
 * @{
 *  Describe Calyp Frame format
 * @}
 *
 * @defgroup CalypStreamGrp Calyp Stream
 * @{
 *  Describe Calyp Stream format
 * @}
 *
 * @}
 *
 */

#include "CalypFrame.h"
#include "CalypStream.h"

#endif  // __CALYPLIB_H_

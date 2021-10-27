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
 * \file     CalypOpenCVModuleIf.cpp
 * \brief    Calyp modules interface for OpenCV
 */

#include "CalypFrame.h"
#include "CalypModuleIf.h"

// OpenCV
#include <opencv2/opencv.hpp>

#include "PixelFormats.h"

bool CalypOpenCVModuleIf::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_
  bool bRet = false;
  std::vector<Mat> acMatList( apcFrameList.size() );
  for( unsigned i = 0; i < apcFrameList.size(); i++ )
  {
    apcFrameList[i]->toMat( acMatList[i], m_bConvertToGray );
  }
  Mat* outputMat = create_using_opencv( acMatList );
  if( outputMat )
  {
    auto pelFormat = m_bConvertToGray ? ClpPixelFormats::Gray : apcFrameList[0]->getPelFormat();
    m_pcOutputFrame = std::make_unique<CalypFrame>( outputMat->cols, outputMat->rows, pelFormat, apcFrameList[0]->getBitsPel() );
    bRet = true;
  }
  else
  {
    return false;
  }
  return bRet;
}

CalypFrame* CalypOpenCVModuleIf::process( std::vector<CalypFrame*> apcFrameList )
{
  m_pcOutputFrame->reset();
  std::vector<Mat> acMatList( apcFrameList.size() );
  for( unsigned i = 0; i < apcFrameList.size(); i++ )
  {
    apcFrameList[i]->toMat( acMatList[i], m_bConvertToGray );
  }
  Mat* cvOutput = process_using_opencv( acMatList );
  m_pcOutputFrame->fromMat( *cvOutput );
  return m_pcOutputFrame.get();
}

void CalypOpenCVModuleIf::destroy()
{
}

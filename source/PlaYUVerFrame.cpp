/*    This file is a part of plaYUVer project
 *    Copyright (C) 2014  by plaYUVer developers
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
 * \file     PlaYUVerFrame.cpp
 * \brief    Video Frame handling
 */
#include <cstdio>

#include "PlaYUVerFrame.h"
#include "LibMemAlloc.h"

#include <QImage>

namespace plaYUVer
{

QStringList PlaYUVerFrame::supportedPixelFormatList()
{
  QStringList formats;
  formats << "yuv420p"
      << "yuv444p";
     /* << "yuv422p"
      << "yuv410p"
      << "yuv411p"
      << "gray"
      << "rgb8"; */
  return formats;
}

PlaYUVerFrame::PlaYUVerFrame( UInt width, UInt height, Int pel_format )
{
  m_pppcInputPel[0] = NULL;
  m_pppcInputPel[1] = NULL;
  m_pppcInputPel[2] = NULL;
  m_pppcRGBPel = NULL;

  m_uiWidth = width;
  m_uiHeight = height;
  m_iPixelFormat = pel_format;

  m_bHasRGBPel = false;

  switch( m_iPixelFormat )
  {
  case YUV420p:
    get_mem2Dpel( &( m_pppcInputPel[0] ), m_uiHeight, m_uiWidth );
    get_mem2Dpel( &( m_pppcInputPel[1] ), m_uiHeight / 2, m_uiWidth / 2 );
    get_mem2Dpel( &( m_pppcInputPel[2] ), m_uiHeight / 2, m_uiWidth / 2 );
    break;
  case YUV444p:
    get_mem2Dpel( &( m_pppcInputPel[0] ), m_uiHeight, m_uiWidth );
    get_mem2Dpel( &( m_pppcInputPel[1] ), m_uiHeight, m_uiWidth );
    get_mem2Dpel( &( m_pppcInputPel[2] ), m_uiHeight, m_uiWidth );
    break;
  case YUV422p:
    get_mem2Dpel( &( m_pppcInputPel[0] ), m_uiHeight, m_uiWidth );
    get_mem2Dpel( &( m_pppcInputPel[1] ), m_uiHeight / 2, m_uiWidth );
    get_mem2Dpel( &( m_pppcInputPel[2] ), m_uiHeight / 2, m_uiWidth );
    break;
  case YUV410p:
  case YUV411p:
    get_mem2Dpel( &( m_pppcInputPel[0] ), m_uiHeight, m_uiWidth );
    get_mem2Dpel( &( m_pppcInputPel[1] ), m_uiHeight, m_uiWidth );
    get_mem2Dpel( &( m_pppcInputPel[2] ), m_uiHeight, m_uiWidth );
    break;
  case GRAY:
    get_mem2Dpel( &( m_pppcInputPel[0] ), m_uiHeight, m_uiWidth );
    break;
  case RGB8:
    get_mem3Dpel( &m_pppcRGBPel, 3, m_uiHeight, m_uiWidth );
    break;
  }
  if( m_iPixelFormat != RGB8 )
    get_mem3Dpel( &m_pppcRGBPel, 3, m_uiHeight, m_uiWidth );

}

PlaYUVerFrame::~PlaYUVerFrame()
{
  if( m_pppcInputPel[0] )
    free_mem2Dpel( m_pppcInputPel[0] );
  if( m_pppcInputPel[1] )
    free_mem2Dpel( m_pppcInputPel[1] );
  if( m_pppcInputPel[2] )
    free_mem2Dpel( m_pppcInputPel[2] );

  if( m_pppcRGBPel )
    free_mem3Dpel( m_pppcRGBPel );
}


template<typename T>
Void yuvToRgb( T iY, T iU, T iV, T &iR, T &iG, T &iB )
{
  iR = iY + 1402 * iV / 1000;
  iG = iY - ( 101004 * iU + 209599 * iV ) / 293500;
  iB = iY + 1772 * iU / 1000;

  if( iR < 0 )
    iR = 0;
  if( iG < 0 )
    iG = 0;
  if( iB < 0 )
    iB = 0;

  if( iR > 255 )
    iR = 255;
  if( iG > 255 )
    iG = 255;
  if( iB > 255 )
    iB = 255;
}

template<typename T>
Void rgbToyuv( T iR, T iG, T iB, T &iY, T &iU, T &iV )
{
  iY = 0;
  iU = 0;
  iV = 0;
}

Void PlaYUVerFrame::FrametoRGB8()
{
  if( m_bHasRGBPel )
    return;

  Int iY, iU, iV, iR, iG, iB;
  Pel** ppInputPelY = m_pppcInputPel[0];
  Pel* pInputPelY = &(m_pppcInputPel[0][0][0]);
  Pel* pInputPelU = &(m_pppcInputPel[1][0][0]);
  Pel* pInputPelV = &(m_pppcInputPel[2][0][0]);
  Pel* pRGBPelR = &(m_pppcRGBPel[0][0][0]);
  Pel* pRGBPelG = &(m_pppcRGBPel[1][0][0]);
  Pel* pRGBPelB = &(m_pppcRGBPel[2][0][0]);

  switch( m_iPixelFormat )
  {
  case YUV420p:
    for( UInt y = 0; y < m_uiHeight; y += 2 )
      for( UInt x = 0; x < m_uiWidth; x += 2 )
      {
        // Pixel (x, y).
        iY = ppInputPelY[y][x];
        iU = *pInputPelU++;
        iV = *pInputPelV++;
        iU -= 128;
        iV -= 128;
        yuvToRgb( iY, iU, iV, iR, iG, iB );
        m_pppcRGBPel[0][y][x] = iR;
        m_pppcRGBPel[1][y][x] = iG;
        m_pppcRGBPel[2][y][x] = iB;
        // Pixel (x+1, y)
        iY = ppInputPelY[y][x+1];
        yuvToRgb( iY, iU, iV, iR, iG, iB );
        m_pppcRGBPel[0][y][x + 1] = iR;
        m_pppcRGBPel[1][y][x + 1] = iG;
        m_pppcRGBPel[2][y][x + 1] = iB;
        // Pixel (x, y+1)
        iY = ppInputPelY[y+1][x];
        yuvToRgb( iY, iU, iV, iR, iG, iB );
        m_pppcRGBPel[0][y + 1][x] = iR;
        m_pppcRGBPel[1][y + 1][x] = iG;
        m_pppcRGBPel[2][y + 1][x] = iB;
        // Pixel (x+1, y+1)
        iY = ppInputPelY[y+1][x+1];
        yuvToRgb( iY, iU, iV, iR, iG, iB );
        m_pppcRGBPel[0][y + 1][x + 1] = iR;
        m_pppcRGBPel[1][y + 1][x + 1] = iG;
        m_pppcRGBPel[2][y + 1][x + 1] = iB;
      }
    break;
  case YUV444p:
    for( UInt y = 0; y < m_uiHeight; y++ )
      for( UInt x = 0; x < m_uiWidth; x++ )
      {
        iY = *pInputPelY++;
        iU = *pInputPelU++;
        iV = *pInputPelV++;
        iU -= 128;
        iV -= 128;
        yuvToRgb( iY, iU, iV, iR, iG, iB );
        *pRGBPelR++ = iR;
        *pRGBPelG++ = iG;
        *pRGBPelB++ = iB;
      }
    break;
  case YUV422p:
  case YUV410p:
  case YUV411p:
  case GRAY:
  case RGB8:
  default:
    // No action.
    break;
  }
  m_bHasRGBPel = true;
}

Void PlaYUVerFrame::FrameFromBuffer( Pel *input_buffer, Int pel_format )
{
  if( m_iPixelFormat != pel_format )
  {
    return;
  }
  UInt64 frame_bytes_input = m_uiWidth * m_uiHeight;
  switch( m_iPixelFormat )
  {
  case YUV420p:
    memcpy( &m_pppcInputPel[0][0][0], input_buffer + frame_bytes_input * 0, frame_bytes_input * sizeof(Pel) );
    memcpy( &m_pppcInputPel[1][0][0], input_buffer + frame_bytes_input * 1, frame_bytes_input / 4 * sizeof(Pel) );
    memcpy( &m_pppcInputPel[2][0][0], input_buffer + frame_bytes_input * 5 / 4, frame_bytes_input / 4 * sizeof(Pel) );
    break;
  case YUV444p:
    memcpy( &m_pppcInputPel[0][0][0], input_buffer + frame_bytes_input * 0, frame_bytes_input * sizeof(Pel) );
    memcpy( &m_pppcInputPel[1][0][0], input_buffer + frame_bytes_input * 1, frame_bytes_input * sizeof(Pel) );
    memcpy( &m_pppcInputPel[2][0][0], input_buffer + frame_bytes_input * 2, frame_bytes_input * sizeof(Pel) );
    break;
  case YUV422p:
  case YUV410p:
  case YUV411p:
  case GRAY:
  case RGB8:
    break;
  }
  m_bHasRGBPel = false;
}

Void PlaYUVerFrame::CopyFrom( PlaYUVerFrame* input_frame )
{
  if( m_iPixelFormat != input_frame->getPelFormat() )
  {
    return;
  }
  UInt64 num_bytes = m_uiWidth * m_uiHeight;
  switch( m_iPixelFormat )
  {
  case YUV420p:
    memcpy( &m_pppcInputPel[0][0][0], &( input_frame->getPelBufferYUV()[0][0][0] ), num_bytes * sizeof(Pel) );
    memcpy( &m_pppcInputPel[1][0][0], &( input_frame->getPelBufferYUV()[1][0][0] ), num_bytes / 4 * sizeof(Pel) );
    memcpy( &m_pppcInputPel[2][0][0], &( input_frame->getPelBufferYUV()[2][0][0] ), num_bytes / 4 * sizeof(Pel) );
    break;
  case YUV444p:
    memcpy( &m_pppcInputPel[0][0][0], &( input_frame->getPelBufferYUV()[0][0][0] ), num_bytes * sizeof(Pel) );
    memcpy( &m_pppcInputPel[1][0][0], &( input_frame->getPelBufferYUV()[1][0][0] ), num_bytes * sizeof(Pel) );
    memcpy( &m_pppcInputPel[2][0][0], &( input_frame->getPelBufferYUV()[2][0][0] ), num_bytes * sizeof(Pel) );
    break;
  case YUV422p:
  case YUV410p:
  case YUV411p:
  case GRAY:
  case RGB8:
    break;
  }
  m_bHasRGBPel = false;
}

QImage PlaYUVerFrame::getQimage()
{
  QImage img( m_uiWidth, m_uiHeight, QImage::Format_RGB888 );
  Pel*** bufferRGB = getPelBufferRGB();

  if( sizeof(Pel) == sizeof(unsigned char) )
  {
    for( UInt y = 0; y < m_uiHeight; y++ )
    {
      for( UInt x = 0; x < m_uiWidth; x++ )
      {
        img.setPixel( x, y, qRgb( bufferRGB[0][y][x], bufferRGB[1][y][x], bufferRGB[2][y][x] ) );
      }
    }
  }
  else
  {
    Q_ASSERT( 0 );
  }
  return img;
}

UInt64 PlaYUVerFrame::getBytesPerFrame()
{
  switch( m_iPixelFormat )
  {
  case YUV420p:
    return m_uiWidth * m_uiHeight * 1.5;
    break;
  case YUV444p:
    return m_uiWidth * m_uiHeight * 3;
    break;
  case YUV422p:
    return m_uiWidth * m_uiHeight * 2;
    break;
  case YUV410p:
    return m_uiWidth * m_uiHeight * 1.25;
    break;
  case GRAY:
    return m_uiWidth * m_uiHeight;
    break;
  case RGB8:
    return m_uiWidth * m_uiHeight * 3;
    break;
  default:
    return 0;
  }
  return 0;
}

Pixel PlaYUVerFrame::ConvertPixel( Pixel sInputPixel, ColorSpace eOutputSpace )
{
  Pixel sOutputPixel = {COLOR_INVALID,0,0,0};

  if( sInputPixel.color_space == eOutputSpace )
    return sInputPixel;

  if( eOutputSpace == COLOR_RGB )
  {
    yuvToRgb( sInputPixel.Luma, sInputPixel.ChromaU, sInputPixel.ChromaU,
        sOutputPixel.ColorR, sOutputPixel.ColorG, sOutputPixel.ColorB );
  }
  if( eOutputSpace == COLOR_YUV )
  {
    rgbToyuv( sInputPixel.ColorR, sInputPixel.ColorG, sInputPixel.ColorB,
        sOutputPixel.Luma, sOutputPixel.ChromaU, sOutputPixel.ChromaU );
  }
  return sOutputPixel;
}


Pixel PlaYUVerFrame::getPixelValue(const QPoint &pos, ColorSpace space)
{
  Pixel pixel_value;
  switch( m_iPixelFormat )
  {
  case YUV420p:
    pixel_value.color_space = COLOR_YUV;
    pixel_value.Luma = m_pppcInputPel[LUMA][pos.y()][pos.x()];
    pixel_value.ChromaU = m_pppcInputPel[CHROMA_U][pos.y()>>1][pos.x()>>1];
    pixel_value.ChromaV = m_pppcInputPel[CHROMA_V][pos.y()>>1][pos.x()>>1];
    break;
  case YUV444p:
    pixel_value.Luma = m_pppcInputPel[LUMA][pos.y()][pos.x()];
    pixel_value.ChromaU = m_pppcInputPel[CHROMA_U][pos.y()][pos.x()];
    pixel_value.ChromaV = m_pppcInputPel[CHROMA_V][pos.y()][pos.x()];
    break;
  case YUV422p:
    pixel_value.color_space = COLOR_YUV;
    pixel_value.Luma = m_pppcInputPel[LUMA][pos.y()][pos.x()];
    pixel_value.ChromaU = m_pppcInputPel[CHROMA_U][pos.y()][pos.x()>>1];
    pixel_value.ChromaV = m_pppcInputPel[CHROMA_V][pos.y()][pos.x()>>1];
    break;
  case YUV410p:
    pixel_value.color_space = COLOR_YUV;
    pixel_value.Luma = m_pppcInputPel[LUMA][pos.y()][pos.x()];
    pixel_value.ChromaU = m_pppcInputPel[CHROMA_U][pos.y()>>2][pos.x()>>2];
    pixel_value.ChromaV = m_pppcInputPel[CHROMA_V][pos.y()>>2][pos.x()>>2];
    break;
  case GRAY:
    pixel_value.color_space = COLOR_YUV;
    pixel_value.Luma = m_pppcInputPel[LUMA][pos.y()][pos.x()];
    pixel_value.ChromaU = 0;
    pixel_value.ChromaV = 0;
    break;
  case RGB8:
    pixel_value.color_space = COLOR_RGB;
    pixel_value.ColorR = m_pppcInputPel[COLOR_R][pos.y()][pos.x()];
    pixel_value.ColorG = m_pppcInputPel[COLOR_G][pos.y()][pos.x()];
    pixel_value.ColorB = m_pppcInputPel[COLOR_B][pos.y()][pos.x()];
    break;
  default:
    break;
  }
  ConvertPixel( pixel_value, space );
  return pixel_value;
}

}  // NAMESPACE

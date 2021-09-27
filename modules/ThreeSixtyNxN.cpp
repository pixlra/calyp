/*    This file is a part of Calyp project
 *    Copyright (C) 2014-2019  by Joao Carreira   (jfmcarreira@gmail.com)
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
 * \file     ThreeSixtyNxN.cpp
 * \brief    Downsampling ERP to NxN
 */

#include "ThreeSixtyNxN.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/photo.hpp>
#include <vector>

using cv::Rect;
using cv::Scalar;
using cv::Size;

ThreeSixtyNxN::ThreeSixtyNxN()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "360Video";
  m_pchModuleName = "ThreeSixtyNxN";
  m_pchModuleLongName = "ERP to NxN";
  m_pchModuleTooltip = "Downsampling ERP to NxN";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_SKIP_WHILE_PLAY |
                           CLP_MODULE_REQUIRES_OPTIONS;

  m_cModuleOptions.addOptions()                                                                     /**/
      ( "Downsampling", m_uiDownsampling, "ERP to NxN (0 for up, 1 for down and 2 for recon) [1]" ) /**/
      ;

  m_uiDownsampling = 1;
  m_pcOutputFrame = NULL;
}

bool ThreeSixtyNxN::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_
  m_uiDownsampling = m_uiDownsampling > 2 ? 1 : m_uiDownsampling;
  if( m_uiDownsampling == 0 )
  {
    m_pcOutputFrame = new CalypFrame( 2 * apcFrameList[0]->getHeight(), apcFrameList[0]->getHeight(), apcFrameList[0]->getPelFormat(), apcFrameList[0]->getBitsPel() );
  }
  else
  {
    m_pcOutputFrame = new CalypFrame( apcFrameList[0]->getHeight(), apcFrameList[0]->getHeight(), apcFrameList[0]->getPelFormat(), apcFrameList[0]->getBitsPel() );
  }

  return true;
}

#define WIDTH_TRIM_START_POINT 0.0
#define WIDTH_FINAL 1.0

//#define CLAMP8BITS( in ) ( uint8_t )( ( in > 255 ? 255 : in < 0 ? 0 : in ) + 0.5 )
#define CLAMP_RANGE( X, MIN, MAX ) ( ( X ) < MIN ? MIN : ( ( X ) > MAX ? MAX : X ) )
#define CIRCULAR( a, b ) ( ( ( a < 0 ? b : 0 ) + ( a % b ) ) % b )
#define PI 3.14159265358979323846264338327950288419716939937510
#define s60 0.8660254037844386467637231707529361834714026269051
#define s45 0.7071067811865475244008443621048490392848359376884

//interpolação Lanczos3 360
static inline ClpPel interpol_lanczos3( ClpPel* image, double x, int N, int max_value )
{
  static const double cs[][2] = {
      { 1, 0 },
      { -0.5, -s60 },
      { -0.5, s60 },
      { 1, 0 },
      { -0.5, -s60 },
      { -0.5, s60 },
  };

  double sum_coeffs = 0, sum = 0, coeffs[6];
  int x_int = floor( x );
  ClpPel xx[6];
  x -= x_int;
  bool n0 = x > 0;
  xx[2] = *( image + CIRCULAR( x_int, N ) );
  if( !n0 )
    return xx[2];
  xx[3] = *( image + CIRCULAR( x_int + n0, N ) );
  xx[1] = *( image + CIRCULAR( x_int - 1, N ) );
  xx[4] = *( image + CIRCULAR( x_int + n0 + 1, N ) );
  xx[0] = *( image + CIRCULAR( x_int - 2, N ) );
  xx[5] = *( image + CIRCULAR( x_int + n0 + 2, N ) );

  double aux = -2 - x, s_aux = sin( aux * PI / 3 ), c_aux = cos( aux * PI / 3 );

  for( int i = 0; i < 6; i++ )
    sum_coeffs += coeffs[i] = ( cs[i][0] * s_aux + cs[i][1] * c_aux ) / ( i - 2 - x ) / ( i - 2 - x );

  for( int i = 0; i < 6; i++ )
    sum += xx[i] * coeffs[i] / sum_coeffs;

  return CLAMP_RANGE( sum, 0, max_value );
}

//interpolação cúbica 360
static inline ClpPel interpol_cub( ClpPel* image, double x, int N, int max_value )
{
  int x_int = floor( x );
  x -= x_int;
  bool n0 = x > 0;
  ClpPel x1 = *( image + CIRCULAR( x_int, N ) );
  if( !n0 )
    return x1;
  ClpPel x2 = *( image + CIRCULAR( x_int + n0, N ) );
  ClpPel x0 = *( image + CIRCULAR( x_int - 1, N ) );
  ClpPel x3 = *( image + CIRCULAR( x_int + n0 + 1, N ) );

  ClpPel out = CLAMP_RANGE( x1 - x * ( x0 - x2 + x * ( 2 * x1 - 2 * x0 - x2 + x3 + x * ( x0 - x1 + x2 - x3 ) ) ), 0, max_value );  //cubic
  return out;
}

void ThreeSixtyNxN::downsamplingOperation( const CalypFrame* pcInputFrame )
{
  for( unsigned ch = 0; ch < m_pcOutputFrame->getNumberChannels(); ch++ )
  {
    int max_value = ( 1 << m_pcOutputFrame->getBitsPel() ) - 1;
    int N = pcInputFrame->getHeight( ch );
    ClpPel* in = &pcInputFrame->getPelBufferYUV()[ch][0][0];
    ClpPel* out = &m_pcOutputFrame->getPelBufferYUV()[ch][0][0];

    int n_2 = N / 2;
    int n1 = N - 1;
    double dist_y;
    double dist_x;
    double novo_x;
    double novo_y;
    double tx_x;
    ClpPel* tx_y;
    ClpPel* rx;
    bool valida;

    for( int y = 0; y < N; y++ )
    {
      for( int x = 0; x < N; x++ )
      {
        dist_y = ( abs( y * 2 - n1 ) - 1 ) / 2;
        dist_x = ( abs( x * 2 - n1 ) - 1 ) / 2;
        valida = ( n_2 - dist_x - dist_y - 1 ) >= 0;

        novo_y = ( valida ? y : ( y < n_2 ? dist_x : n1 - dist_x ) ) * 2;
        novo_x = ( valida ? x : ( x < n_2 ? -dist_y + x * 2 : n1 + dist_y - ( n1 - x ) * 2 ) ) - n_2 + 1;

        tx_x = ( N / ( N - fabs( novo_y - n1 ) ) ) * novo_x + n1;
        tx_y = (int)novo_y * N + in;
        rx = x + y * N + out;

        *( rx ) = interpol_lanczos3( tx_y, tx_x, N * 2, max_value );
      }
    }
  }
}

void ThreeSixtyNxN::upsamplingOperation( const CalypFrame* pcInputFrame )
{
  for( unsigned ch = 0; ch < m_pcOutputFrame->getNumberChannels(); ch++ )
  {
    int max_value = ( 1 << m_pcOutputFrame->getBitsPel() ) - 1;
    int N = pcInputFrame->getHeight( ch );
    ClpPel* in = &pcInputFrame->getPelBufferYUV()[ch][0][0];
    ClpPel* out = &m_pcOutputFrame->getPelBufferYUV()[ch][0][0];

    int x;
    int y;
    int n2 = 2 * N;
    double dist_y;
    double dist_x;
    double dist_Lj;
    double novo_x;
    double novo_y;
    double Lj;
    double Dj;
    double tx;
    ClpPel* rx;
    std::vector<ClpPel> linhaY( n2, 0 );

    for( y = 0; y < N; y++ )
    {
      Lj = ( N - abs( 2 * y - N + 1 ) ) * 2;
      Dj = n2 / Lj;

      for( x = 0; x < Lj; x++ )
      {
        dist_x = x - Lj / 2;
        dist_y = ( N - abs( 2 * y - N + 1 ) - 1 ) / 2;
        dist_Lj = ( Lj - abs( 2 * x - Lj + 1 ) - 1 ) / 2;

        novo_x = ( dist_x > dist_y ? dist_y : ( dist_x + 1 < -dist_y ? -dist_y - 1 : dist_x ) ) + N / 2;
        novo_y = abs( dist_x ) > dist_y ? ( y < N / 2 ? dist_Lj : N - dist_Lj - 1 ) : y;

        linhaY[x] = *( in + (int)novo_x + (int)novo_y * N );
      }

      for( x = 0; x < n2; x++ )
      {
        tx = ( x + 1 ) / Dj - 1;
        rx = x + y * n2 + out;

        *( rx ) = interpol_lanczos3( linhaY.data(), tx, Lj, max_value );
      }
    }
  }
}

CalypFrame* ThreeSixtyNxN::process( std::vector<CalypFrame*> apcFrameList )
{
  m_pcOutputFrame->reset();
  if( m_uiDownsampling == 1 || m_uiDownsampling == 2 )
  {
    downsamplingOperation( apcFrameList[0] );
  }
  if( m_uiDownsampling == 0 || m_uiDownsampling == 2 )
  {
    upsamplingOperation( m_uiDownsampling == 2 ? m_pcOutputFrame : apcFrameList[0] );
  }
  return m_pcOutputFrame;
}

void ThreeSixtyNxN::destroy()
{
  if( m_pcOutputFrame )
    delete m_pcOutputFrame;
  m_pcOutputFrame = NULL;
}

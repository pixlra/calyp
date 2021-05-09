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
 * \file     ThreeSixtyDownsampling.cpp
 * \brief    Adaptive Latitude Downsampling
 */

#include "ThreeSixtyDownsampling.h"

#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/photo.hpp>

//using cv::InputArray;
//using cv::OutputArray;
using cv::Rect;
using cv::Scalar;
using cv::Size;

ThreeSixtyDownsampling::ThreeSixtyDownsampling()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "360Video";
  m_pchModuleName = "ThreeSixtyDownsampling";
  m_pchModuleLongName = "Adaptive Latitude Downsampling";
  m_pchModuleTooltip = "Adaptive Latitude Downsampling";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_SKIP_WHILE_PLAY |
                           CLP_MODULE_REQUIRES_OPTIONS |
                           CLP_MODULES_HAS_INFO |
                           CLP_MODULE_USES_KEYS;

  m_cModuleOptions.addOptions()                                                                     /**/
      ( "Downsampling", m_uiDownsampling, "ERP to NxN (0 for up, 1 for down and 2 for recon) [1]" ) /**/
      ( "y0", m_uiY0, "Vertical anchor point ratio (0-100) [0]" )                                   /**/
      ( "x0", m_uiX0, "Horizontal anchor point ratio (0-100) [25%]" )                               /**/
      // INTER_NEAREST - a nearest-neighbor interpolation
      // INTER_LINEAR - a bilinear interpolation (used by default)
      // INTER_AREA - resampling using pixel area relation. It may be a preferred
      //              method for image decimation, as it gives moire’-free results. But when
      //              the image is zoomed, it is similar to the INTER_NEAREST method.
      // INTER_CUBIC - a bicubic interpolation over 4x4 pixel neighborhood
      // INTER_LANCZOS4 - a Lanczos interpolation over 8x8 pixel neighborhood
      ( "Interpolation", m_iInterpolation, "Interpolation method (1-4) [2]" )                         /**/
      ( "IntegerSlope", m_bForceIntSlope, "Force the downsampling slope to be integer (binary) [0]" ) /**/
      ( "Packing", m_iRearrange, "Rearrange image into a rectangle (0-3) [0]" )                       /**/
      ( "Inpainting", m_iInpaintMethod, "Use inpainting to fill empty pixels (0-2) [0]" )             /**/
      ( "UpsampWidth", m_iWidth, "Image width after upsampling [-1]" );

  m_uiDownsampling = 1;
  m_uiY0 = 0;
  m_uiX0 = 0;
  m_iInterpolation = 2;
  m_iInpaintMethod = 0;
  m_bForceIntSlope = 0;
  m_iRearrange = 0;
  m_iWidth = -1;
  m_pcOutputFrame = NULL;
  m_pcDownsampled = NULL;
  m_dPixelRatio = 0;

  //#ifdef DEBUG
  //  m_uiY0 = 25;
  //  m_uiX0 = 0;
  //  m_iInterpolation = 4;
  //  m_bForceIntSlope = 1;
  //  m_iRearrange = 1;
  //#endif
}

ClpString ThreeSixtyDownsampling::moduleInfo()
{
  ClpString info( "" );
  info += "Interpolation: ";
  switch( m_iInterpolation )
  {
  case cv::INTER_NEAREST:
    info += "Nearest";
    break;
  case cv::INTER_LINEAR:
    info += "Linear";
    break;
  case cv::INTER_AREA:
    info += "Area";
    break;
  case cv::INTER_CUBIC:
    info += "Cubic";
    break;
  case cv::INTER_LANCZOS4:
    info += "Lanczos4";
    break;
  default:
    assert( 0 );
  }
  info += "\ny0 = " + std::to_string( m_uiY0 ) + "\nx0 = " + std::to_string( m_uiX0 );
  if( m_bForceIntSlope == 0 )
    info += "\nNo integer slope";
  else
    info += "\nUisng integer slope";
  info += "\nPixel ratio = " + std::to_string( m_dPixelRatio );
  return info;
}

bool ThreeSixtyDownsampling::create( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_

  m_uiDownsampling = m_uiDownsampling > 2 ? 1 : m_uiDownsampling;

  if( !createDownsamplingMask( apcFrameList[0] ) )
  {
    return false;
  }

  switch( m_iInterpolation )
  {
  case 2:
    m_iInterpolation = cv::INTER_LINEAR;
    break;
  case 3:
    m_iInterpolation = cv::INTER_CUBIC;
    break;
  case 4:
    m_iInterpolation = cv::INTER_LANCZOS4;
    break;
  default:
    m_iInterpolation = cv::INTER_NEAREST;
  }

  switch( m_iInpaintMethod )
  {
  case 1:
    m_iInterpolation = cv::INPAINT_NS;
    break;
  case 2:
    m_iInpaintMethod = cv::INPAINT_TELEA;
    break;
  }
  return true;
}

#define WIDTH_TRIM_START_POINT 0.0
#define WIDTH_FINAL 1.0

bool ThreeSixtyDownsampling::createDownsamplingMask( CalypFrame* pcInputFrame )
{
  if( m_iRearrange == 3 )
  {
    m_uiX0 = 0;
    m_uiY0 = std::ceil( 100.0 / 3.0 );
  }

  if( m_iRearrange == 4 )
  {
    m_uiX0 = 0;
    m_uiY0 = m_uiY0 > 37 ? 40 : 34;
  }

  m_iHeight = -1;
  if( m_uiDownsampling == 0 )
  {
    if( m_iWidth == -1 )
    {
      // Guess the output dimensions when upsampling
      switch( m_iRearrange )
      {
      case 1:  // Cannot get this - user input needed
        break;
      case 2:
        m_iWidth = pcInputFrame->getWidth();
        break;
      }
    }
    m_iHeight = m_iWidth / 2;
  }
  else
  {
    m_iWidth = pcInputFrame->getWidth();
    m_iHeight = pcInputFrame->getHeight();
  }
  if( m_iWidth == -1 || m_iHeight == -1 )  // output dimensions are needed
    return false;

  //m_pcDownsampled = new CalypFrame( m_iWidth, m_uiHeight, pcInputFrame->getPelFormat(), pcInputFrame->getBitsPel() );
  m_pcDownsampled = new CalypFrame( m_iWidth, m_iHeight, CalypFrame::findPixelFormat( "GRAY" ), pcInputFrame->getBitsPel() );

  m_isOdd = !( m_iHeight % 2 );
  for( unsigned ch = 0; ch < m_pcDownsampled->getNumberChannels(); ch++ )
  {
    Mat* downsamplingMaskPtr = new Mat( m_pcDownsampled->getHeight( ch ), m_pcDownsampled->getWidth( ch ), CV_8UC1, Scalar( 0 ) );
    Mat downsamplingMask = *downsamplingMaskPtr;
    Mat_<Point> initialReshapePoints( m_pcDownsampled->getHeight( ch ), m_pcDownsampled->getWidth( ch ), Point( -1, -1 ) );

    int imH = m_pcDownsampled->getHeight( ch );
    int imN = imH / 2;

    // Find lines
    double theta = atan2( imN, imH - WIDTH_TRIM_START_POINT - WIDTH_FINAL );
    double y0 = std::ceil( float( int( m_uiY0 ) * imH ) / 200.0 );
    double x0 = std::round( float( int( m_uiX0 ) * imH ) / 100.0 );
    double xl = 0;
    double slope2 = 0;

    if( m_iRearrange == 3 || m_iRearrange == 4 )
    {
      y0 = std::ceil( y0 / 64.0 ) * 64;
    }

    Point p0;
    Point p1;
    Point p2;

    if( m_uiY0 > 0 )
    {
      double x0min = ( 1 + y0 / tan( theta ) - imH + WIDTH_TRIM_START_POINT ) * double( imN - y0 ) / y0 + y0 / tan( theta );

      x0 = std::max( x0, x0min );
      xl = double( imH - WIDTH_TRIM_START_POINT ) - y0 / tan( theta ) - y0 * y0 / double( imN - y0 ) * ( 1.0 / tan( theta ) - x0 / y0 );
      xl = std::floor( xl );
      if( m_iRearrange == 3 )
        xl = imH / 2;
      else if( m_iRearrange == 4 && m_uiY0 == 34 )
        xl = imH / 2;
      else if( m_iRearrange == 4 && m_uiY0 == 40 )
        xl = imH / 3;

      slope2 = xl / ( double( imN ) - y0 );
      if( m_bForceIntSlope )
        slope2 = std::round( slope2 );

      // calculate the final coordinates of point corresponding to y0
      // p0 corresponds to a coordinate system with reference ate left-most equator
      p0 = Point( int( ( double( y0 + 1 ) - double( imH ) / 2.0 ) * slope2 + double( imH - WIDTH_FINAL ) ), y0 );
      // give the y value of the triangle which can be used to fill the gap
      if( m_iRearrange == 2 )
      {
        int y1 = double( -p0.x + WIDTH_FINAL ) / slope2 + imN;
        p1 = Point( int( ( double( y1 + 1 ) - double( imH ) / 2.0 ) * slope2 + double( imH - WIDTH_FINAL ) ), y1 );
        p2 = Point( -1, ( imH - p1.y ) / 2 );
      }
    }
    else
    {
      slope2 = 2;
      p0 = Point( 0, 0 );
      p1 = Point( -1, imN );
    }

    // Reset mask
    downsamplingMask = Scalar( 0 );

    for( int line = 0; line < imN; line++ )
    {
      for( int hemisfer = -1; hemisfer < 2; hemisfer += 2 )
      {
        // calculate destination sizes
        int ypos = ( ( imH + ( m_isOdd ? hemisfer : 0 ) ) >> 1 ) + line * hemisfer;
        int lineWidth = 0;

        if( m_uiY0 == 0 )
        {
          lineWidth = ( 2 * imH - 2 ) - 4 * line;
        }
        else
        {
          double lp = 0;
          if( line < y0 )
            lp = line * x0 / y0;
          else
            lp = ( line + 1 - imN ) * slope2 + double( imH - WIDTH_FINAL );
          lp = std::round( lp );
          assert( imH >= lp );
          lineWidth = 2 * imH - 2 * lp;
        }
        assert( lineWidth > 0 );
        int xpos = imH - lineWidth / 2;  // N - WIDTH/2

        // create mask and init reshape matrix
        Mat downsamplingMaskRow = downsamplingMask.row( ypos );
        Mat_<Point> reshapePointsRow = initialReshapePoints.row( ypos );
        for( int x = xpos; x < xpos + lineWidth; x++ )
        {
          *( downsamplingMaskRow.data + x ) = ch == 0 ? 255 : 128;
          reshapePointsRow( x ) = Point( x, ypos );
        }
      }
    }
    if( m_iRearrange > 0 )
    {
      // initialReshapePoints -> Original coordinates without re-arrange
      // intermediateReshapePoints -> Coordinates with re-arrange <- Set this

      Mat_<Point> intermediateReshapePoints;

      int imH = int( m_pcDownsampled->getHeight( ch ) );
      int imN = imH / 2;

      // Strecth into wide rectangle
      if( m_iRearrange == 1 )
      {
        intermediateReshapePoints = Mat_<Point>( initialReshapePoints.rows, initialReshapePoints.cols, Point( -1, -1 ) );
        Size maskSize( imH - p0.x, imN - p0.y );
        for( int hemisfer = -1; hemisfer < 2; hemisfer += 2 )
        {
          // copy center
          Point r0_pt( 0, imN );
          Point r0_size( 2 * imH, hemisfer * p0.y );
          initialReshapePoints( Rect( r0_pt, r0_pt + r0_size ) ).copyTo( intermediateReshapePoints( Rect( r0_pt, r0_pt + r0_size ) ) );

          // main triangle
          Mat currMask;
          downsamplingMask( Rect( Point( p0.x, 0 ), maskSize ) ).copyTo( currMask );
          if( hemisfer == 1 )
            cv::flip( currMask, currMask, 0 );  // Filp vertically

          initialReshapePoints( Rect( Point( p0.x, hemisfer == -1 ? 0 : imN + p0.y ), maskSize ) )
              .copyTo( intermediateReshapePoints( Rect( Point( 1, hemisfer == -1 ? 0 : imN + p0.y ), maskSize ) ), currMask );

          // rotated triangle
          Mat tmpTriangle;
          initialReshapePoints( Rect( Point( imH, hemisfer == -1 ? 0 : imN + p0.y ), maskSize ) ).copyTo( tmpTriangle );
          cv::flip( tmpTriangle, tmpTriangle, 0 );

          cv::flip( currMask, currMask, 1 );  // Filp horizontaly
          cv::flip( currMask, currMask, 0 );
          tmpTriangle.copyTo( intermediateReshapePoints( Rect( Point( 0, hemisfer == -1 ? 0 : imN + p0.y ), maskSize ) ), currMask );
          tmpTriangle.release();
        }
        // At this point the two triangle were merged into a rectangle
        if( ( ( imN - p0.y ) % p0.y ) == 0 )  // check if we can divide the rectangles into equally heighted slices
        {
          int numberRectSlices = ( imN - p0.y ) / p0.y;
          int rectSlicesWidth = 2 * imN - p0.x + 1;
          // create extended  reshapePoints
          Mat_<Point> extendedReshapePoints( 2 * p0.y, intermediateReshapePoints.cols + rectSlicesWidth * numberRectSlices, Point( -1, -1 ) );
          intermediateReshapePoints( Rect( Point( 0, imN - p0.y ), Size( intermediateReshapePoints.cols, 2 * p0.y ) ) )
              .copyTo( extendedReshapePoints( Rect( Point( 0, 0 ), Size( intermediateReshapePoints.cols, 2 * p0.y ) ) ) );
          for( int hemisfer = -1; hemisfer < 2; hemisfer += 2 )
          {
            Point initSize( rectSlicesWidth, hemisfer * p0.y );
            Point finalPoint = Point( 0, p0.y ) + Point( 2 * imH, 0 );
            for( int i = 0; i < numberRectSlices; i++ )
            {
              Point initPoint = Point( 0, imN + hemisfer * ( p0.y + i * p0.y ) );
              intermediateReshapePoints( Rect( initPoint, initPoint + initSize ) ).copyTo( extendedReshapePoints( Rect( finalPoint, finalPoint + initSize ) ) );
              finalPoint += Point( rectSlicesWidth, 0 );
            }
          }
          intermediateReshapePoints.release();
          extendedReshapePoints.copyTo( intermediateReshapePoints );
          extendedReshapePoints.release();
        }
      }
      // Rotate triangles into a rectangle and spread the top small triangles
      else if( m_iRearrange == 2 )
      {
        intermediateReshapePoints = Mat_<Point>( initialReshapePoints.rows, initialReshapePoints.cols, Point( -1, -1 ) );
        for( int hemisfer = -1; hemisfer < 2; hemisfer += 2 )
        {
          if( p0.x < imN )
          {
            for( int side = -1; side < 2; side += 2 )
            {
              // center
              Point r0_pt( imH, imN );
              Point r0_size( side * imH, hemisfer * p0.y );
              initialReshapePoints( Rect( r0_pt, r0_pt + r0_size ) ).copyTo( intermediateReshapePoints( Rect( r0_pt, r0_pt + r0_size ) ) );

              // copy center unmodified
              Point r1_pt = r0_pt + Point( 0, hemisfer * p0.y );
              Point r1_size( side * imH, hemisfer * ( p1.y - p0.y ) / 2 );
              Rect mainRoi( r1_pt, r1_pt + r1_size );
              initialReshapePoints( mainRoi ).copyTo( intermediateReshapePoints( mainRoi ), downsamplingMask( mainRoi ) );

              // rotated polygon
              Point r2_pt = r1_pt + Point( 0, hemisfer * ( p1.y - p0.y ) / 2 );
              Mat_<Point> auxPointMap;
              Mat auxMask;
              initialReshapePoints( Rect( r2_pt, r2_pt + r1_size ) ).copyTo( auxPointMap );
              downsamplingMask( Rect( r2_pt, r2_pt + r1_size ) ).copyTo( auxMask );
              cv::flip( auxPointMap, auxPointMap, -1 );
              cv::flip( auxMask, auxMask, -1 );
              auxPointMap.copyTo( intermediateReshapePoints( mainRoi ), auxMask );
              auxPointMap.release();
              auxMask.release();

              // top triangle
              // the top triangle is converted to a rectangle and then reshape into and new rectangle with a larger width
              Point r34_size( side * ( imH - p1.x + 1 ), hemisfer * ( imN - p1.y ) / 2 );
              Point r3_pt( imH, imN + hemisfer * p1.y );
              Point r4_pt = r3_pt + Point( 0, r34_size.y );

              Point r3l_pt( r3_pt.x, r2_pt.y );

              Mat_<Point> topTriangleReshape;
              initialReshapePoints( Rect( r3_pt, r3_pt + r34_size ) ).copyTo( topTriangleReshape );

              // rotate the top of the top triangle to form a rectangle
              initialReshapePoints( Rect( r4_pt, r4_pt + r34_size ) ).copyTo( auxPointMap );
              downsamplingMask( Rect( r4_pt, r4_pt + r34_size ) ).copyTo( auxMask );

              cv::flip( auxPointMap, auxPointMap, -1 );
              cv::flip( auxMask, auxMask, -1 );

              //cv::imwrite( "auxMask.png", auxMask );
              auxPointMap.copyTo( topTriangleReshape, auxMask );

              long shape_area = std::abs( r34_size.x ) * std::abs( r34_size.y );
              int reshapedWidth = imH;
              while( 1 )
              {
                if( !( shape_area % reshapedWidth ) )
                  break;
                reshapedWidth--;
              }
              int reshaped_height = shape_area / reshapedWidth;
              if( shape_area % reshapedWidth )
              {
                return false;
              }
              topTriangleReshape = topTriangleReshape.reshape( 1, reshaped_height );
              topTriangleReshape.copyTo( intermediateReshapePoints( Rect( r3l_pt, r3l_pt + Point( side * topTriangleReshape.cols, hemisfer * topTriangleReshape.rows ) ) ) );
            }
          }
          else
          {
            return false;
          }
        }
      }
      else if( m_iRearrange == 3 )
      {
        intermediateReshapePoints = Mat_<Point>( 8 * p0.y, initialReshapePoints.cols, Point( -1, -1 ) );
        for( int hemisfer = -1; hemisfer < 2; hemisfer += 2 )
        {
          Point r_pt;
          Point r_size;
          Point r_pt_final;
          Mat currMask;

          // copy half of rectangle 6 and 5
          r_pt = Point( p0.x, imN );
          r_size = Point( 2 * ( imH - p0.x ), hemisfer * p0.y );
          r_pt_final = Point( p0.x, 3 * p0.y );
          initialReshapePoints( Rect( r_pt, r_pt + r_size ) ).copyTo( intermediateReshapePoints( Rect( r_pt_final, r_pt_final + r_size ) ) );

          // copy triangle 2 or 4
          r_pt = Point( imH, imN + hemisfer * p0.y );
          r_size = Point( -( imH - p0.x ), hemisfer * ( imN - p0.y ) );
          r_pt_final = Point( imH, 3 * p0.y + hemisfer * p0.y );
          downsamplingMask( Rect( r_pt, r_pt + r_size ) ).copyTo( currMask );
          initialReshapePoints( Rect( r_pt, r_pt + r_size ) ).copyTo( intermediateReshapePoints( Rect( r_pt_final, r_pt_final + r_size ) ), currMask );
          currMask.release();

          // copy triangle 3 or 1
          r_pt = Point( imH, imN - hemisfer * p0.y );
          r_size = Point( imH - p0.x, -1 * hemisfer * ( imN - p0.y ) );
          r_pt_final = Point( p0.x, hemisfer == -1 ? 0 : 6 * p0.y );

          downsamplingMask( Rect( r_pt, r_pt + r_size ) ).copyTo( currMask );
          initialReshapePoints( Rect( r_pt, r_pt + r_size ) ).copyTo( intermediateReshapePoints( Rect( r_pt_final, r_pt_final + r_size ) ), currMask );

          // copy rectangle 7 and 8
          r_pt = Point( imH - hemisfer * ( imH - p0.x ), imN - p0.y );
          r_size = Point( -hemisfer * p0.x, p0.y * 2 );
          if( hemisfer == -1 )
            r_pt_final = Point( imH, 0 );
          else
            r_pt_final = Point( 2 * imH - p0.x, 4 * p0.y );
          initialReshapePoints( Rect( r_pt, r_pt + r_size ) ).copyTo( intermediateReshapePoints( Rect( r_pt_final, r_pt_final + r_size ) ) );
        }
      }
      else if( m_iRearrange == 4 )
      {
        intermediateReshapePoints = Mat_<Point>( 10 * p0.y, initialReshapePoints.cols, Point( -1, -1 ) );
        //unsigned hor_pad = 0;
        unsigned ver_pad = 0;
        if( m_uiY0 == 40 )
        {
          if( unsigned( 4 * p0.y + ( imH - p0.y ) ) % 2 )
          {
            ver_pad = 1;
          }
        }
        else
        {
          if( unsigned( 6 * p0.y + ( imH - p0.y ) ) % 2 )
          {
            ver_pad = 1;
          }
        }
        for( int side = -1; side < 2; side += 2 )
        {
          Point r_pt;
          Point r_size;
          Point r_pt_final;
          Mat currMask;

          // copy half of rectangle 6 and 5
          r_pt = Point( p0.x, imN - p0.y );
          r_size = Point( 2 * ( imH - p0.x ), 2 * p0.y );
          r_pt_final = Point( p0.x, imN - p0.y );
          initialReshapePoints( Rect( r_pt, r_pt + r_size ) ).copyTo( intermediateReshapePoints( Rect( r_pt_final, r_pt_final + r_size ) ) );

          // copy triangle 2 or 1
          r_pt = Point( imH, imN - p0.y );
          r_size = Point( side * ( imH - p0.x ), -( imN - p0.y ) );
          r_pt_final = Point( imH, imN - p0.y );
          downsamplingMask( Rect( r_pt, r_pt + r_size ) ).copyTo( currMask );
          initialReshapePoints( Rect( r_pt, r_pt + r_size ) ).copyTo( intermediateReshapePoints( Rect( r_pt_final, r_pt_final + r_size ) ), currMask );
          currMask.release();

          // copy triangle 4 or 3
          r_pt = Point( imH, imN + p0.y );
          r_size = Point( side * ( imH - p0.x ), imN - p0.y );
          r_pt_final = Point( imH - side * ( imH - p0.x ), 0 );
          downsamplingMask( Rect( r_pt, r_pt + r_size ) ).copyTo( currMask );
          initialReshapePoints( Rect( r_pt, r_pt + r_size ) ).copyTo( intermediateReshapePoints( Rect( r_pt_final, r_pt_final + r_size ) ), currMask );

          // copy rectangle 7 and 8
          if( m_uiY0 == 40 )
          {
            r_pt = Point( imH + side * ( imH - p0.x ), imN - p0.y );
            r_size = Point( side * p0.x, p0.y * 2 );
            r_pt_final = Point( imH - side * ( imH - p0.x ), ver_pad + imN + ( side == -1 ? p0.y : 3 * p0.y ) );
            if( side == -1 )
            {
              Mat flipRect;
              cv::flip( initialReshapePoints( Rect( r_pt, r_pt + r_size ) ), flipRect, -1 );
              flipRect.copyTo( intermediateReshapePoints( Rect( r_pt_final, r_pt_final + r_size ) ) );
            }
            else
            {
              initialReshapePoints( Rect( r_pt, r_pt + r_size ) ).copyTo( intermediateReshapePoints( Rect( r_pt_final, r_pt_final + r_size ) ) );
            }
          }
          else
          {
            r_pt = Point( imH + side * ( imH - p0.x ), imN - p0.y );
            r_size = Point( side * p0.x, p0.y * 2 );
            r_pt_final = Point( imH - side * p0.x, imN + p0.y + ver_pad );
            initialReshapePoints( Rect( r_pt, r_pt + r_size ) ).copyTo( intermediateReshapePoints( Rect( r_pt_final, r_pt_final + r_size ) ) );
          }
        }
      }
      initialReshapePoints.release();
      intermediateReshapePoints.copyTo( initialReshapePoints );  // copy back to the initial Mat
    }

    // Create a mask with valid reshape points in order to create the final rectangular frame
    Mat reshapePointsMask( initialReshapePoints.rows, initialReshapePoints.cols, CV_8UC1, Scalar( 0 ) );
    m_dPixelRatio = 0;
    for( int y = 0; y < initialReshapePoints.rows; y++ )
      for( int x = 0; x < initialReshapePoints.cols; x++ )
        if( initialReshapePoints.at<Point>( y, x ).x != -1 )
        {
          reshapePointsMask.at<uchar>( y, x ) = 1;
          m_dPixelRatio++;
        }
    m_dPixelRatio = m_dPixelRatio / double( pcInputFrame->getWidth() * pcInputFrame->getHeight() );

    Rect reshapeArea = boundingRect( reshapePointsMask );
    Mat_<Point>* finalReshapePointsPtr = new Mat_<Point>( reshapeArea.height, reshapeArea.width );
    initialReshapePoints( reshapeArea ).copyTo( *finalReshapePointsPtr );

    m_cvDownsamplingMask.push_back( downsamplingMaskPtr );  // Save Mat's for each channel
    m_cvReshapePoints.push_back( finalReshapePointsPtr );
  }
  switch( m_uiDownsampling )
  {
  case 0:
    m_pcOutputFrame = new CalypFrame( m_pcDownsampled->getWidth(), m_pcDownsampled->getHeight(), pcInputFrame->getPelFormat(), pcInputFrame->getBitsPel() );
    break;
  case 1:
    m_pcOutputFrame = new CalypFrame( m_cvReshapePoints[0]->cols, m_cvReshapePoints[0]->rows, pcInputFrame->getPelFormat(), pcInputFrame->getBitsPel() );
    break;
  case 2:
    m_pcOutputFrame = new CalypFrame( pcInputFrame );
    m_pcOutputFrame->reset();
    break;
  }
  return true;
}

void ThreeSixtyDownsampling::downsamplingOperation( CalypFrame* pcInputFrame )
{
  unsigned numBytes = m_pcDownsampled->getBitsPel() > 8 ? 2 : 1;
  m_pcDownsampled->reset();
  m_pcOutputFrame->reset();
  for( unsigned ch = 0; ch < m_pcDownsampled->getNumberChannels(); ch++ )
  {
    int imH = int( m_pcDownsampled->getHeight( ch ) );

    Mat cvInputFrame;
    pcInputFrame->toMat( cvInputFrame, true, false, ch );
    for( int y = 0; y < imH; y++ )
    {
      // calculate destination sizes
      int lineWidth = cv::countNonZero( m_cvDownsamplingMask[ch]->row( y ) );
      int xpos = imH - lineWidth / 2;  // (2*HEIGHT - WIDTH)/2

      Mat cvDownsample( 1, lineWidth, numBytes > 2 ? CV_16UC1 : CV_8UC1 );
      cv::resize( cvInputFrame.row( y ), cvDownsample, cvDownsample.size(), 0, 0, m_iInterpolation );

      // copy to output
      ClpPel* outpel = &m_pcDownsampled->getPelBufferYUV()[ch][y][xpos];
      unsigned char* cv_data = cvDownsample.data;
      for( int i = 0; i < lineWidth; i++ )
      {
        *outpel = 0;
        for( unsigned b = 0; b < numBytes; b++ )
        {
          *outpel = ClpPel( *outpel + ( *cv_data++ << ( 8 * b ) ) );
        }
        outpel++;
      }
    }
    Mat_<Point> reshapePoints = *( m_cvReshapePoints[ch] );
    ClpPel** downSampPelBuff = m_pcDownsampled->getPelBufferYUV()[ch];
    ClpPel* pelOutputPtr = m_pcOutputFrame->getPelBufferYUV()[ch][0];

    Mat outputMask( m_pcOutputFrame->getHeight( ch ), m_pcOutputFrame->getWidth( ch ), CV_8UC1, Scalar( 1 ) );
    for( unsigned y = 0; y < m_pcOutputFrame->getHeight( ch ); y++ )
    {
      for( unsigned x = 0; x < m_pcOutputFrame->getWidth( ch ); x++ )
      {
        Point pt = reshapePoints.at<Point>( y, x );
        if( pt.x > -1 )
        {
          *pelOutputPtr = downSampPelBuff[pt.y][pt.x];
          outputMask.at<unsigned char>( y, x ) = 0;
        }
        pelOutputPtr++;
      }
    }
    if( m_iInpaintMethod )
    {
      Mat outputFrameUnfilled, outputFrame;
      m_pcOutputFrame->toMat( outputFrameUnfilled, true, false, ch );
      cv::inpaint( outputFrameUnfilled, outputMask, outputFrame, 16, m_iInpaintMethod );
      m_pcOutputFrame->fromMat( outputFrame, ch );
    }
  }
}

void ThreeSixtyDownsampling::upsamplingOperation( CalypFrame* pcInputFrame )
{
  unsigned numBytes = m_pcDownsampled->getBitsPel() > 8 ? 2 : 1;
  m_pcDownsampled->reset();
  m_pcOutputFrame->reset();
  for( unsigned ch = 0; ch < m_pcDownsampled->getNumberChannels(); ch++ )
  {
    Mat_<Point> reshapePoints = *( m_cvReshapePoints[ch] );
    ClpPel** downSampPelBuff = m_pcDownsampled->getPelBufferYUV()[ch];
    ClpPel* pelInputPtr = pcInputFrame->getPelBufferYUV()[ch][0];
    for( unsigned y = 0; y < pcInputFrame->getHeight( ch ); y++ )
    {
      for( unsigned x = 0; x < pcInputFrame->getWidth( ch ); x++ )
      {
        Point pt = reshapePoints.at<Point>( y, x );
        if( pt.x > -1 )
          downSampPelBuff[pt.y][pt.x] = *( pelInputPtr );
        pelInputPtr++;
      }
    }

    Mat cvDownsampled;
    m_pcDownsampled->toMat( cvDownsampled, true, false, ch );
    int imH = m_pcOutputFrame->getHeight( ch );
    Mat cvUpsample( 1, 2 * imH, numBytes > 2 ? CV_16UC1 : CV_8UC1 );
    for( int y = 0; y < imH; y++ )
    {
      unsigned lineWidth = cv::countNonZero( m_cvDownsamplingMask[ch]->row( y ) );
      unsigned int xpos = imH - lineWidth / 2;  // N - WIDTH/2

      // resize
      cv::resize( cvDownsampled.row( y ).colRange( xpos, xpos + lineWidth ), cvUpsample, cvUpsample.size(), 0, 0, m_iInterpolation );

      // copy to output
      ClpPel* outpel = &m_pcOutputFrame->getPelBufferYUV()[ch][y][0];
      unsigned char* cv_data = cvUpsample.data;
      for( int i = 0; i < 2 * imH; i++ )
      {
        *outpel = 0;
        for( unsigned b = 0; b < numBytes; b++ )
        {
          *outpel = ClpPel( *outpel + ( *cv_data++ << ( 8 * b ) ) );
        }
        outpel++;
      }
    }
  }
  //m_pcOutputFrame->copyFrom( m_pcDownsampled );
}

CalypFrame* ThreeSixtyDownsampling::process( std::vector<CalypFrame*> apcFrameList )
{
  m_pcDownsampled->reset();
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

bool ThreeSixtyDownsampling::keyPressed( enum Module_Key_Supported value )
{
  bool bRet = false;
  if( value == MODULE_KEY_LEFT )
  {
    m_uiX0 = m_uiX0 == 0 ? 0 : m_uiX0 - 1;
  }
  else if( value == MODULE_KEY_RIGHT )
  {
    m_uiX0 = m_uiX0 == 60 ? 60 : m_uiX0 + 1;
  }
  else if( value == MODULE_KEY_DOWN )
  {
    m_uiY0 = m_uiY0 == 0 ? 0 : m_uiY0 - 1;
  }
  else if( value == MODULE_KEY_UP )
  {
    m_uiY0 = m_uiY0 == 60 ? 60 : m_uiY0 + 1;
  }
  if( bRet )
  {
    bRet = createDownsamplingMask( m_pcDownsampled );
  }
  return bRet;
}

void ThreeSixtyDownsampling::destroy()
{
  if( m_pcDownsampled )
    delete m_pcDownsampled;
  m_pcDownsampled = NULL;
  if( m_pcOutputFrame )
    delete m_pcOutputFrame;
  m_pcOutputFrame = NULL;
}

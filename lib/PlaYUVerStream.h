/*    This file is a part of plaYUVer project
 *    Copyright (C) 2014-2017  by Luis Lucas      (luisfrlucas@gmail.com)
 *                                Joao Carreira   (jfmcarreira@gmail.com)
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
 * \file     PlaYUVerStream.h
 * \brief    Input stream handling
 */

#ifndef __PLAYUVERSTREAM_H__
#define __PLAYUVERSTREAM_H__

#include "PlaYUVerDefs.h"

class PlaYUVerFrame;
class PlaYUVerStreamHandlerIf;

typedef struct
{
  String shortName;
  UInt uiWidth;
  UInt uiHeight;
} PlaYUVerStdResolution;

typedef PlaYUVerStreamHandlerIf* ( *CreateStreamHandlerFn )( void );

typedef struct
{
  String formatName;
  String formatExt;
  String formatPattern;
  CreateStreamHandlerFn formatFct;
} PlaYUVerSupportedFormat;

#define INI_REGIST_PLAYUVER_SUPPORTED_FMT           \
  std::vector<PlaYUVerSupportedFormat> formatsList; \
  PlaYUVerSupportedFormat formatElem;

#define END_REGIST_PLAYUVER_SUPPORTED_FMT return formatsList;

#define REGIST_PLAYUVER_SUPPORTED_FMT( handler, name, ext ) \
  formatElem.formatName = name;                             \
  formatElem.formatExt = lowercase( ext );                  \
  formatElem.formatFct = handler;                           \
  formatsList.push_back( formatElem );

#define REGIST_PLAYUVER_SUPPORTED_ABSTRACT_FMT( handler, name, pattern ) \
  formatElem.formatPattern = pattern;                                    \
  REGIST_PLAYUVER_SUPPORTED_FMT( handler, name, "" )

#define APPEND_PLAYUVER_SUPPORTED_FMT( class_name, fct )                                   \
  {                                                                                        \
    std::vector<PlaYUVerSupportedFormat> new_fmts = class_name::supported##fct##Formats(); \
    formatsList.insert( formatsList.end(), new_fmts.begin(), new_fmts.end() );             \
  }

/**
 * \class PlaYUVerStream
 * \ingroup  PlaYUVerLib PlaYUVerLib_Stream
 * \brief  Stream handling class
 */
class PlaYUVerStream
{
public:
  static std::vector<PlaYUVerSupportedFormat> supportedReadFormats();
  static std::vector<PlaYUVerSupportedFormat> supportedWriteFormats();

  static CreateStreamHandlerFn findStreamHandler( String strFilename, bool bRead );

  static std::vector<PlaYUVerStdResolution> stdResolutionSizes();

  PlaYUVerStream();
  ~PlaYUVerStream();

  enum PlaYUVerStreamErrors
  {
    NO_STREAM_ERROR = 0,
    READING,
    WRITING,
    LAST_FRAME,
    END_OF_SEQ,
  };

  String getFormatName();
  String getCodecName();

  Bool open( String filename,
             String resolution,
             String input_format,
             UInt bitsPel,
             Int endianness,
             UInt frame_rate,
             Bool bInput );
  Bool open( String filename,
             UInt width,
             UInt height,
             Int input_format,
             UInt bitsPel,
             Int endianness,
             UInt frame_rate,
             Bool bInput );
  Bool reload();
  Void close();

  String getFileName();
  UInt getFrameNum();
  UInt getWidth() const;
  UInt getHeight() const;
  Int getEndianess() const;
  Int getCurrFrameNum();
  Double getFrameRate();
  Void getFormat( UInt& rWidth,
                  UInt& rHeight,
                  Int& rInputFormat,
                  UInt& rBitsPerPel,
                  Int& rEndianness,
                  UInt& rFrameRate );

  Void loadAll();

  Void readFrame();
  Void readFrameFillRGBBuffer();
  Void writeFrame();
  Void writeFrame( PlaYUVerFrame* pcFrame );

  Bool saveFrame( const String& filename );
  static Bool saveFrame( const String& filename, PlaYUVerFrame* saveFrame );

  Bool setNextFrame();
  PlaYUVerFrame* getCurrFrame();
  PlaYUVerFrame* getCurrFrame( PlaYUVerFrame* );
  PlaYUVerFrame* getNextFrame();

  Bool seekInputRelative( Bool bIsFoward );
  Bool seekInput( UInt64 new_frame_num );

  Bool isInit() { return m_bInit; }
  Void getDuration( Int* duration_array );

private:
  Bool m_bInit;
  CreateStreamHandlerFn m_pfctCreateHandler;
  PlaYUVerStreamHandlerIf* m_pcHandler;

  Bool m_bIsInput;
  String m_cFilename;
  UInt64 m_uiTotalFrameNum;
  Int64 m_iCurrFrameNum;

  Bool m_bLoadAll;

  UInt m_uiFrameBufferSize;
  PlaYUVerFrame** m_ppcFrameBuffer;
  PlaYUVerFrame* m_pcCurrFrame;
  PlaYUVerFrame* m_pcNextFrame;
  UInt m_uiFrameBufferIndex;
  UInt64 m_uiCurrFrameFileIdx;
};

#endif  // __PLAYUVERSTREAM_H__

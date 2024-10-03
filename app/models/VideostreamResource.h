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
 * \file     VideostreamResource.h
 * \brief    Class to control video playback
 */

#ifndef __VIDEOSTREAMRESOURCE_H__
#define __VIDEOSTREAMRESOURCE_H__

#include <QElapsedTimer>
#include <QWidget>

#include "CalypResource.h"
#include "CommonDefs.h"
#include "config.h"
#include "lib/CalypStream.h"

struct CalypFileInfo
{
  QString m_cFilename;
  unsigned int m_uiWidth;
  unsigned int m_uiHeight;
  ClpPixelFormats m_iPelFormat;
  unsigned int m_uiBitsPelPixel{ 8 };
  int m_iEndianness{ CLP_LITTLE_ENDIAN };
  unsigned int m_uiFrameRate{ 30 };
  unsigned long long int m_uiFileSize;
  bool m_bForceRaw{ false };
};

using CalypFileInfoVector = QVector<CalypFileInfo>;

auto operator<<( QDataStream& out, const CalypFileInfoVector& infos ) -> QDataStream&;
auto operator>>( QDataStream& in, CalypFileInfoVector& infos ) -> QDataStream&;
auto findCalypStreamInfo( const CalypFileInfoVector& array, const QString& filename ) -> int;

class VideostreamResource : public QObject, public CalypResource
{
  Q_OBJECT
public:
  VideostreamResource( QObject* parent = nullptr );
  VideostreamResource( VideostreamResource&& other ) noexcept = delete;
  VideostreamResource& operator=( VideostreamResource&& other ) noexcept = delete;
  VideostreamResource( const VideostreamResource& other ) = delete;
  VideostreamResource& operator=( const VideostreamResource& other ) = delete;
  ~VideostreamResource();

  auto getResourceName() -> std::string override { return m_currStream.getFileName(); };

  auto iteration() -> bool override;
  auto isReady() -> bool override;

  auto loadFile( const CalypFileInfo& streamInfo, bool optimistic = true ) -> bool;
  auto getStreamInfo() const -> CalypFileInfo { return m_sStreamInfo; }

  auto getResource() -> CalypStream* override { return &m_currStream; };
  auto getStream() const -> const CalypStream& { return m_currStream; };
  auto getCurrFrame() -> std::shared_ptr<CalypFrame>;

  bool play();
  void pause();
  void stop();
  bool playEvent();
  void seekAbsoluteEvent( unsigned int new_frame_num );
  void seekRelativeEvent( bool bIsForward );

private:
  auto goToNextFrame( bool bThreaded ) -> bool;
  void triggerRefresh();

  // Q_SIGNALS:
  // private Q_SLOTS:

private:  // NOLINT
  CalypStream m_currStream;
  CalypFileInfo m_sStreamInfo;

  bool m_bIsPlaying;
};

auto video_resource_guess_format( const QString& filename, unsigned int& rWidth, unsigned int& rHeight,
                                  ClpPixelFormats& rInputFormat, unsigned int& rBitsPerPixel, int& rEndianness,
                                  unsigned int& rFrameRate ) -> bool;

Q_DECLARE_METATYPE( CalypFileInfo )        // NOLINT
Q_DECLARE_METATYPE( CalypFileInfoVector )  // NOLINT

#endif  // __VIDEOSTREAMRESOURCE_H__

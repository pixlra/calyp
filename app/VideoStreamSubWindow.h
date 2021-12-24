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
 * \file     VideoStreamSubWindow.h
 * \brief    Video Sub windows handling
 */

#ifndef __VIDEOSTREAMSUBWINDOW_H__
#define __VIDEOSTREAMSUBWINDOW_H__

#include <QDataStream>
#include <QFuture>
#include <QRect>
#include <QString>
#include <QVector>
#include <cstdint>

#include "CommonDefs.h"
#include "VideoSubWindow.h"
#include "config.h"
#include "lib/CalypStream.h"
#include "models/VideostreamResource.h"

class QScrollArea;

class VideoInformation;
class VideoStreamSubWindow;
class ResourceHandle;
class CalypAppModuleIf;

class VideoStreamSubWindow : public VideoSubWindow
{
  Q_OBJECT

public:
  VideoStreamSubWindow( ResourceHandle* resourceManager, QWidget* parent = 0 );
  ~VideoStreamSubWindow();

  void resetWindowName() override;
  void updateVideoWindowInfo() override;

  bool isPlaying() override;
  void stop() override;
  void advanceOneFrame() override { goToNextFrame( true ); }
  auto getFrameNum() -> std::uint64_t override { return m_pCurrStream->getFrameNum(); }

  bool supportsFormatConfiguration() const { return m_pCurrStream->supportsFormatConfiguration(); };

  void setResource( std::size_t id );
  // bool loadFile( QString cFilename, bool bForceDialog = false );
  // bool loadFile( CalypFileInfo streamInfo );
  void loadAll();
  bool saveStream( const QString& filename );

  bool play();
  void pause();
  bool playEvent();
  void seekAbsoluteEvent( unsigned int new_frame_num );
  void seekRelativeEvent( bool bIsFoward );

  CalypFileInfo getStreamInfo() { return m_sStreamInfo; }

  const CalypStream* getInputStream() { return m_pCurrStream; }
  const CalypStream* getCurrStream() { return m_pCurrStream; }

  QString getCurrentFileName() { return m_cFilename; }

  void refreshSubWindow() override;
  void refreshFrame();
  void refreshFrame( bool bThreaded );

private:
  bool goToNextFrame( bool bThreaded );

private:  // NO_LINT
  ResourceHandle* m_pcResourceManager;
  std::size_t m_uiResourceId;

  QString m_cFilename;
  CalypFileInfo m_sStreamInfo;
  CalypStream* m_pCurrStream;
  QString m_cCurrFileName;

  bool m_bIsPlaying;

  /**
   * Threads variables
   * QtConcurrent
   */
  QFuture<void> m_cRefreshResult;
  QFuture<void> m_cReadResult;
};

#endif  // __VIDEOSTREAMSUBWINDOW_H__

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
 * \file     ModuleSubWindow.h
 * \brief    Video Sub windows handling
 */

#ifndef __MODULESUBWINDOW_H__
#define __MODULESUBWINDOW_H__

#include <QDataStream>
#include <QFuture>
#include <QRect>
#include <QString>
#include <QVector>

#include "CalypAppModuleIf.h"
#include "CommonDefs.h"
#include "VideoSubWindow.h"
#include "config.h"
#include "lib/CalypStream.h"

class VideoInformation;

class ModuleSubWindow : public VideoSubWindow
{
  Q_OBJECT

public:
  ModuleSubWindow( CalypAppModuleIf* module, QWidget* parent = 0 );
  ~ModuleSubWindow();

  void resetWindowName();

  void updateVideoWindowInfo();

  bool isPlaying();
  void stop(){};
  void advanceOneFrame(){};
  auto getFrameNum() -> int { return 1; };

  void refreshSubWindow();

  void setCurrFrame( std::shared_ptr<CalypFrame> pcCurrFrame );
};

#endif  // __MODULESUBWINDOW_H__

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
 * \file     ModulesHandle.h
 * \brief    Calyp modules handle
 */

#ifndef __MODULESHANDLE_H__
#define __MODULESHANDLE_H__

#include <QtCore>
#include <QtWidgets>
#include <cstdio>
#include <iostream>

#include "CalypAppModuleIf.h"
#include "CommonDefs.h"
#include "config.h"
#include "lib/CalypFrame.h"

class VideoHandle;
class SubWindowHandle;
class CalypAppModuleIf;
class SubWindowHandle;

class ModulesHandle : public QWidget
{
  Q_OBJECT
public:
  ModulesHandle( QWidget*, SubWindowHandle*, VideoHandle* );
  ~ModulesHandle();

  void createActions();
  QMenu* createMenu();
  // QDockWidget* createDock();
  void buildMenu();
  void updateMenus();

  void readSettings();
  void writeSettings();

  static void destroyModuleIf( CalypAppModuleIf* pcCurrModuleIf );
  static void applyModuleIf( QList<CalypAppModuleIf*> pcCurrModuleIfList, bool isPlaying = false,
                             bool disableThreads = false );

private:
  QWidget* m_pcParent;
  SubWindowHandle* m_pcMainWindowManager;
  VideoHandle* m_appModuleVideo;

  enum
  {
    INVALID_OPT = -1,
    SWAP_FRAMES_OPT = 0,
    APPLY_ALL_OPT = 1,
  };

  QMenu* m_pcModulesMenu;
  QList<QMenu*> m_pcModulesSubMenuList;
  QList<CalypAppModuleIf*> m_pcCalypAppModuleIfList;

  QVector<QAction*> m_arrayModulesActions;

  enum MODULES_ACTION_LIST
  {
    LOAD_EXTERNAL_ACT,
    // FORCE_PLAYING_REFRESH_ACT,
    APPLY_ALL_ACT,
    SWAP_FRAMES_ACT,
    DISABLE_ACT,
    DISABLE_ALL_ACT,
    FORCE_NEW_WINDOW_ACT,
    MODULES_TOTAL_ACT
  };
  QVector<QAction*> m_arrayActions;
  QSignalMapper* m_pcActionMapper;

  void enableModuleIf( CalypAppModuleIf* pcCurrModuleIf );
  void applyAllModuleIf( CalypAppModuleIf* pcCurrModuleIf );
  void swapModulesWindowsIf( CalypAppModuleIf* pcCurrModuleIf );

  void customEvent( QEvent* event );

Q_SIGNALS:
  void changed();

private Q_SLOTS:
  void loadExternalModule();
  void activateModule();
  void processOpt( int index );
  void destroyWindowModules();
  void destroyAllModulesIf();
};

#endif  // __MODULESHANDLE_H__

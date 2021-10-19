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
 * \file     CalypAppModuleIf.h
 * \brief    Calyp App modules interface
 */

#ifndef __CALYPAPPMODULESIF_H__
#define __CALYPAPPMODULESIF_H__

#include <QEvent>
#include <QPointer>
#include <QVector>
#include <cstdio>
#include <iostream>

#include "config.h"

#ifdef CALYP_THREADED_MODULES
#include <QMutex>
#include <QThread>
#endif

#include "CommonDefs.h"
#include "ModuleHandleDock.h"
// #include "lib/CalypFrame.h"
#include "lib/CalypModuleIf.h"
// #include "lib/CalypStream.h"

class QAction;
class QDockWidget;
class CalypStream;
class CalypFrame;

#define CLP_MODULE_MAX_NUM_FRAMES 5

class VideoSubWindow;
class ModuleSubWindow;

class CalypAppModuleIf : std::enable_shared_from_this<CalypAppModuleIf>,
#ifdef CALYP_THREADED_MODULES
                         public QThread
#else
                         public QObject
#endif

{
  friend class ModulesHandle;
  friend class ModuleHandleDock;
  friend class ModulesHandleOptDialog;

public:
  class EventData : public QEvent
  {
  public:
    EventData( bool success, CalypAppModuleIf* module )
        : QEvent( QEvent::User ), m_bSuccess{ success }, m_pcModule{ module }
    {
    }
    bool m_bSuccess;
    QPointer<CalypAppModuleIf> m_pcModule;
  };

  CalypAppModuleIf( QObject* parent, QAction* action, CalypModulePtr&& module );
  ~CalypAppModuleIf();

  auto getSubWindowList() const -> const std::vector<VideoSubWindow*>& { return m_pcSubWindow; }

  CalypModuleIf* getModule() { return m_pcModule.get(); }
  auto getModuleRequirements() const -> ClpModuleFeatures { return m_pcModule->m_uiModuleRequirements; }
  std::string moduleInfo() const { return m_pcModule->moduleInfo(); }
  void update( bool isPlaying );
  bool apply( bool isPlaying = false, bool disableThreads = false );
  bool process();
  // void setPlaying( bool isPlaying );
  bool isRunning() const;
  void show();
  void disable();

protected:
  virtual void run();

private:
  bool m_bSuccess{ false };

  QAction* m_pcModuleAction{ nullptr };
  CalypModulePtr m_pcModule{ nullptr };

  std::vector<VideoSubWindow*> m_pcSubWindow;

  // The module owns the related widgets
  ModuleSubWindow* m_pcDisplaySubWindow{ nullptr };
  QDockWidget* m_pcDockWidget{ nullptr };
  ModuleHandleDock* m_pcModuleDock{ nullptr };

  std::unique_ptr<CalypStream> m_pcModuleStream{ nullptr };

  std::vector<std::shared_ptr<CalypFrame>> m_frameList;
  std::vector<CalypFrame*> m_frameListPtr;

  std::shared_ptr<CalypFrame> m_pcProcessedFrame{ nullptr };
  double m_dMeasurementResult{ 0 };

#ifdef CALYP_THREADED_MODULES
  QMutex m_Mutex;
  std::atomic<bool> m_canceling{ false };
#endif
};

#endif  // __CALYPAPPMODULESIF_H__

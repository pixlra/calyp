/*    This file is a part of plaYUVer project
 *    Copyright (C) 2014  by Luis Lucas      (luisfrlucas@gmail.com)
 *                           Joao Carreira   (jfmcarreira@gmail.com)
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
 * \file     PlaYUVerModuleIf.h
 * \brief    PlaYUVer modules interface
 */

#ifndef __PLAYUVERMODULESIF_H__
#define __PLAYUVERMODULESIF_H__

#include "config.h"
#include "PlaYUVerDefs.h"
#include <iostream>
#include <cstdio>
#include "PlaYUVerFrame.h"
#include "PlaYUVerStream.h"
#include "ModuleHandleDock.h"

class QAction;

namespace plaYUVer
{

enum // Module type
{
  FRAME_PROCESSING_MODULE,
  FRAME_MEASUREMENT_MODULE,
};

#define APPLY_WHILE_PLAYING true
#define MAX_NUMBER_FRAMES 3

enum // Number of frames
{
  MODULE_REQUIRES_ONE_FRAME = 1,
  MODULE_REQUIRES_TWO_FRAMES = 2,
  MODULE_REQUIRES_THREE_FRAMES = 3,
};

enum // Requirements
{
  MODULE_REQUIRES_NOTHING = 0,
  MODULE_REQUIRES_NEW_WINDOW = 2,
  MODULE_REQUIRES_SIDEBAR = 4,
  MODULES_REQUIREMENTS_TOTAL,
};

#define REGISTER_MODULE(X)                      \
    {                                           \
      if( USE_##X )                             \
      {                                         \
        X *pMod = new X();                      \
        appendModule( pMod );                   \
      }                                         \
    }

typedef struct
{
  Int m_iModuleType;
  const Char* m_pchModuleCategory;
  const Char* m_pchModuleName;
  const Char* m_pchModuleTooltip;
  UInt m_uiNumberOfFrames;
  UInt m_uiModuleRequirements;
  Bool m_bApplyWhilePlaying;
} PlaYUVerModuleDefinition;

class SubWindowHandle;

class PlaYUVerModuleIf
#ifdef PLAYUVER_THREADED_MODULES
    : public QThread
#else
    : public QObject
#endif
{
  friend class ModulesHandle;

private:
  QAction* m_pcAction;
  SubWindowHandle* m_pcSubWindow[MAX_NUMBER_FRAMES];

  SubWindowHandle* m_pcDisplaySubWindow;

  QDockWidget* m_pcDockWidget;
  ModuleHandleDock* m_pcModuleDock;

  PlaYUVerModuleDefinition m_cModuleDef;

  PlaYUVerStream* m_pcModuleStream;
  PlaYUVerFrame* m_pcProcessedFrame;
  Double m_dMeasurementResult;

  Void postProgress( Bool success, PlaYUVerFrame* pcProcessedFrame, Double moduleResult  );
public:
  class EventData: public QEvent
  {
  public:
    EventData() :
            QEvent( QEvent::User )
    {
      m_bSuccess = false;
      m_pcProcessedFrame = NULL;
      m_dMeasurementResult = 0;
      m_pcModule = NULL;
    }
    Bool m_bSuccess;
    PlaYUVerFrame* m_pcProcessedFrame;
    Double m_dMeasurementResult;
    PlaYUVerModuleIf* m_pcModule;
  };

  PlaYUVerModuleIf();
  virtual ~PlaYUVerModuleIf() {}


  virtual Void create() {}
  virtual Void create( PlaYUVerFrame* ) {}

  virtual PlaYUVerFrame*  process( PlaYUVerFrame* ) { return NULL; }
  virtual PlaYUVerFrame*  process( PlaYUVerFrame*, PlaYUVerFrame* ) { return NULL; }
  virtual PlaYUVerFrame*  process( PlaYUVerFrame*, PlaYUVerFrame*, PlaYUVerFrame* ) { return NULL; }

  virtual Double          measure( PlaYUVerFrame* ) { return 0; }
  virtual Double          measure( PlaYUVerFrame*, PlaYUVerFrame* ) { return 0; }
  virtual Double          measure( PlaYUVerFrame*, PlaYUVerFrame*, PlaYUVerFrame* ) { return 0; }

  virtual Void destroy() { }

  Void setModuleDefinition( PlaYUVerModuleDefinition def )  { m_cModuleDef = def;   }
  PlaYUVerModuleDefinition getModuleDefinition()            { return m_cModuleDef;  }
protected:
    virtual void run();
};

}  // NAMESPACE

#endif // __PLAYUVERMODULESIF_H__


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
 * \file     ResourceHandle.h
 * \brief    Handle multiple sub-windows
 */

#ifndef __RESOURCEHANDLE_H__
#define __RESOURCEHANDLE_H__

#include <QMutex>
#include <QThread>
#include <QVector>
#include <QWaitCondition>
#include <atomic>

#include "CommonDefs.h"
#include "lib/CalypStream.h"

class ResourceWorker : public QThread
{
private:
  QMutex m_Mutex;
  QWaitCondition m_ResourceIdle;
  std::shared_ptr<CalypStream> m_pcStream;
  std::atomic_flag m_bStop;

public:
  ResourceWorker( std::shared_ptr<CalypStream> stream );
  void stop();
  void wake();
  void run();
};

class ResourceHandle
{
public:
  ResourceHandle();
  ~ResourceHandle() = default;

  auto getResource( CalypStream* ptr ) -> std::size_t;
  auto getResourceAsset( std::size_t id ) -> CalypStream*;
  void removeResource( std::size_t id );
  void stopResourceWorker( std::size_t id );
  void startResourceWorker( std::size_t id );
  void wakeResourceWorker( std::size_t id );

private:
  auto addResource() -> std::size_t;

private:
  std::size_t unique_id{ 0 };
  std::map<std::size_t, std::shared_ptr<CalypStream>> m_apcStreamResourcesList;
  std::map<std::size_t, std::unique_ptr<ResourceWorker>> m_apcStreamResourcesWorkersList;
};

#endif  // __RESOURCEHANDLE_H__

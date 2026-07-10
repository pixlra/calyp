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
#include <QMutexLocker>
#include <QThread>
#include <QVector>
#include <QWaitCondition>
#include <atomic>

#include "CalypResource.h"
#include "lib/CalypStream.h"

class ResourceWorker : public QThread
{
private:
  QMutex m_Mutex;
  QWaitCondition m_ResourceIdle;
  std::shared_ptr<CalypResource> m_resource;
  std::atomic<bool> m_bStop{ false };
  std::atomic<bool> m_bStarted{ false };
  std::atomic<bool> m_bStarting{ false };

public:
  explicit ResourceWorker( std::shared_ptr<CalypResource> resource ) : m_resource{ std::move( resource ) } {}
  ResourceWorker( ResourceWorker&& other ) noexcept = delete;
  ResourceWorker( const ResourceWorker& other ) = delete;
  auto operator=( ResourceWorker&& other ) noexcept -> ResourceWorker& = delete;
  auto operator=( const ResourceWorker& other ) -> ResourceWorker& = delete;
  ~ResourceWorker() override;
  void stop();
  void wake();
  void start();
  void run() override;
  auto mutex() -> QMutex* { return &m_Mutex; }
};

class ResourceWorker;

class ResourceHandle : public QObject
{
  Q_OBJECT
public:
  ResourceHandle();
  ResourceHandle( ResourceHandle&& other ) noexcept = delete;
  ResourceHandle( const ResourceHandle& other ) = delete;
  auto operator=( ResourceHandle&& other ) noexcept -> ResourceHandle& = delete;
  auto operator=( const ResourceHandle& other ) -> ResourceHandle& = delete;
  ~ResourceHandle() override;

  auto getResource( CalypStream* ptr ) -> std::size_t;
  auto getResourceAsset( std::size_t id ) -> CalypStream*;
  auto appendResource( std::unique_ptr<CalypResource>&& resource ) -> std::size_t;

  void removeResource( std::size_t id );
  void stopResourceWorker( std::size_t id );
  void startResourceWorker( std::size_t id );
  void wakeResourceWorker( std::size_t id );

  template <typename T>
  auto executeResourceAction( std::size_t id, const std::function<bool( T* )>& action ) -> bool
  {
    static_assert( std::is_base_of<CalypResource, T>::value, "T must inherit from CalypResource" );
    if( !m_apcStreamResourcesList.contains( id ) )
    {
      assert( false );
      return false;
    }
    bool result{ false };
    if( auto* resource = dynamic_cast<T*>( m_apcStreamResourcesList[id].get() ) )
    {
      QMutexLocker locker( m_apcStreamResourcesWorkersList[id]->mutex() );
      result = action( resource );
    }
    m_apcStreamResourcesWorkersList[id]->wake();
    return result;
  }

public slots:
  void cleanup();

private:
  auto addResource() -> std::size_t;

private:  // NOLINT
  std::unique_ptr<QThread> m_thread;
  std::map<std::size_t, std::shared_ptr<CalypResource>> m_apcStreamResourcesList;
  std::map<std::size_t, std::unique_ptr<ResourceWorker>> m_apcStreamResourcesWorkersList;
};

#endif  // __RESOURCEHANDLE_H__

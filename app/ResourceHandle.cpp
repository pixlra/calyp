/*    This file is a part of Calyp project
 *    Copyright (C) 2014-2021  by Joao Carreira   (jfmcarreira@gmail.com)
 *                                Luis Lucas      (luisfrlucas@gmail.com)
 *                                Joao Santos     (joaompssantos@gmail.com)
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
 * \file     ResourceHandle.cpp
 * \brief    Handle resources
 */

#include "ResourceHandle.h"

#include <QCoreApplication>
#include <QEvent>
#include <QFileInfo>
#include <QMutex>
#include <QThread>
#include <memory>

ResourceWorker::~ResourceWorker()
{
  stop();
}

void ResourceWorker::stop()
{
  while( m_bStarting )
  {
  }
  m_bStop = true;
  if( m_bStarted )
  {
    wake();
    wait();
  }
  m_bStarted = false;
}

void ResourceWorker::wake()
{
  m_Mutex.lock();
  m_ResourceIdle.wakeAll();
  m_Mutex.unlock();
}

void ResourceWorker::start()
{
  m_bStarting = true;
  QThread::start();
}

void ResourceWorker::run()
{
  m_bStop = false;
  m_bStarted = true;
  m_bStarting = false;

  // Loop forever
  while( true )
  {
    while( !m_bStop && !m_resource->isReady() )
    {
      // Wait here
      m_Mutex.lock();
      m_ResourceIdle.wait( &m_Mutex );
      m_Mutex.unlock();
    }

    // auto start = std::chrono::steady_clock::now();
    if( m_bStop || !m_resource->iteration() )
    {
      break;
    }
    // auto end = std::chrono::steady_clock::now();
    // std::cout << "Elapsed time reading and processing a frame: "
    //           << std::chrono::duration_cast<std::chrono::milliseconds>( end - start ).count()
    //           << " ms" << std::endl;
  }
  m_bStarted = false;
  m_bStop = false;
}

class OldStreamResource : public CalypResource
{
public:
  OldStreamResource() = default;
  CalypStream m_stream;
  auto getResourceName() -> std::string override { return m_stream.getFileName(); };
  auto getResource() -> CalypStream* override { return &m_stream; };

  auto iteration() -> bool override
  {
    if( m_stream.getFrameNum() < 2 )
    {
      return false;
    }
    m_stream.readNextFrameFillRGBBuffer();
    return true;
  }
  auto isReady() -> bool override { return m_stream.hasWritingSlot(); }
};

ResourceHandle::ResourceHandle() : m_thread{ std::make_unique<QThread>() }
{
  moveToThread( m_thread.get() );
  m_thread->setObjectName( "ResourceHandleThread" );
  m_thread->start();
};

ResourceHandle::~ResourceHandle()
{
  QMetaObject::invokeMethod( this, "cleanup" );
  m_thread->wait();
};

void ResourceHandle::cleanup()
{
  m_thread->quit();
};

auto ResourceHandle::addResource() -> std::size_t
{
  auto resource_id = m_uiNextUniqueId++;
  auto newStreamResource = std::make_shared<OldStreamResource>();
  auto newStreamResourceWorker = std::make_unique<ResourceWorker>( newStreamResource );
  m_apcStreamResourcesList[resource_id] = newStreamResource;
  m_apcStreamResourcesWorkersList[resource_id] = std::move( newStreamResourceWorker );
  return resource_id;
}

auto ResourceHandle::getResource( CalypStream* ptr ) -> std::size_t
{
  if( ptr != nullptr )
  {
    for( std::size_t i = 0; i < m_apcStreamResourcesList.size(); i++ )
    {
      if( auto* resource = dynamic_cast<OldStreamResource*>( m_apcStreamResourcesList[i].get() ) )
      {
        if( &resource->m_stream == ptr ) return i;
      }
    }
  }
  return addResource();
}

auto ResourceHandle::getResourceAsset( std::size_t id ) -> CalypStream*
{
  if( !m_apcStreamResourcesWorkersList.contains( id ) ) return nullptr;
  return m_apcStreamResourcesList[id]->getResource();
}

auto ResourceHandle::appendResource( std::unique_ptr<CalypResource>&& resource ) -> std::size_t
{
  auto resource_id = m_uiNextUniqueId++;
  m_apcStreamResourcesList[resource_id] = std::move( resource );
  m_apcStreamResourcesWorkersList[resource_id] =
      std::make_unique<ResourceWorker>( m_apcStreamResourcesList[resource_id] );
  return resource_id;
}

void ResourceHandle::removeResource( std::size_t id )
{
  if( !m_apcStreamResourcesList.contains( id ) )
  {
    assert( false );
    return;
  }

  // Remove worker first
  m_apcStreamResourcesWorkersList[id]->stop();
  m_apcStreamResourcesWorkersList[id] = nullptr;

  // Then the underlying stream
  m_apcStreamResourcesList[id] = nullptr;
}

void ResourceHandle::stopResourceWorker( std::size_t id )
{
  if( !m_apcStreamResourcesWorkersList.contains( id ) )
  {
    assert( false );
    return;
  }
  m_apcStreamResourcesWorkersList[id]->stop();
}

void ResourceHandle::startResourceWorker( std::size_t id )
{
  if( !m_apcStreamResourcesWorkersList.contains( id ) )
  {
    assert( false );
    return;
  }
  m_apcStreamResourcesWorkersList[id]->setObjectName(
      "RW-" +
      QFileInfo( QString::fromStdString( m_apcStreamResourcesList[id]->getResourceName() ) ).completeBaseName() );
  m_apcStreamResourcesWorkersList[id]->start();
}

void ResourceHandle::wakeResourceWorker( std::size_t id )
{
  if( !m_apcStreamResourcesWorkersList.contains( id ) )
  {
    assert( false );
    return;
  }
  m_apcStreamResourcesWorkersList[id]->wake();
}

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
#include <chrono>
#include <iostream>
#include <memory>
#include <optional>

ResourceWorker::ResourceWorker( std::shared_ptr<CalypStream> stream )
    : m_pcStream{ stream }
{
}

void ResourceWorker::stop()
{
  m_bStop = true;
  wake();
  wait();
}

void ResourceWorker::wake()
{
  m_Mutex.lock();
  m_ResourceIdle.wakeAll();
  m_Mutex.unlock();
}

void ResourceWorker::run()
{
  m_bStop = false;

  // Loop forever
  while( !m_bStop )
  {
    // auto start = std::chrono::steady_clock::now();
    m_pcStream->readNextFrameFillRGBBuffer();
    // auto end = std::chrono::steady_clock::now();
    // std::cout << "Elapsed time reading and processing a frame: "
    //           << std::chrono::duration_cast<std::chrono::milliseconds>( end - start ).count()
    //           << " ms" << std::endl;

    while( !m_bStop && !m_pcStream->hasWritingSlot() )
    {
      // Wait here
      m_Mutex.lock();
      m_ResourceIdle.wait( &m_Mutex );
      m_Mutex.unlock();
    }
  }
}

ResourceHandle::ResourceHandle()
{
}

auto ResourceHandle::addResource() -> std::size_t
{
  auto resource_id = unique_id;
  unique_id++;
  auto newStreamResource = std::make_shared<CalypStream>();
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
      if( m_apcStreamResourcesList[i].get() == ptr )
        return i;
    }
  }
  return addResource();
}

auto ResourceHandle::getResourceAsset( std::size_t id ) -> CalypStream*
{
  if( m_apcStreamResourcesWorkersList.count( id ) )
  {
    return m_apcStreamResourcesList[id].get();
  }
  return nullptr;
}

void ResourceHandle::removeResource( std::size_t id )
{
  if( !m_apcStreamResourcesWorkersList.count( id ) )
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
  if( !m_apcStreamResourcesWorkersList.count( id ) )
  {
    assert( false );
    return;
  }
  m_apcStreamResourcesWorkersList[id]->stop();
}

void ResourceHandle::startResourceWorker( std::size_t id )
{
  if( !m_apcStreamResourcesWorkersList.count( id ) )
  {
    assert( false );
    return;
  }
  if( m_apcStreamResourcesList[id]->getFrameNum() < 2 )
  {
    return;  // No work to be done!
  }
  m_apcStreamResourcesWorkersList[id]->setObjectName(
      "RW-" +
      QFileInfo( QString::fromStdString( m_apcStreamResourcesList[id]->getFileName() ) ).completeBaseName() );
  m_apcStreamResourcesWorkersList[id]->start();
}

void ResourceHandle::wakeResourceWorker( std::size_t id )
{
  if( !m_apcStreamResourcesWorkersList.count( id ) )
  {
    assert( false );
    return;
  }
  m_apcStreamResourcesWorkersList[id]->wake();
}
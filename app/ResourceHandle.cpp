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
#include <optional>

ResourceWorker::ResourceWorker( std::shared_ptr<CalypStream> stream )
{
  m_pcStream = stream;
}

void ResourceWorker::stop()
{
  m_bStop = true;
  wait();
}

void ResourceWorker::run()
{
  while( !m_bStop )
  {
    m_pcStream->readNextFrameFillRGBBuffer();
    while( !m_bStop && !m_pcStream->hasWritingSlot() )
      ;  // Keep
  }
}

ResourceHandle::ResourceHandle()
{
}

auto ResourceHandle::addResource() -> CalypStream*
{
  auto newStreamResource = std::make_shared<CalypStream>();
  auto newStreamResourceWorker = std::make_unique<ResourceWorker>( newStreamResource );
  CalypStream* ptr = newStreamResource.get();

  m_apcStreamResourcesList.push_back( newStreamResource );
  m_apcStreamResourcesWorkersList.push_back( std::move( newStreamResourceWorker ) );
  return ptr;
}

auto ResourceHandle::getResource( CalypStream* ptr ) -> CalypStream*
{
  if( ptr != nullptr )
  {
    for( auto& resource_ptr : m_apcStreamResourcesList )
    {
      if( resource_ptr.get() == ptr )
        return resource_ptr.get();
    }
  }
  return addResource();
}

void ResourceHandle::removeResource( CalypStream* ptr )
{
  std::optional<std::size_t> posToRemove;
  for( std::size_t i = 0; i < m_apcStreamResourcesList.size(); i++ )
  {
    if( m_apcStreamResourcesList[i].get() == ptr )
    {
      posToRemove = i;
      break;
    }
  }
  if( !posToRemove.has_value() )
  {
    assert( false );
    return;
  }

  // Remove worker first
  m_apcStreamResourcesWorkersList[*posToRemove]->stop();
  m_apcStreamResourcesWorkersList.erase( m_apcStreamResourcesWorkersList.begin() + *posToRemove );

  // Then the underlying stream
  m_apcStreamResourcesList.erase( m_apcStreamResourcesList.begin() + *posToRemove );
}

void ResourceHandle::startResourceWorker( CalypStream* ptr )
{
  std::optional<std::size_t> position;
  for( std::size_t i = 0; i < m_apcStreamResourcesList.size(); i++ )
  {
    if( m_apcStreamResourcesList[i].get() == ptr )
    {
      position = i;
      break;
    }
  }
  if( !position.has_value() )
  {
    assert( false );
    return;
  }
  m_apcStreamResourcesWorkersList[*position]->setObjectName(
      "RW-" +
      QFileInfo( QString::fromStdString( m_apcStreamResourcesList[*position]->getFileName() ) ).completeBaseName() );
  m_apcStreamResourcesWorkersList[*position]->start();
}
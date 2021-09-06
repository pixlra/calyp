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

#include <QThread>
#include <QVector>

#include "CommonDefs.h"
#include "lib/CalypStream.h"

class ResourceWorker : public QThread
{
private:
  std::shared_ptr<CalypStream> m_pcStream;
  bool m_bStop{ false };

public:
  ResourceWorker( std::shared_ptr<CalypStream> stream );
  void stop();
  void run();
};

class ResourceHandle
{
public:
  ResourceHandle();
  ~ResourceHandle() = default;

  auto getResource( CalypStream* ptr ) -> CalypStream*;
  void removeResource( CalypStream* ptr );
  void startResourceWorker( CalypStream* ptr );

private:
  auto addResource() -> CalypStream*;

private:
  std::vector<std::shared_ptr<CalypStream>> m_apcStreamResourcesList;
  std::vector<std::unique_ptr<ResourceWorker>> m_apcStreamResourcesWorkersList;
};

#endif  // __RESOURCEHANDLE_H__

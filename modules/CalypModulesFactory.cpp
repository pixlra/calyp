/*    This file is a part of Calyp project
 *    Copyright (C) 2014-2018  by Joao Carreira   (jfmcarreira@gmail.com)
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
 * \file     CalypModuleFactory.cpp
 * \brief    Calyp modules factory
 */

#include "CalypModulesFactory.h"
#include "config.h"

#include <cstring>
#include <functional>
#ifdef USE_DYNLOAD
#include <dlfcn.h>
#include <iostream>
#endif

// Generated by cmake
#include "ModulesListHeader.h"

CalypModulesFactory::CalypModulesFactory(){REGISTER_ALL_MODULES}

CalypModulesFactory::~CalypModulesFactory()
{
  m_FactoryMap.clear();
}

void CalypModulesFactory::Register( const char* moduleName, CreateModuleFn pfnCreate )
{
  m_FactoryMap[moduleName] = pfnCreate;
}

bool CalypModulesFactory::RegisterDl( const char* dlName )
{
#ifdef USE_DYNLOAD
  void* pHndl = dlopen( dlName, RTLD_NOW );
  if( pHndl == NULL )
  {
    std::cerr << dlerror() << std::endl;
    return false;
  }
  CreateModuleFn pfnCreate = (CreateModuleFn)dlsym( pHndl, "Maker" );
  if( pfnCreate == NULL )
  {
    return false;
  }
  Register( dlName, pfnCreate );
  return true;
#else
  return false;
#endif
}

CalypModuleIf* CalypModulesFactory::CreateModule( const char* moduleName )
{
  CalypModulesFactoryMap& moduleFactoryMap = CalypModulesFactory::Get()->getMap();
  CalypModulesFactoryMap::iterator it = moduleFactoryMap.begin();
  for( ; it != moduleFactoryMap.end(); ++it )
  {
    if( !strcmp( it->first, moduleName ) )
    {
      return it->second();
    }
  }
  return NULL;
}
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
 * \file     CalypModuleFactory.h
 * \brief    Calyp modules factory
 */

#ifndef __CALYPMODULESFACTORY_H__
#define __CALYPMODULESFACTORY_H__

#include <functional>
#include <map>

// CalypLib
#include "lib/CalypModuleIf.h"

// Factory for creating instances of CalypModuleIf
class CalypModulesFactory
{
  using CalypModulesFactoryMap = std::map<std::string, std::function<CalypModulePtr( void )>>;

private:
  CalypModulesFactory();
  CalypModulesFactoryMap m_FactoryMap;

public:
  static CalypModulesFactory* Get()
  {
    static CalypModulesFactory instance;
    return &instance;
  }

  void Register( const std::string& moduleName, std::function<CalypModulePtr( void )> pfnCreate );
  void Register( const char* moduleName, std::function<CalypModulePtr( void )> pfnCreate );
  bool RegisterDl( const char* dlName );

  CalypModulePtr CreateModule( const std::string& moduleName ) const;
  CalypModulesFactoryMap& getMap() { return m_FactoryMap; }
};

#endif  // __CALYPMODULESFACTORY_H__

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
 * \file     ModuleSubWindow.cpp
 * \brief    Video Sub windows handling
 */

#include "ModuleSubWindow.h"

#include <QSettings>
#include <QStaticText>
#include <cassert>

#include "ModulesHandle.h"

ModuleSubWindow::ModuleSubWindow( CalypAppModuleIf* module, QWidget* parent )
    : VideoSubWindow( VideoSubWindow::MODULE_SUBWINDOW, parent )
{
  m_pcDisplayModule = module;
}

ModuleSubWindow::~ModuleSubWindow()
{
}

void ModuleSubWindow::resetWindowName()
{
  QString windowName{ "Module" };
  if( auto module_ptr = m_pcDisplayModule )
  {
    windowName.append( " " ).append( module_ptr->getModule()->m_pchModuleLongName );
  }
  setWindowName( windowName );
}

void ModuleSubWindow::updateVideoWindowInfo()
{
  m_cStreamInformation = "";

  m_cStreamInformation = "Module";
  if( auto module_ptr = m_pcDisplayModule )
  {
    const auto& arraySubWindows = module_ptr->getSubWindowList();
    QStringList windowInfoList;
    if( arraySubWindows.size() > 0 )
    {
      for( std::size_t i = 0; i < arraySubWindows.size(); i++ )
      {
        if( arraySubWindows[i]->getWindowName() != getWindowName() )
        {
          windowInfoList.append( QString( "Input %1 - " + arraySubWindows[i]->getWindowName() ).arg( i + 1 ) );
        }
      }
    }
    if( module_ptr->getModuleRequirements() & CLP_MODULES_HAS_INFO )
    {
      QStringList list = QString::fromStdString( module_ptr->moduleInfo() ).split( '\n' );
      windowInfoList.append( list );
    }
    if( windowInfoList.size() > 0 )
    {
      m_pcVideoInfo->setInformationTopLeft( windowInfoList );
    }
  }
  if( m_pcCurrFrameAsset )
  {
    QString m_cPelFmtName = QString::fromStdString( m_pcCurrFrameAsset->getPelFmtName() );
    m_cStreamInformation += " | " + m_cPelFmtName;
  }
  if( m_cStreamInformation.isEmpty() )
  {
    m_cStreamInformation = "          ";
  }
}

bool ModuleSubWindow::isPlaying()
{
  if( auto module_ptr = m_pcDisplayModule )
  {
    for( auto window : module_ptr->getSubWindowList() )
    {
      if( window->isPlaying() )
        return true;
    }
  }
  return false;
}

void ModuleSubWindow::refreshSubWindow()
{
  if( auto module_ptr = m_pcDisplayModule )
  {
    m_bWindowBusy = true;
    updateVideoWindowInfo();
    module_ptr->apply( m_bIsPlaying, false );
  }
}
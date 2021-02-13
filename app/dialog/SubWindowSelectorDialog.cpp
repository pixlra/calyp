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
 * \file     DialogSubWindowSelector.cpp
 * \brief    Dialog box to select sub windows
 */

#include "SubWindowSelectorDialog.h"

#include <QCheckBox>
#include <QGroupBox>

#include "SubWindowHandle.h"
#include "VideoSubWindow.h"

SubWindowSelectorDialog::SubWindowSelectorDialog( QWidget* parent, SubWindowHandle* windowManager,
                                                  unsigned int uiCategory, int minWindowsSelected, int maxWindowsSelected )
    : QDialog( parent )
    , m_uiCategory( uiCategory )
    , m_iMinSelectedWindows( minWindowsSelected )
    , m_iMaxSlectedWindows( maxWindowsSelected )
    , m_pcMainWindowManager( windowManager )
{
  QSize windowSize( 350, 80 );
  resize( windowSize );
  setWindowTitle( "Select desired Sub-Windows" );

  QVBoxLayout* mainLayout = new QVBoxLayout( this );

  minWindowsSelected = minWindowsSelected == 0 ? 1 : minWindowsSelected;

  m_pcGroupCheckBox = new QGroupBox();
  m_pcGroupCheckBox->setLayout( new QVBoxLayout() );

  mainLayout->addWidget( m_pcGroupCheckBox );

  m_pushButtonAddAll = new QPushButton( "Add all", this );
  m_pushButtonRemoveAll = new QPushButton( "Remove All", this );
  m_pushButtonRemoveAll->setEnabled( false );

  // Confirmation buttons
  m_pushButtonOkCancel = new QDialogButtonBox();
  m_pushButtonOkCancel->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );
  m_pushButtonOkCancel->setOrientation( Qt::Horizontal );
  m_pushButtonOkCancel->setCenterButtons( false );
  m_pushButtonOkCancel->buttons().at( 0 )->setEnabled( false );

  QHBoxLayout* BottomHLayout = new QHBoxLayout();
  BottomHLayout->addWidget( m_pushButtonAddAll );
  BottomHLayout->addWidget( m_pushButtonRemoveAll );
  BottomHLayout->addWidget( m_pushButtonOkCancel );

  // Add components
  mainLayout->addLayout( BottomHLayout );

  setLayout( mainLayout );

  // Create actions
  connect( m_pushButtonOkCancel, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( m_pushButtonOkCancel, SIGNAL( rejected() ), this, SLOT( reject() ) );

  connect( m_pushButtonAddAll, SIGNAL( clicked() ), this, SLOT( addAllSubWindow() ) );
  connect( m_pushButtonRemoveAll, SIGNAL( clicked() ), this, SLOT( removeAllSubWindow() ) );

  m_mapperWindowsList = new QSignalMapper( this );
  connect( m_mapperWindowsList, SIGNAL( mapped( int ) ), this, SLOT( toggleSubWindow( int ) ) );

  SubWindowAbstract* subWindow;
  QString currSubWindowName;
  QCheckBox* windowCheckBox;
  m_apcSubWindowList = m_pcMainWindowManager->findSubWindow( m_uiCategory );
  for( int i = 0; i < m_apcSubWindowList.size(); i++ )
  {
    subWindow = m_apcSubWindowList.at( i );
    currSubWindowName = subWindow->getWindowName();
    windowCheckBox = new QCheckBox( currSubWindowName, this );
    connect( windowCheckBox, SIGNAL( stateChanged( int ) ), m_mapperWindowsList, SLOT( map() ) );
    m_mapperWindowsList->setMapping( windowCheckBox, i );
    m_apcWindowsListCheckBox.append( windowCheckBox );
  }

  update();

  setFixedSize( mainLayout->sizeHint() );
}

void SubWindowSelectorDialog::updateSubWindowList()
{
  QLayout* pcLayout = m_pcGroupCheckBox->layout();
  QLayoutItem* pcLayoutItem;
  QCheckBox* windowCheckBox;
  int iIdx;

  /**
   * Remove all check boxes
   */
  for( int i = 0; i < pcLayout->count(); i++ )
  {
    pcLayoutItem = pcLayout->itemAt( i );
    if( pcLayoutItem )
    {
      pcLayout->removeWidget( pcLayoutItem->widget() );
      pcLayout->removeItem( pcLayoutItem );
    }
  }

  for( int i = 0; i < m_apcSelectedSubWindowList.size(); i++ )
  {
    iIdx = m_apcSubWindowList.indexOf( m_apcSelectedSubWindowList.at( i ) );
    if( iIdx >= 0 )
    {
      windowCheckBox = m_apcWindowsListCheckBox.at( iIdx );
      m_pcGroupCheckBox->layout()->addWidget( windowCheckBox );
    }
  }
  for( int i = 0; i < m_apcWindowsListCheckBox.size(); i++ )
  {
    if( !m_apcSelectedSubWindowList.contains( m_apcSubWindowList.at( i ) ) )
      m_pcGroupCheckBox->layout()->addWidget( m_apcWindowsListCheckBox.at( i ) );
  }
}

void SubWindowSelectorDialog::selectSubWindow( SubWindowAbstract* subWindow )
{
  int iIdx = m_apcSubWindowList.indexOf( subWindow );
  if( iIdx >= 0 )
  {
    m_apcWindowsListCheckBox.at( iIdx )->setChecked( true );
  }
}

void SubWindowSelectorDialog::update()
{
  if( m_apcSelectedSubWindowList.size() > 0 )
  {
    m_pushButtonRemoveAll->setEnabled( true );
  }
  else
  {
    m_pushButtonRemoveAll->setEnabled( false );
  }

  if( m_apcSelectedSubWindowList.size() >= m_iMinSelectedWindows )
  {
    m_pushButtonOkCancel->buttons().at( 0 )->setEnabled( true );
  }
  else
  {
    m_pushButtonOkCancel->buttons().at( 0 )->setEnabled( false );
  }

  if( m_iMaxSlectedWindows > 0 )
  {
    if( m_apcWindowsListCheckBox.size() > m_iMaxSlectedWindows )
    {
      m_pushButtonAddAll->setEnabled( false );
    }
    else
    {
      m_pushButtonAddAll->setEnabled( true );
    }
    if( m_apcSelectedSubWindowList.size() >= m_iMaxSlectedWindows )
    {
      for( int i = 0; i < m_apcWindowsListCheckBox.size(); i++ )
      {
        if( m_apcSelectedSubWindowList.contains( m_apcSubWindowList.at( i ) ) )
        {
          m_apcWindowsListCheckBox.at( i )->setEnabled( true );
        }
        else
        {
          m_apcWindowsListCheckBox.at( i )->setEnabled( false );
        }
      }
    }
    else
    {
      for( int i = 0; i < m_apcWindowsListCheckBox.size(); i++ )
      {
        m_apcWindowsListCheckBox.at( i )->setEnabled( true );
      }
    }
  }
  updateSubWindowList();
}

// -----------------------  Slot Functions  -----------------------

void SubWindowSelectorDialog::toggleSubWindow( int idx )
{
  bool isChecked = m_apcWindowsListCheckBox.at( idx )->isChecked();

  if( isChecked )
  {
    m_apcSelectedSubWindowList.append( m_apcSubWindowList.at( idx ) );
  }
  else
  {
    m_apcSelectedSubWindowList.removeOne( m_apcSubWindowList.at( idx ) );
  }
  update();
}

void SubWindowSelectorDialog::addAllSubWindow()
{
  m_apcSelectedSubWindowList.clear();
  for( int i = 0; i < m_apcWindowsListCheckBox.size(); i++ )
  {
    m_apcWindowsListCheckBox.at( i )->setChecked( true );
  }
  update();
}

void SubWindowSelectorDialog::removeAllSubWindow()
{
  for( int i = 0; i < m_apcWindowsListCheckBox.size(); i++ )
  {
    m_apcWindowsListCheckBox.at( i )->setChecked( false );
  }
  m_apcSelectedSubWindowList.clear();
  update();
}

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
 * \file     ModulesHandleOptDialog.cpp
 * \brief    Dialog box to config modules opts
 */

#include "ModulesHandleOptDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "lib/CalypOptions.h"

class QLineEdit;

class OpionConfiguration : public QWidget
{
public:
  OpionConfiguration( OptionBase* option )
  {
    m_pcChecked = NULL;
    m_cName = option->opt_string;
    m_pcDescription = new QLabel;
    QString desc = QString::fromStdString( option->opt_desc );
    m_pcDescription->setText( desc );
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget( m_pcDescription, 0, Qt::AlignLeft );
    if( !option->isBinary() )
    {
      m_pcValue = new QLineEdit;
      m_pcValue->setMinimumWidth( 40 );
      m_pcValue->setMaximumWidth( 65 );
      layout->addWidget( m_pcValue, 0, Qt::AlignRight );
    }
    else
    {
      m_pcChecked = new QCheckBox;
      m_pcChecked->setChecked( false );
      layout->addWidget( m_pcChecked, 0, Qt::AlignRight );
    }
    layout->setContentsMargins( 1, 1, 1, 1 );
    setLayout( layout );
  }
  const QString getValue() const
  {
    if( m_pcChecked )
    {
      return m_pcChecked->isChecked() ? "true" : "";
    }
    return m_pcValue->text();
  }
  const std::string& getName() { return m_cName; }

private:
  QLineEdit* m_pcValue;
  QCheckBox* m_pcChecked;
  QLabel* m_pcDescription;
  std::string m_cName;
};

ModulesHandleOptDialog::ModulesHandleOptDialog( QWidget* parent, CalypAppModuleIf* pcCurrModuleIf )
    : QDialog( parent ), m_pcCurrModuleIf( pcCurrModuleIf )
{
  resize( 400, 10 );
  setWindowTitle( "Select module parameters" );

  const CalypOptions::OptionsList& moduleOptions = m_pcCurrModuleIf->m_pcModule->m_cModuleOptions.getOptionList();

  QVBoxLayout* optionsLayout = new QVBoxLayout;
  OpionConfiguration* pcOption;
  for( CalypOptions::OptionsList::const_iterator it = moduleOptions.begin(); it != moduleOptions.end(); ++it )
  {
    pcOption = new OpionConfiguration( ( *it )->opt );
    m_apcOptionList.append( pcOption );
    optionsLayout->addWidget( pcOption );
  }

  QDialogButtonBox* dialogButtonOkCancel = new QDialogButtonBox();
  dialogButtonOkCancel->setObjectName( QString::fromUtf8( "dialogButtonBox" ) );
  dialogButtonOkCancel->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );
  dialogButtonOkCancel->setCenterButtons( false );

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addItem( optionsLayout );
  mainLayout->addWidget( dialogButtonOkCancel );
  setLayout( mainLayout );

  setFixedSize( mainLayout->sizeHint() );

  connect( dialogButtonOkCancel, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( dialogButtonOkCancel, SIGNAL( rejected() ), this, SLOT( reject() ) );
}

int ModulesHandleOptDialog::runConfiguration()
{
  if( exec() == QDialog::Rejected )
  {
    return QDialog::Rejected;
  }

  std::vector<std::string> argsArray;
  std::string optionString;
  QString valueString( "" );

  for( int i = 0; i < m_apcOptionList.size(); i++ )
  {
    valueString = m_apcOptionList.at( i )->getValue();
    if( !valueString.isEmpty() )
    {
      optionString.append( "--" );
      optionString.append( m_apcOptionList.at( i )->getName() );
      if( !( valueString == "true" ) )
      {
        optionString.append( "=" );
        optionString.append( valueString.toStdString() );
      }
      argsArray.push_back( optionString );
    }
    optionString.clear();
  }
  if( argsArray.size() > 0 )
  {
    m_pcCurrModuleIf->m_pcModule->m_cModuleOptions.parse( argsArray );
  }
  return QDialog::Accepted;
}

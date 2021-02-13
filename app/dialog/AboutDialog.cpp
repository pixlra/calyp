/*    This file is a part of Calyp project
 *    Copyright (C) 2014-2019  by Joao Carreira   (jfmcarreira@gmail.com)
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
 * \file     AboutDialog.cpp
 * \brief    About Dialog
 *           Based on the work of Glad Deschrijver <glad.deschrijver@gmail.com>
 * in KTikZ project
 */

#include "AboutDialog.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>

#include "config.h"

#define TAB_FROM_SPACES "&nbsp;&nbsp;&nbsp;"

AboutDialog::AboutDialog( QWidget* parent )
    : QDialog( parent )
{
  QPixmap logo = QPixmap( ":logos/calyp-logo.png" ).scaledToWidth( 200, Qt::SmoothTransformation );
  QLabel* pixmapLabel = new QLabel;
  pixmapLabel->setPixmap( logo );

  QLabel* label = new QLabel( QString( "<h2 style='text-align:center'>%1</h2><h3 style='text-align:center'>Version %2</h3>" )
                                  .arg( tr( "Enhanced open-source video player" ) )
                                  .arg( QApplication::applicationVersion() ) );
  label->setWordWrap( false );

  QString featuresList = QStringLiteral( "<p><h3><b>Features:</b></h3><ul>" );

#ifdef USE_QTDBUS
  featuresList.append( QStringLiteral( "<li>Support for Qt DBus messages" ) );
#endif
#ifdef USE_FFMPEG
  featuresList.append( QStringLiteral( "<li>Support for FFmpeg" ) );
#endif
#ifdef USE_OPENCV
  featuresList.append( QStringLiteral( "<li>Support for OpenCV" ) );
#endif
  featuresList.append( QStringLiteral( "</ul></p>" ) );
  QLabel* labelFeatures = new QLabel( featuresList );

  QLabel* labelCopyright = new QLabel( QStringLiteral( "Copyright © 2014–2018 Joao Carreira and Luis Lucas" ) );

  QWidget* topWidget = new QWidget;
  QVBoxLayout* topLayout = new QVBoxLayout;
  topLayout->addWidget( pixmapLabel, 0, Qt::AlignHCenter );
  topLayout->addWidget( label );
  topLayout->addWidget( labelFeatures );
  topLayout->addWidget( labelCopyright );
  topWidget->setLayout( topLayout );

  //	QTextEdit *textEdit = new QTextEdit(tr("<p>This program is free "
  //	    "software; you can redistribute it and/or modify it under the "
  //	    "terms of the GNU General Public License as published by the "
  //	    "Free Software Foundation; either version 2 of the License, "
  //	    "or (at your option) any later version.</p>"
  //	    "<p>This program is distributed in the hope that it will "
  //	    "be useful, but WITHOUT ANY WARRANTY; without even the implied "
  //	    "warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  "
  //	    "See the GNU General Public License for more details.</p>"));
  //	textEdit->setReadOnly(true);
  QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok );
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );

  QVBoxLayout* mainLayout = new QVBoxLayout( this );
  mainLayout->addWidget( topWidget );
  // mainLayout->addWidget(textEdit);
  mainLayout->addWidget( buttonBox );
  mainLayout->setSpacing( 10 );
  buttonBox->setFocus();

  setWindowTitle( tr( "About %1" ).arg( QApplication::applicationName() ) );

  // Prevent window resize
  this->layout()->setSizeConstraint( QLayout::SetFixedSize );
}

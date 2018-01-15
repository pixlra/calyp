/*    This file is a part of PlaYUVer project
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
 * \file     PlaYUVerMdiSubWindow.cpp
 * \brief    Re-implementation QMdiSubWindow
 */

#include "PlaYUVerMdiSubWindow.h"

#include "SubWindowAbstract.h"

#include <QCloseEvent>

PlaYUVerMdiSubWindow::PlaYUVerMdiSubWindow( QWidget* parent )
    : QMdiSubWindow( parent )
{
  setAttribute( Qt::WA_DeleteOnClose );
  setBackgroundRole( QPalette::Background );
}

QSize PlaYUVerMdiSubWindow::sizeHint() const
{
  QSize maxSize;
  if( parent() )
    maxSize = parentWidget()->size();
  QSize sizeHint = qobject_cast<SubWindowAbstract*>( widget() )->sizeHint( maxSize );
  return sizeHint;
}

Void PlaYUVerMdiSubWindow::closeEvent( QCloseEvent* event )
{
  emit aboutToClose( this );
  event->accept();
}

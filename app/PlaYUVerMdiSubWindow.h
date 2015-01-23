/*    This file is a part of plaYUVer project
 *    Copyright (C) 2014-2015  by Luis Lucas      (luisfrlucas@gmail.com)
 *                                Joao Carreira   (jfmcarreira@gmail.com)
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
 * \file     PlaYUVerMdiSubWindow.h
 * \brief    Re-implementation QMdiSubWindow
 */

#ifndef __PLAYUVERMDISUBWINDOW_H__
#define __PLAYUVERMDISUBWINDOW_H__

#include "config.h"
#include "lib/PlaYUVerDefs.h"
#if( QT_VERSION_PLAYUVER == 5 )
#include <QtWidgets>
#elif( QT_VERSION_PLAYUVER == 4 )
#include <QtGui>
#endif

namespace plaYUVer
{

class PlaYUVerMdiSubWindow: public QMdiSubWindow
{
Q_OBJECT

public:
  PlaYUVerMdiSubWindow( QWidget *parent = 0 );

Q_SIGNALS:
  void aboutToClose( PlaYUVerMdiSubWindow* );

protected:
  QSize sizeHint() const;
  void closeEvent( QCloseEvent *event );

};

}  // NAMESPACE

#endif // __PLAYUVERMDISUBWINDOW_H__


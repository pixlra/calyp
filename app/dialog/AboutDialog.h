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
 * \file     AboutDialog.h
 * \brief    About Dialog
 *           Based on the work of Glad Deschrijver <glad.deschrijver@gmail.com>
 * in KTikZ project
 */

#ifndef __ABOUTDIALOG_H__
#define __ABOUTDIALOG_H__

#include <QDialog>

class AboutDialog : public QDialog
{
  Q_OBJECT

public:
  AboutDialog( QWidget* parent = 0 );
};

#endif  // __ABOUTDIALOG_H__

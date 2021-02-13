/*    This file is a part of Calyp project
 *    Copyright (C) 2014-2019  by Joao Carreira   (jfmcarreira@gmail.com)
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
 * \file     SeekStreamDialog.h
 * \brief    Dialog box to config modules opts
 */

#ifndef __SEEKSTREAMDIALOG_H__
#define __SEEKSTREAMDIALOG_H__

#include <QDialog>
#include <QVector>

#include "CalypAppModuleIf.h"
#include "CommonDefs.h"
#include "config.h"

class QSpinBox;

/**
 *
 */
class SeekStreamDialog : public QDialog
{
  Q_OBJECT

public:
  SeekStreamDialog( CalypStream* pcCurrStream, QWidget* parent = 0 );
  int runDialog();

private:
  QSpinBox* m_spinFrameNum;
  int m_iSelectedFrameNum;
};

#endif  // __MODULEHANDLEOPTDIALOG_H__

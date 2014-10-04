/*    This file is a part of plaYUVer project
 *    Copyright (C) 2014  by Luis Lucas      (luisfrlucas@gmail.com)
 *                           Joao Carreira   (jfmcarreira@gmail.com)
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
 * \file     QualityMeasurementSidebar.h
 * \brief    Definition of the quality measurement sidebar
 */

#ifndef __QUALITYMEASUREMENTSIDEBAR_H__
#define __QUALITYMEASUREMENTSIDEBAR_H__

#include "config.h"
#include "PlaYUVerDefs.h"
#if( QT_VERSION_PLAYUVER == 5 )
#include <QtWidgets>
#elif( QT_VERSION_PLAYUVER == 4 )
#include <QtGui>
#include <QWidget>
#endif
#include "PlaYUVerStream.h"
#include "PlaYUVerFrame.h"
#include "HistogramWidget.h"

namespace plaYUVer
{

class SubWindowHandle;

class QualityMeasurementSidebar: public QWidget
{
Q_OBJECT
public:
  QualityMeasurementSidebar( QWidget*, QMdiArea * );
  ~QualityMeasurementSidebar();

  QSize sizeHint() const;

  Void updateSubWindowList();
  Void updateCurrentWindow( SubWindowHandle * );
  Void updateSidebarData();

private:
  QMdiArea* m_pcMainWindowMdiArea;

  QStringList m_pcWindowListNames;
  QStringList m_pcSelectedWindowListNames;

  SubWindowHandle *m_pcCurrentSubWindow;

  QComboBox *m_comboBoxMetric;

  QComboBox *m_comboBoxRef;
  QComboBox *m_comboBoxRec;

  QLabel* m_ppcLabelQualityLabel[3];
  QLabel* m_ppcLabelQualityValue[3];

  HistogramWidget *histogramWidget;

private Q_SLOTS:
  void slotReferenceChanged( int );
  void slotQualityMetricChanged();

};

}   // NAMESPACE

#endif // __QUALITYMEASUREMENTSIDEBAR_H__
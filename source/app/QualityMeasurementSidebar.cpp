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
 * \file     QualityMeasurementSidebar.cpp
 * \brief    Definition of the quality measurement sidebar
 */

#include "PlaYUVerDefs.h"
#include <QtGui>
#include "QualityMeasurementSidebar.h"
#include "SubWindowHandle.h"

namespace plaYUVer
{

QualityMeasurementSidebar::QualityMeasurementSidebar( QWidget* parent, QMdiArea *mdiArea ) :
        QWidget( parent ),
        m_pcMainWindowMdiArea( mdiArea ),
        m_pcCurrentSubWindow( NULL )
{

  // Side bar area -----------------------------------------------------

  QLabel *MetricLabel = new QLabel( tr( "Metric:" ) );
  MetricLabel->setAlignment( Qt::AlignVCenter | Qt::AlignRight );

  QLabel *RefLabel = new QLabel( tr( "Ref:" ) );
  RefLabel->setAlignment( Qt::AlignVCenter | Qt::AlignRight );

  QLabel *RecLabel = new QLabel( tr( "Rec:" ) );
  RecLabel->setAlignment( Qt::AlignVCenter | Qt::AlignRight );

  m_comboBoxMetric = new QComboBox;
  m_comboBoxMetric->insertItems( 0, PlaYUVerFrame::supportedQualityMetricsList());
  m_comboBoxMetric->setSizeAdjustPolicy( QComboBox::AdjustToContents );
  m_comboBoxMetric->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed);

  m_comboBoxRef = new QComboBox;
  m_comboBoxRef->setSizeAdjustPolicy( QComboBox::AdjustToMinimumContentsLength );
  m_comboBoxRef->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed);
  updateSubWindowList();

  QGridLayout *mainLayout = new QGridLayout;
  mainLayout->addWidget( MetricLabel, 0, 0, Qt::AlignLeft );
  mainLayout->addWidget( m_comboBoxMetric, 0, 1, Qt::AlignLeft );
  mainLayout->addWidget( RefLabel, 1, 0, Qt::AlignLeft );
  mainLayout->addWidget( m_comboBoxRef, 1, 1, Qt::AlignLeft );

  m_ppcLabelQualityLabel[LUMA] = new QLabel( "PSNR Y:" );
  m_ppcLabelQualityLabel[LUMA]->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  m_ppcLabelQualityValue[LUMA] = new QLabel;
  m_ppcLabelQualityValue[LUMA]->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

  m_ppcLabelQualityLabel[CHROMA_U] = new QLabel( "PSNR U:" );
  m_ppcLabelQualityLabel[CHROMA_U]->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  m_ppcLabelQualityValue[CHROMA_U] = new QLabel;
  m_ppcLabelQualityValue[CHROMA_U]->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

  m_ppcLabelQualityLabel[CHROMA_V] = new QLabel( "PSNR V:" );
  m_ppcLabelQualityLabel[CHROMA_V]->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  m_ppcLabelQualityValue[CHROMA_V] = new QLabel;
  m_ppcLabelQualityValue[CHROMA_V]->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

  QGridLayout *qualityResultsLayout = new QGridLayout;
  qualityResultsLayout->addWidget( m_ppcLabelQualityLabel[LUMA], 0, 0 );
  qualityResultsLayout->addWidget( m_ppcLabelQualityValue[LUMA], 0, 1 );
  qualityResultsLayout->addWidget( m_ppcLabelQualityLabel[CHROMA_U], 1, 0 );
  qualityResultsLayout->addWidget( m_ppcLabelQualityValue[CHROMA_U], 1, 1 );
  qualityResultsLayout->addWidget( m_ppcLabelQualityLabel[CHROMA_V], 2, 0 );
  qualityResultsLayout->addWidget( m_ppcLabelQualityValue[CHROMA_V], 2, 1 );

  //QGroupBox *statisticsGroup = new QGroupBox( tr( "Quality" ) );
  QGroupBox *statisticsGroup = new QGroupBox;
  statisticsGroup->setLayout( qualityResultsLayout );
  statisticsGroup->setWhatsThis( tr( "<p>Here you can see the quality results calculated for the "
      "selected windows." ) );

  mainLayout->addWidget( statisticsGroup, 4, 0, 3, 3 );

  mainLayout->setRowStretch( 8, 10 );
  setLayout( mainLayout );

  connect( m_comboBoxRef, SIGNAL( activated(int) ), this, SLOT( slotReferenceChanged(int) ) );
  connect( m_comboBoxMetric, SIGNAL( currentIndexChanged(int) ), this, SLOT( slotQualityMetricChanged() ) );

}

QualityMeasurementSidebar::~QualityMeasurementSidebar()
{

}

QSize QualityMeasurementSidebar::sizeHint() const
{
  QSize currSize = size();
  QSize bestSize( 240, currSize.height() );
  if( currSize.width() < bestSize.width() )
    return bestSize;
  else
    return currSize;
}

Void QualityMeasurementSidebar::updateSubWindowList()
{
  SubWindowHandle *subWindow;
  QString currSubWindowName;
  m_pcWindowListNames.clear();
  for( Int i = 0; i < m_pcMainWindowMdiArea->subWindowList().size(); i++ )
  {
    subWindow = qobject_cast<SubWindowHandle *>( m_pcMainWindowMdiArea->subWindowList().at( i ) );
    if( subWindow )
    {
      currSubWindowName = subWindow->getWindowShortName();
      if( !m_pcSelectedWindowListNames.contains( currSubWindowName ) && !subWindow->getIsModule() )
      {
        m_pcWindowListNames.append( currSubWindowName );
        //m_comboBoxRef->setItemText( currIdx++, currSubWindowName );
      }
    }
  }
  m_comboBoxRef->clear();
  m_comboBoxRef->insertItems( -1, m_pcWindowListNames );
  m_comboBoxRef->setCurrentIndex( -1 );
  if( m_pcCurrentSubWindow )
  {
    if( SubWindowHandle* refSubWindow = m_pcCurrentSubWindow->getRefSubWindow() )
    {
      Int index = m_pcWindowListNames.indexOf( refSubWindow->getWindowShortName() );
      m_comboBoxRef->setCurrentIndex( index );
    }
  }
  m_comboBoxRef->setSizeAdjustPolicy( QComboBox::AdjustToContents );
}

Void QualityMeasurementSidebar::updateCurrentWindow( SubWindowHandle *subWindow )
{
  m_pcCurrentSubWindow = subWindow;
  updateSidebarData();
}

Void QualityMeasurementSidebar::updateSidebarData()
{
  QString value( "0.00" );
  if( m_pcCurrentSubWindow )
  {
    if( SubWindowHandle* refSubWindow = m_pcCurrentSubWindow->getRefSubWindow() )
    {
      Int index = m_pcWindowListNames.indexOf( refSubWindow->getWindowName() );
      m_comboBoxRef->setCurrentIndex( index );

      PlaYUVerFrame* currFrame = m_pcCurrentSubWindow->getCurrFrame();
      PlaYUVerFrame* refFrame = refSubWindow->getCurrFrame();
      Double quality;

      for( Int component = 0; component < 3; component++ )
      {
        quality = currFrame->getQuality( m_comboBoxMetric->currentIndex(), refFrame, component );
        m_ppcLabelQualityValue[component]->setText( value.setNum( quality, 'f', 2 ) );
      }
      return;
    }
    else
    {
      m_comboBoxRef->setCurrentIndex( -1 );
    }
  }
  for( Int component = 0; component < 3; component++ )
  {
    m_ppcLabelQualityValue[component]->setText( value );
  }

}

Void QualityMeasurementSidebar::slotReferenceChanged( Int index )
{
  if( index > -1 && m_pcCurrentSubWindow )
  {
    SubWindowHandle *subWindow = qobject_cast<SubWindowHandle *>( m_pcMainWindowMdiArea->subWindowList().at( index ) );
    if( subWindow )
    {
      if( m_pcCurrentSubWindow->getCurrFrame()->haveSameFmt( subWindow->getCurrFrame() ) )
      {
        m_pcCurrentSubWindow->setRefSubWindow( subWindow );
        updateSidebarData();
        m_comboBoxRef->setCurrentIndex( index );
        return;
      }
    }
  }
  else
  {
    m_comboBoxRef->setCurrentIndex( -1 );
  }
}

Void QualityMeasurementSidebar::slotQualityMetricChanged()
{
  QString labelQuality = PlaYUVerFrame::supportedQualityMetricsList().at( m_comboBoxMetric->currentIndex() );
  m_ppcLabelQualityLabel[LUMA]->setText( labelQuality + " Y");
  m_ppcLabelQualityLabel[CHROMA_U]->setText( labelQuality + " U");
  m_ppcLabelQualityLabel[CHROMA_V]->setText( labelQuality + " V");
  updateSidebarData();
}
}   // NAMESPACE

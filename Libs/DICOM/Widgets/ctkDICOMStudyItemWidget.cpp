/*=========================================================================

  Library:   CTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
  and development was supported by the Center for Intelligent Image-guided Interventions (CI3).

=========================================================================*/

//Qt includes
#include <QDebug>
#include <QLabel>
#include <QTableWidgetItem>

// CTK includes
#include <ctkLogger.h>

// ctkDICOMCore includes
#include "ctkDICOMDatabase.h"
#include "ctkDICOMScheduler.h"
#include "ctkDICOMJobResponseSet.h"

// ctkDICOMWidgets includes
#include "ctkDICOMSeriesItemWidget.h"
#include "ctkDICOMStudyItemWidget.h"
#include "ui_ctkDICOMStudyItemWidget.h"

#include <math.h>

static ctkLogger logger("org.commontk.DICOM.Widgets.ctkDICOMStudyItemWidget");

//----------------------------------------------------------------------------
static void skipDelete(QObject* obj)
{
  Q_UNUSED(obj);
  // this deleter does not delete the object from memory
  // useful if the pointer is not owned by the smart pointer
}

//----------------------------------------------------------------------------
class ctkDICOMStudyItemWidgetPrivate: public Ui_ctkDICOMStudyItemWidget
{
  Q_DECLARE_PUBLIC(ctkDICOMStudyItemWidget);

protected:
  ctkDICOMStudyItemWidget* const q_ptr;

public:
  ctkDICOMStudyItemWidgetPrivate(ctkDICOMStudyItemWidget& obj);
  ~ctkDICOMStudyItemWidgetPrivate();

  void init(QWidget* parentWidget);
  void updateColumnsWidths();
  void createSeries();
  void addEmptySeriesItemWidget(const int& rowIndex,
                                const int& columnIndex);
  bool isSeriesItemAlreadyAdded(const QString& seriesItem);

  QString FilteringSeriesDescription;
  QStringList FilteringModalities;

  QSharedPointer<ctkDICOMDatabase> DicomDatabase;
  QSharedPointer<ctkDICOMScheduler> Scheduler;
  QSharedPointer<QWidget> VisualDICOMBrowser;

  int ThumbnailSize;
  QString PatientID;
  QString StudyInstanceUID;
  QString StudyItem;
};

//----------------------------------------------------------------------------
// ctkDICOMStudyItemWidgetPrivate methods

//----------------------------------------------------------------------------
ctkDICOMStudyItemWidgetPrivate::ctkDICOMStudyItemWidgetPrivate(ctkDICOMStudyItemWidget& obj)
  : q_ptr(&obj)
{
  this->ThumbnailSize = 300;
  this->FilteringSeriesDescription = "";
  this->PatientID = "";
  this->StudyInstanceUID = "";
  this->StudyItem = "";

  this->DicomDatabase = nullptr;
  this->Scheduler = nullptr;
  this->VisualDICOMBrowser = nullptr;
}

//----------------------------------------------------------------------------
ctkDICOMStudyItemWidgetPrivate::~ctkDICOMStudyItemWidgetPrivate()
{
  Q_Q(ctkDICOMStudyItemWidget);

  for (int row = 0; row < this->SeriesListTableWidget->rowCount(); row++)
    {
    for (int column = 0 ; column < this->SeriesListTableWidget->columnCount(); column++)
      {
      ctkDICOMSeriesItemWidget* seriesItemWidget =
        qobject_cast<ctkDICOMSeriesItemWidget*>(this->SeriesListTableWidget->cellWidget(row, column));
      if (!seriesItemWidget)
        {
        continue;
        }

      q->disconnect(seriesItemWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
                    this->VisualDICOMBrowser.data(), SLOT(showSeriesContextMenu(const QPoint&)));
      }
    }
}

//----------------------------------------------------------------------------
void ctkDICOMStudyItemWidgetPrivate::init(QWidget* parentWidget)
{
  Q_Q(ctkDICOMStudyItemWidget);
  this->setupUi(q);

  this->VisualDICOMBrowser = QSharedPointer<QWidget>(parentWidget, skipDelete);

  this->StudyDescriptionTextBrowser->hide();
  this->StudyDescriptionTextBrowser->setReadOnly(true);
  this->StudyItemCollapsibleGroupBox->setCollapsed(false);

   q->connect(this->StudySelectionCheckBox, SIGNAL(clicked(bool)),
              q, SLOT(onStudySelectionClicked(bool)));
}

//------------------------------------------------------------------------------
void ctkDICOMStudyItemWidgetPrivate::updateColumnsWidths()
{
  for (int i = 0; i < this->SeriesListTableWidget->columnCount(); ++i)
    {
    this->SeriesListTableWidget->setColumnWidth(i, this->ThumbnailSize);
    }
}

//------------------------------------------------------------------------------
void ctkDICOMStudyItemWidgetPrivate::createSeries()
{
  Q_Q(ctkDICOMStudyItemWidget);

  if (!this->DicomDatabase)
    {
    logger.error("createSeries failed, no DICOM Database has been set. \n");
    return;
    }

  QStringList seriesList = this->DicomDatabase->seriesForStudy(this->StudyInstanceUID);
  if (seriesList.count() == 0)
    {
    return;
    }

  // Sort by SeriesNumber
  QMap<int, QString> seriesMap;
  foreach (QString seriesItem, seriesList)
    {
    if (this->isSeriesItemAlreadyAdded(seriesItem))
      {
      continue;
      }

    QString modality = this->DicomDatabase->fieldForSeries("Modality", seriesItem);
    QString seriesDescription = this->DicomDatabase->fieldForSeries("SeriesDescription", seriesItem);
    // Filter with modality and seriesDescription
    if ((this->FilteringSeriesDescription.isEmpty() ||
         seriesDescription.contains(this->FilteringSeriesDescription, Qt::CaseInsensitive)) &&
        (this->FilteringModalities.contains("Any") || this->FilteringModalities.contains(modality)))
      {
      int seriesNumber = this->DicomDatabase->fieldForSeries("SeriesNumber", seriesItem).toInt();
      while (seriesMap.contains(seriesNumber))
        {
        seriesNumber++;
        }
      // QMap automatically sort in ascending with the key
      seriesMap[seriesNumber] = seriesItem;
      }
    }

  int tableIndex = 0;
  int seriesIndex = 0;
  int numberOfSeries = seriesMap.count();
  foreach (QString seriesItem, seriesMap)
    {
    QString seriesInstanceUID = this->DicomDatabase->fieldForSeries("SeriesInstanceUID", seriesItem);
    if (seriesInstanceUID.isEmpty())
      {
      numberOfSeries--;
      continue;
      }
    seriesIndex++;

    QString modality = this->DicomDatabase->fieldForSeries("Modality", seriesItem);
    QString seriesDescription = this->DicomDatabase->fieldForSeries("SeriesDescription", seriesItem);

    q->addSeriesItemWidget(tableIndex, seriesItem, seriesInstanceUID, modality, seriesDescription);
    tableIndex++;

    if (seriesIndex == numberOfSeries)
      {
      int emptyIndex = tableIndex;
      int columnIndex = emptyIndex % this->SeriesListTableWidget->columnCount();
      while (columnIndex != 0)
        {
        int rowIndex = floor(emptyIndex / this->SeriesListTableWidget->columnCount());
        columnIndex = emptyIndex % this->SeriesListTableWidget->columnCount();
        this->addEmptySeriesItemWidget(rowIndex, columnIndex);
        emptyIndex++;
        }
      }

    int iHeight = 0;
    for (int rowIndex = 0; rowIndex < this->SeriesListTableWidget->rowCount(); ++rowIndex)
      {
      iHeight += this->SeriesListTableWidget->verticalHeader()->sectionSize(rowIndex);
      }
    if (iHeight < this->ThumbnailSize)
      {
      iHeight = this->ThumbnailSize;
      }
    iHeight += 25;
    this->SeriesListTableWidget->setMinimumHeight(iHeight);
    }
}

//------------------------------------------------------------------------------
void ctkDICOMStudyItemWidgetPrivate::addEmptySeriesItemWidget(const int& rowIndex,
                                                              const int& columnIndex)
{
  QTableWidgetItem *tableItem = new QTableWidgetItem;
  tableItem->setFlags(Qt::NoItemFlags);
  tableItem->setSizeHint(QSize(this->ThumbnailSize, this->ThumbnailSize));

  this->SeriesListTableWidget->setItem(rowIndex, columnIndex, tableItem);
}

//------------------------------------------------------------------------------
bool ctkDICOMStudyItemWidgetPrivate::isSeriesItemAlreadyAdded(const QString &seriesItem)
{
  bool alreadyAdded = false;
  for (int i = 0; i < this->SeriesListTableWidget->rowCount(); i++)
    {
    for (int j = 0 ; j < this->SeriesListTableWidget->columnCount(); j++)
      {
      ctkDICOMSeriesItemWidget* seriesItemWidget =
        qobject_cast<ctkDICOMSeriesItemWidget*>(this->SeriesListTableWidget->cellWidget(i, j));
      if (!seriesItemWidget)
        {
        continue;
        }

      if (seriesItemWidget->seriesItem() == seriesItem)
        {
        alreadyAdded = true;
        break;
        }
      }

    if (alreadyAdded)
      {
      break;
      }
    }

  return alreadyAdded;
}

//----------------------------------------------------------------------------
// ctkDICOMStudyItemWidget methods

//----------------------------------------------------------------------------
ctkDICOMStudyItemWidget::ctkDICOMStudyItemWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new ctkDICOMStudyItemWidgetPrivate(*this))
{
  Q_D(ctkDICOMStudyItemWidget);
  d->init(parentWidget);
}

//----------------------------------------------------------------------------
ctkDICOMStudyItemWidget::~ctkDICOMStudyItemWidget()
{
}

//----------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::setStudyItem(const QString &studyItem)
{
  Q_D(ctkDICOMStudyItemWidget);
  d->StudyItem = studyItem;
}

//----------------------------------------------------------------------------
QString ctkDICOMStudyItemWidget::studyItem() const
{
  Q_D(const ctkDICOMStudyItemWidget);
  return d->StudyItem;
}

//------------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::setPatientID(const QString &patientID)
{
  Q_D(ctkDICOMStudyItemWidget);
  d->PatientID = patientID;
}

//------------------------------------------------------------------------------
QString ctkDICOMStudyItemWidget::patientID() const
{
  Q_D(const ctkDICOMStudyItemWidget);
  return d->PatientID;
}

//----------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::setStudyInstanceUID(const QString& studyInstanceUID)
{
  Q_D(ctkDICOMStudyItemWidget);
  d->StudyInstanceUID = studyInstanceUID;
}

//------------------------------------------------------------------------------
QString ctkDICOMStudyItemWidget::studyInstanceUID() const
{
  Q_D(const ctkDICOMStudyItemWidget);
  return d->StudyInstanceUID;
}

//----------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::setTitle(const QString& title)
{
  Q_D(ctkDICOMStudyItemWidget);
  d->StudyItemCollapsibleGroupBox->setTitle(title);
}

//------------------------------------------------------------------------------
QString ctkDICOMStudyItemWidget::title() const
{
  Q_D(const ctkDICOMStudyItemWidget);
  return d->StudyItemCollapsibleGroupBox->title();
}

//----------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::setDescription(const QString& description)
{
  Q_D(ctkDICOMStudyItemWidget);
  if (description.isEmpty())
    {
    d->StudyDescriptionTextBrowser->hide();
    }
  else
    {
    d->StudyDescriptionTextBrowser->setText(description);
    d->StudyDescriptionTextBrowser->show();
    }
}

//------------------------------------------------------------------------------
QString ctkDICOMStudyItemWidget::description() const
{
  Q_D(const ctkDICOMStudyItemWidget);
  return d->StudyDescriptionTextBrowser->toPlainText();
}

//----------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::setCollapsed(bool collapsed)
{
  Q_D(ctkDICOMStudyItemWidget);
  d->StudyItemCollapsibleGroupBox->setCollapsed(collapsed);
}

//------------------------------------------------------------------------------
bool ctkDICOMStudyItemWidget::collapsed()const
{
  Q_D(const ctkDICOMStudyItemWidget);
  return d->StudyItemCollapsibleGroupBox->collapsed();
}

//----------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::setNumberOfSeriesPerRow(int numberOfSeriesPerRow)
{
  Q_D(ctkDICOMStudyItemWidget);
  d->SeriesListTableWidget->setColumnCount(numberOfSeriesPerRow);
  d->updateColumnsWidths();
}

//------------------------------------------------------------------------------
int ctkDICOMStudyItemWidget::numberOfSeriesPerRow() const
{
  Q_D(const ctkDICOMStudyItemWidget);
  return d->SeriesListTableWidget->columnCount();
}

//----------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::setThumbnailSize(int thumbnailSize)
{
  Q_D(ctkDICOMStudyItemWidget);
  d->ThumbnailSize = thumbnailSize;
  d->updateColumnsWidths();
}

//------------------------------------------------------------------------------
int ctkDICOMStudyItemWidget::thumbnailSize() const
{
  Q_D(const ctkDICOMStudyItemWidget);
  return d->ThumbnailSize;
}

//------------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::setSelection(bool selected)
{
  Q_D(const ctkDICOMStudyItemWidget);
  if (selected)
    {
    d->SeriesListTableWidget->selectAll();
    }
  else
    {
    d->SeriesListTableWidget->clearSelection();
    }

  d->StudySelectionCheckBox->setChecked(selected);
}

//------------------------------------------------------------------------------
bool ctkDICOMStudyItemWidget::selection() const
{
  Q_D(const ctkDICOMStudyItemWidget);
  return d->StudySelectionCheckBox->isChecked();
}

//----------------------------------------------------------------------------
ctkDICOMScheduler* ctkDICOMStudyItemWidget::scheduler()const
{
  Q_D(const ctkDICOMStudyItemWidget);
  return d->Scheduler.data();
}

//----------------------------------------------------------------------------
QSharedPointer<ctkDICOMScheduler> ctkDICOMStudyItemWidget::schedulerShared()const
{
  Q_D(const ctkDICOMStudyItemWidget);
  return d->Scheduler;
}

//----------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::setScheduler(ctkDICOMScheduler& scheduler)
{
  Q_D(ctkDICOMStudyItemWidget);
  if (d->Scheduler)
    {
    QObject::disconnect(d->Scheduler.data(), SIGNAL(progressJobDetail(QVariant)),
                       this, SLOT(updateGUIFromScheduler(QVariant)));
    }

  d->Scheduler = QSharedPointer<ctkDICOMScheduler>(&scheduler, skipDelete);

  if (d->Scheduler)
    {
    QObject::connect(d->Scheduler.data(), SIGNAL(progressJobDetail(QVariant)),
                     this, SLOT(updateGUIFromScheduler(QVariant)));
    }
}

//----------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::setScheduler(QSharedPointer<ctkDICOMScheduler> scheduler)
{
  Q_D(ctkDICOMStudyItemWidget);
  if (d->Scheduler)
    {
    QObject::disconnect(d->Scheduler.data(), SIGNAL(progressJobDetail(QVariant)),
                       this, SLOT(updateGUIFromScheduler(QVariant)));
    }

  d->Scheduler = scheduler;

  if (d->Scheduler)
    {
    QObject::connect(d->Scheduler.data(), SIGNAL(progressJobDetail(QVariant)),
                     this, SLOT(updateGUIFromScheduler(QVariant)));
    }
}

//----------------------------------------------------------------------------
ctkDICOMDatabase* ctkDICOMStudyItemWidget::dicomDatabase()const
{
  Q_D(const ctkDICOMStudyItemWidget);
  return d->DicomDatabase.data();
}

//----------------------------------------------------------------------------
QSharedPointer<ctkDICOMDatabase> ctkDICOMStudyItemWidget::dicomDatabaseShared()const
{
  Q_D(const ctkDICOMStudyItemWidget);
  return d->DicomDatabase;
}

//----------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::setDicomDatabase(ctkDICOMDatabase& dicomDatabase)
{
  Q_D(ctkDICOMStudyItemWidget);
  d->DicomDatabase = QSharedPointer<ctkDICOMDatabase>(&dicomDatabase, skipDelete);
}

//----------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::setDicomDatabase(QSharedPointer<ctkDICOMDatabase> dicomDatabase)
{
  Q_D(ctkDICOMStudyItemWidget);
  d->DicomDatabase = dicomDatabase;
}

//------------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::setFilteringSeriesDescription(const QString& filteringSeriesDescription)
{
  Q_D(ctkDICOMStudyItemWidget);
  d->FilteringSeriesDescription = filteringSeriesDescription;
}

//------------------------------------------------------------------------------
QString ctkDICOMStudyItemWidget::filteringSeriesDescription() const
{
  Q_D(const ctkDICOMStudyItemWidget);
  return d->FilteringSeriesDescription;
}

//------------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::setFilteringModalities(const QStringList &filteringModalities)
{
  Q_D(ctkDICOMStudyItemWidget);
  d->FilteringModalities = filteringModalities;
}

//------------------------------------------------------------------------------
QStringList ctkDICOMStudyItemWidget::filteringModalities() const
{
  Q_D(const ctkDICOMStudyItemWidget);
  return d->FilteringModalities;
}

//------------------------------------------------------------------------------
QTableWidget *ctkDICOMStudyItemWidget::seriesListTableWidget()
{
  Q_D(ctkDICOMStudyItemWidget);
  return d->SeriesListTableWidget;
}

//------------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::addSeriesItemWidget(const int& tableIndex,
                                                  const QString &seriesItem,
                                                  const QString &seriesInstanceUID,
                                                  const QString &modality,
                                                  const QString &seriesDescription)
{
  Q_D(ctkDICOMStudyItemWidget);
  if (!d->DicomDatabase)
    {
    logger.error("addSeriesItemWidget failed, no DICOM Database has been set. \n");
    return;
    }

  QString seriesNumber = d->DicomDatabase->fieldForSeries("SeriesNumber", seriesItem);
  ctkDICOMSeriesItemWidget* seriesItemWidget = new ctkDICOMSeriesItemWidget;
  seriesItemWidget->setSeriesItem(seriesItem);
  seriesItemWidget->setPatientID(d->PatientID);
  seriesItemWidget->setStudyInstanceUID(d->StudyInstanceUID);
  seriesItemWidget->setSeriesInstanceUID(seriesInstanceUID);
  seriesItemWidget->setSeriesNumber(seriesNumber);
  seriesItemWidget->setModality(modality);
  seriesItemWidget->setSeriesDescription(seriesDescription);
  seriesItemWidget->setThumbnailSize(d->ThumbnailSize);
  seriesItemWidget->setDicomDatabase(d->DicomDatabase);
  seriesItemWidget->setScheduler(d->Scheduler);
  seriesItemWidget->generateInstances();
  seriesItemWidget->setContextMenuPolicy(Qt::CustomContextMenu);

  this->connect(seriesItemWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
                d->VisualDICOMBrowser.data(), SLOT(showSeriesContextMenu(const QPoint&)));

  QTableWidgetItem *tableItem = new QTableWidgetItem;
  tableItem->setSizeHint(QSize(d->ThumbnailSize, d->ThumbnailSize));

  int rowIndex = floor(tableIndex / d->SeriesListTableWidget->columnCount());
  int columnIndex = tableIndex % d->SeriesListTableWidget->columnCount();
  if (columnIndex == 0)
    {
    d->SeriesListTableWidget->insertRow(rowIndex);
    d->SeriesListTableWidget->setRowHeight(rowIndex, d->ThumbnailSize + 30);
    }

  d->SeriesListTableWidget->setItem(rowIndex, columnIndex, tableItem);
  d->SeriesListTableWidget->setCellWidget(rowIndex, columnIndex, seriesItemWidget);
}

//------------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::removeSeriesItemWidget(const QString& seriesItem)
{
  Q_D(ctkDICOMStudyItemWidget);

  for (int row = 0; row < d->SeriesListTableWidget->rowCount(); row++)
    {
    for (int column = 0 ; column < d->SeriesListTableWidget->columnCount(); column++)
      {
      ctkDICOMSeriesItemWidget* seriesItemWidget =
        qobject_cast<ctkDICOMSeriesItemWidget*>(d->SeriesListTableWidget->cellWidget(row, column));
      if (!seriesItemWidget || seriesItemWidget->seriesItem() != seriesItem)
        {
        continue;
        }

      d->SeriesListTableWidget->removeCellWidget(row, column);
      this->disconnect(seriesItemWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
                       d->VisualDICOMBrowser.data(), SLOT(showSeriesContextMenu(const QPoint&)));
      delete seriesItemWidget;
      QTableWidgetItem *tableItem = d->SeriesListTableWidget->item(row, column);
      delete tableItem;

      d->addEmptySeriesItemWidget(row, column);
      break;
      }
  }
}

//------------------------------------------------------------------------------
ctkCollapsibleGroupBox *ctkDICOMStudyItemWidget::collapsibleGroupBox()
{
  Q_D(ctkDICOMStudyItemWidget);
  return d->StudyItemCollapsibleGroupBox;
}

//------------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::generateSeries(bool toggled)
{
  Q_D(ctkDICOMStudyItemWidget);
  if (!toggled)
    {
    return;
    }

  d->createSeries();

  if (d->Scheduler && d->Scheduler->getNumberOfQueryRetrieveServers() > 0)
    {
    d->Scheduler->querySeries(d->PatientID,
                             d->StudyInstanceUID,
                             QThread::NormalPriority);
    }
}

//------------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::updateGUIFromScheduler(QVariant data)
{
  Q_D(ctkDICOMStudyItemWidget);

  ctkJobDetail td = data.value<ctkJobDetail>();
    if (td.JobUID.isEmpty())
    {
    d->createSeries();
    }

  if (td.JobUID.isEmpty() ||
      td.TypeOfJob != ctkDICOMJobResponseSet::JobType::QuerySeries ||
      td.PatientID != d->PatientID ||
      td.StudyInstanceUID != d->StudyInstanceUID)
    {
    return;
    }

  d->createSeries();
}

//------------------------------------------------------------------------------
void ctkDICOMStudyItemWidget::onStudySelectionClicked(bool toggled)
{
  Q_D(ctkDICOMStudyItemWidget);

  this->setSelection(toggled);
}

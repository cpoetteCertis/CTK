﻿/*=========================================================================

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

#ifndef __ctkDICOMStudyItemWidget_h
#define __ctkDICOMStudyItemWidget_h

#include "ctkDICOMWidgetsExport.h"

// Qt includes 
#include <QWidget>
#include <QVariant>

class ctkCollapsibleGroupBox;
class ctkDICOMStudyItemWidgetPrivate;
class ctkDICOMDatabase;
class ctkDICOMScheduler;

class QTableWidget;

/// \ingroup DICOM_Widgets
class CTK_DICOM_WIDGETS_EXPORT ctkDICOMStudyItemWidget : public QWidget
{
  Q_OBJECT;
  Q_PROPERTY(QString studyItem READ studyItem WRITE setStudyItem);
  Q_PROPERTY(QString patientID READ patientID WRITE setPatientID);
  Q_PROPERTY(QString studyInstanceUID READ studyInstanceUID WRITE setStudyInstanceUID);
  Q_PROPERTY(QString title READ title WRITE setTitle);
  Q_PROPERTY(QString description READ description WRITE setDescription);
  Q_PROPERTY(bool collapsed READ collapsed WRITE setCollapsed);
  Q_PROPERTY(int numberOfSeriesPerRow READ numberOfSeriesPerRow WRITE setNumberOfSeriesPerRow);
  Q_PROPERTY(int thumbnailSize READ thumbnailSize WRITE setThumbnailSize);

public:
  typedef QWidget Superclass;
  explicit ctkDICOMStudyItemWidget(QWidget* parent = nullptr);
  virtual ~ctkDICOMStudyItemWidget();

  /// Study item
  void setStudyItem(const QString& studyItem);
  QString studyItem() const;

  /// Patient ID
  void setPatientID(const QString& patientID);
  QString patientID() const;

  /// Study instance UID
  void setStudyInstanceUID(const QString& studyInstanceUID);
  QString studyInstanceUID() const;

  /// Study title
  void setTitle(const QString& title);
  QString title() const;

  /// Study Description
  void setDescription(const QString& description);
  QString description() const;

  /// Study GroupBox collapsed
  /// False by default
  void setCollapsed(bool collapsed);
  bool collapsed() const;

  /// Number of series displayed per row
  /// 6 by default
  void setNumberOfSeriesPerRow(int numberOfSeriesPerRow);
  int numberOfSeriesPerRow() const;

  /// Series Thumbnail size
  /// 300 px by default
  void setThumbnailSize(int thumbnailSize);
  int thumbnailSize() const;

  /// Study is selected
  void setSelection(bool selected);
  bool selection() const;

  /// Query Filters
  /// Empty by default
  void setFilteringSeriesDescription(const QString& filteringSeriesDescription);
  QString filteringSeriesDescription() const;
  /// ["Any", "CR", "CT", "MR", "NM", "US", "PT", "XA"] by default
  void setFilteringModalities(const QStringList& filteringModalities);
  QStringList filteringModalities() const;

  /// Return the scheduler.
  Q_INVOKABLE ctkDICOMScheduler* scheduler() const;
  /// Return the scheduler as a shared pointer
  /// (not Python-wrappable).
  QSharedPointer<ctkDICOMScheduler> schedulerShared() const;
  /// Set the scheduler.
  Q_INVOKABLE void setScheduler(ctkDICOMScheduler& scheduler);
  /// Set the scheduler as a shared pointer
  /// (not Python-wrappable).
  void setScheduler(QSharedPointer<ctkDICOMScheduler> scheduler);

  /// Return the Dicom Database.
  Q_INVOKABLE ctkDICOMDatabase* dicomDatabase() const;
  /// Return Dicom Database as a shared pointer
  /// (not Python-wrappable).
  QSharedPointer<ctkDICOMDatabase> dicomDatabaseShared() const;
  /// Set the Dicom Database.
  Q_INVOKABLE void setDicomDatabase(ctkDICOMDatabase& dicomDatabase);
  /// Set the Dicom Database as a shared pointer
  /// (not Python-wrappable).
  void setDicomDatabase(QSharedPointer<ctkDICOMDatabase> dicomDatabase);

  /// Series list table.
  Q_INVOKABLE QTableWidget* seriesListTableWidget();

  /// Add/Remove Series item widget
  Q_INVOKABLE void addSeriesItemWidget(const int& tableIndex,
                                       const QString &seriesItem,
                                       const QString &seriesInstanceUID,
                                       const QString& modality,
                                       const QString& seriesDescription);
  Q_INVOKABLE void removeSeriesItemWidget(const QString& seriesItem);

  /// Collapsible group box.
  Q_INVOKABLE ctkCollapsibleGroupBox* collapsibleGroupBox();

public Q_SLOTS:
  void generateSeries(bool toggled = true);
  void updateGUIFromScheduler(QVariant data);
  void onStudySelectionClicked(bool);

protected:
  QScopedPointer<ctkDICOMStudyItemWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(ctkDICOMStudyItemWidget);
  Q_DISABLE_COPY(ctkDICOMStudyItemWidget);
};

#endif

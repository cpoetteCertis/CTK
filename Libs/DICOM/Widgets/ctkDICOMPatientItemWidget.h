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

#ifndef __ctkDICOMPatientItemWidget_h
#define __ctkDICOMPatientItemWidget_h

#include "ctkDICOMWidgetsExport.h"

// Qt includes 
#include <QWidget>
#include <QVariant>

class ctkDICOMPatientItemWidgetPrivate;

class ctkDICOMDatabase;
class ctkDICOMScheduler;
class ctkDICOMStudyItemWidget;

/// \ingroup DICOM_Widgets
class CTK_DICOM_WIDGETS_EXPORT ctkDICOMPatientItemWidget : public QWidget
{
  Q_OBJECT;
  Q_ENUMS(DateType)
  Q_PROPERTY(QString patientItem READ patientItem WRITE setPatientItem);
  Q_PROPERTY(QString patientID READ patientID WRITE setPatientID);
  Q_PROPERTY(int numberOfStudiesPerPatient READ numberOfStudiesPerPatient WRITE setNumberOfStudiesPerPatient);
  Q_PROPERTY(int numberOfSeriesPerRow READ numberOfSeriesPerRow WRITE setNumberOfSeriesPerRow);
  Q_PROPERTY(int minimumThumbnailSize READ minimumThumbnailSize WRITE setMinimumThumbnailSize);

public:
  typedef QWidget Superclass;
  explicit ctkDICOMPatientItemWidget(QWidget* parent = nullptr);
  virtual ~ctkDICOMPatientItemWidget();

  /// Patient item
  void setPatientItem(const QString& patientItem);
  QString patientItem() const;

  /// Patient ID
  void setPatientID(const QString& patientID);
  QString patientID() const;

  /// Query Filters
  /// Empty by default
  void setFilteringStudyDescription(const QString& filteringStudyDescription);
  QString filteringStudyDescription() const;
  /// Date filtering enum
  enum DateType
    {
    Any = 0,
    Today,
    Yesterday,
    LastWeek,
    LastMonth,
    LastYear
    };

  /// Available values:
  /// Any,
  /// Today,
  /// Yesterday,
  /// LastWeek,
  /// LastMonth,
  /// LastYear.
  /// Any by default.
  void setFilteringDate(const DateType& filteringDate);
  DateType filteringDate() const;
  /// Empty by default
  void setFilteringSeriesDescription(const QString& filteringSeriesDescription);
  QString filteringSeriesDescription() const;
  /// ["Any", "CR", "CT", "MR", "NM", "US", "PT", "XA"] by default
  void setFilteringModalities(const QStringList& filteringModalities);
  QStringList filteringModalities() const;

  /// Number of non collapsed studies per patient
  /// 2 by default
  void setNumberOfStudiesPerPatient(int numberOfStudiesPerPatient);
  int numberOfStudiesPerPatient() const;

  /// Number of series displayed per row
  /// 6 by default
  void setNumberOfSeriesPerRow(int numberOfSeriesPerRow);
  int numberOfSeriesPerRow() const;

  /// Minimum thumbnail size in pixel
  /// 300 by default
  void setMinimumThumbnailSize(int minimumThumbnailSize);
  int minimumThumbnailSize() const;

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

  /// Return all the study item widgets for the patient
  Q_INVOKABLE QList<ctkDICOMStudyItemWidget*> studyItemWidgetsList()const;

  /// Return number of days from filtering date attribute
  Q_INVOKABLE static int getNDaysFromFilteringDate(ctkDICOMPatientItemWidget::DateType filteringDate);

  /// Add/Remove study item widgets
  Q_INVOKABLE void addStudyItemWidget(const QString &studyItem);
  Q_INVOKABLE void removeStudyItemWidget(const QString &studyItem);

  /// Set selection for all studies/series
  Q_INVOKABLE void setSelection(bool selected);

public Q_SLOTS:
  void generateStudies();
  void updateGUIFromScheduler(QVariant data);
  void onSeriesItemClicked();
  void raiseSelectedSeriesJobsPriority();

protected:
  QScopedPointer<ctkDICOMPatientItemWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(ctkDICOMPatientItemWidget);
  Q_DISABLE_COPY(ctkDICOMPatientItemWidget);
};

#endif

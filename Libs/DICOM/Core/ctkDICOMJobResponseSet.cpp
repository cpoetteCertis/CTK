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

// ctkDICOMCore includes
#include "ctkDICOMItem.h"
#include "ctkDICOMJobResponseSet.h"
#include "ctkLogger.h"

// DCMTK includes
#include <dcmtk/dcmdata/dcdeftag.h>

static ctkLogger logger("org.commontk.dicom.DICOMTaskResults");

//------------------------------------------------------------------------------
class ctkDICOMJobResponseSetPrivate: public QObject
{
  Q_DECLARE_PUBLIC(ctkDICOMJobResponseSet);

protected:
  ctkDICOMJobResponseSet* const q_ptr;

public:
  ctkDICOMJobResponseSetPrivate(ctkDICOMJobResponseSet& obj);
  ~ctkDICOMJobResponseSetPrivate();

  QString FilePath;
  bool CopyFile;
  bool OverwriteExistingDataset;

  ctkDICOMJobResponseSet::JobType TypeOfJob;
  QString JobUID;
  QString PatientID;
  QString StudyInstanceUID;
  QString SeriesInstanceUID;
  QString SOPInstanceUID;
  QString ConnectionName;
  QMap<QString, QSharedPointer<ctkDICOMItem>> Datasets;
};

//------------------------------------------------------------------------------
// ctkDICOMJobResponseSetPrivate methods

//------------------------------------------------------------------------------
ctkDICOMJobResponseSetPrivate::ctkDICOMJobResponseSetPrivate(ctkDICOMJobResponseSet& obj)
  : q_ptr(&obj)
{
  this->TypeOfJob = ctkDICOMJobResponseSet::JobType::None;
  this->JobUID = "";
  this->PatientID = "";
  this->StudyInstanceUID = "";
  this->SeriesInstanceUID = "";
  this->SOPInstanceUID = "";
  this->ConnectionName = "";
  this->CopyFile = false;
  this->OverwriteExistingDataset = false;
  this->FilePath = "";
}

//------------------------------------------------------------------------------
ctkDICOMJobResponseSetPrivate::~ctkDICOMJobResponseSetPrivate()
{
  this->Datasets.clear();
}

//------------------------------------------------------------------------------
// ctkDICOMJobResponseSet methods

//------------------------------------------------------------------------------
ctkDICOMJobResponseSet::ctkDICOMJobResponseSet(QObject* parent)
  : QObject(parent),
    d_ptr(new ctkDICOMJobResponseSetPrivate(*this))
{
}

//------------------------------------------------------------------------------
ctkDICOMJobResponseSet::~ctkDICOMJobResponseSet()
{
}

//----------------------------------------------------------------------------
void ctkDICOMJobResponseSet::setFilePath(const QString &filePath)
{
  Q_D(ctkDICOMJobResponseSet);
  d->FilePath = filePath;

  if (d->FilePath.isEmpty())
    {
    return;
    }

  QSharedPointer<ctkDICOMItem> dataset =
    QSharedPointer<ctkDICOMItem>(new ctkDICOMItem);
  dataset->InitializeFromFile(filePath);

  DcmItem dcmItem = dataset->GetDcmItem();
  OFString SOPInstanceUID;
  dcmItem.findAndGetOFString(DCM_SOPInstanceUID, SOPInstanceUID);

  d->Datasets.insert(QString(SOPInstanceUID.c_str()), dataset);
}

//----------------------------------------------------------------------------
QString ctkDICOMJobResponseSet::filePath() const
{
  Q_D(const ctkDICOMJobResponseSet);
  return d->FilePath;
}

//----------------------------------------------------------------------------
void ctkDICOMJobResponseSet::setCopyFile(const bool &copyFile)
{
  Q_D(ctkDICOMJobResponseSet);
  d->CopyFile = copyFile;
}

//----------------------------------------------------------------------------
bool ctkDICOMJobResponseSet::copyFile() const
{
  Q_D(const ctkDICOMJobResponseSet);
  return d->CopyFile;
}

//----------------------------------------------------------------------------
void ctkDICOMJobResponseSet::setOverwriteExistingDataset(const bool &overwriteExistingDataset)
{
  Q_D(ctkDICOMJobResponseSet);
  d->OverwriteExistingDataset = overwriteExistingDataset;
}

//----------------------------------------------------------------------------
bool ctkDICOMJobResponseSet::overwriteExistingDataset() const
{
  Q_D(const ctkDICOMJobResponseSet);
  return d->OverwriteExistingDataset;
}

//----------------------------------------------------------------------------
void ctkDICOMJobResponseSet::setTypeOfJob(const ctkDICOMJobResponseSet::JobType &typeOfJob)
{
  Q_D(ctkDICOMJobResponseSet);
  d->TypeOfJob = typeOfJob;
}

//------------------------------------------------------------------------------
ctkDICOMJobResponseSet::JobType ctkDICOMJobResponseSet::typeOfJob() const
{
  Q_D(const ctkDICOMJobResponseSet);
  return d->TypeOfJob;
}

//------------------------------------------------------------------------------
void ctkDICOMJobResponseSet::setJobUID(const QString &jobUID)
{
  Q_D(ctkDICOMJobResponseSet);
  d->JobUID = jobUID;
}

//------------------------------------------------------------------------------
QString ctkDICOMJobResponseSet::jobUID() const
{
  Q_D(const ctkDICOMJobResponseSet);
  return d->JobUID;
}

//------------------------------------------------------------------------------
void ctkDICOMJobResponseSet::setPatientID(const QString& patientID)
{
  Q_D(ctkDICOMJobResponseSet);
  d->PatientID = patientID;
}

//------------------------------------------------------------------------------
QString ctkDICOMJobResponseSet::patientID() const
{
  Q_D(const ctkDICOMJobResponseSet);
  return d->PatientID;
}

//------------------------------------------------------------------------------
void ctkDICOMJobResponseSet::setStudyInstanceUID(const QString& studyInstanceUID)
{
  Q_D(ctkDICOMJobResponseSet);
  d->StudyInstanceUID = studyInstanceUID;
}

//------------------------------------------------------------------------------
QString ctkDICOMJobResponseSet::studyInstanceUID() const
{
  Q_D(const ctkDICOMJobResponseSet);
  return d->StudyInstanceUID;
}

//----------------------------------------------------------------------------
void ctkDICOMJobResponseSet::setSeriesInstanceUID(const QString& seriesInstanceUID)
{
  Q_D(ctkDICOMJobResponseSet);
  d->SeriesInstanceUID = seriesInstanceUID;
}

//------------------------------------------------------------------------------
QString ctkDICOMJobResponseSet::seriesInstanceUID() const
{
  Q_D(const ctkDICOMJobResponseSet);
  return d->SeriesInstanceUID;
}

//----------------------------------------------------------------------------
void ctkDICOMJobResponseSet::setSOPInstanceUID(const QString& sopInstanceUID)
{
  Q_D(ctkDICOMJobResponseSet);
  d->SOPInstanceUID = sopInstanceUID;
}

//------------------------------------------------------------------------------
QString ctkDICOMJobResponseSet::sopInstanceUID() const
{
  Q_D(const ctkDICOMJobResponseSet);
  return d->SOPInstanceUID;
}

//------------------------------------------------------------------------------
void ctkDICOMJobResponseSet::setConnectionName(const QString &connectionName)
{
  Q_D(ctkDICOMJobResponseSet);
  d->ConnectionName = connectionName;
}

//------------------------------------------------------------------------------
QString ctkDICOMJobResponseSet::connectionName() const
{
  Q_D(const ctkDICOMJobResponseSet);
  return d->ConnectionName;
}

//------------------------------------------------------------------------------
void ctkDICOMJobResponseSet::setDataset(DcmItem *dcmItem, bool takeOwnership)
{
  Q_D(ctkDICOMJobResponseSet);
  if (!dcmItem)
    {
    return;
    }

  QSharedPointer<ctkDICOMItem> dataset =
    QSharedPointer<ctkDICOMItem>(new ctkDICOMItem);
  dataset->InitializeFromItem(dcmItem, takeOwnership);

  OFString SOPInstanceUID;
  dcmItem->findAndGetOFString(DCM_SOPInstanceUID, SOPInstanceUID);
  d->Datasets.insert(QString(SOPInstanceUID.c_str()), dataset);
}

//------------------------------------------------------------------------------
ctkDICOMItem *ctkDICOMJobResponseSet::dataset() const
{
  Q_D(const ctkDICOMJobResponseSet);
  if (d->Datasets.count() == 0)
    {
    return nullptr;
    }
  return d->Datasets.first().data();
}

//------------------------------------------------------------------------------
QSharedPointer<ctkDICOMItem> ctkDICOMJobResponseSet::datasetShared() const
{
  Q_D(const ctkDICOMJobResponseSet);
  if (d->Datasets.count() == 0)
    {
    return nullptr;
    }
  return d->Datasets.first();
}

//------------------------------------------------------------------------------
void ctkDICOMJobResponseSet::setDatasets(QMap<QString, DcmItem *> dcmItems, bool takeOwnership)
{
  Q_D(ctkDICOMJobResponseSet);
  for(QString key : dcmItems.keys())
    {
    DcmItem *dcmItem = dcmItems.value(key);
    if (!dcmItem)
      {
      continue;
      }

    QSharedPointer<ctkDICOMItem> dataset =
      QSharedPointer<ctkDICOMItem>(new ctkDICOMItem);
    dataset->InitializeFromItem(dcmItem, takeOwnership);

    d->Datasets.insert(key, dataset);
    }
}

//------------------------------------------------------------------------------
QMap<QString, ctkDICOMItem*> ctkDICOMJobResponseSet::datasets() const
{
  Q_D(const ctkDICOMJobResponseSet);
  QMap<QString, ctkDICOMItem*> datasets;

  for(QString key : d->Datasets.keys())
    {
    QSharedPointer<ctkDICOMItem> dcmItem = d->Datasets.value(key);
    if (!dcmItem)
      {
      continue;
      }

    datasets.insert(key, dcmItem.data());
    }

  return datasets;
}

//------------------------------------------------------------------------------
QMap<QString, QSharedPointer<ctkDICOMItem>> ctkDICOMJobResponseSet::datasetsShared() const
{
  Q_D(const ctkDICOMJobResponseSet);
  return d->Datasets;
}

//------------------------------------------------------------------------------
void ctkDICOMJobResponseSet::deepCopy(ctkDICOMJobResponseSet *node)
{
  Q_D(ctkDICOMJobResponseSet);

  if (!node)
    {
    return;
    }

  this->setFilePath(node->filePath());
  this->setCopyFile(node->copyFile());
  this->setOverwriteExistingDataset(node->overwriteExistingDataset());
  this->setTypeOfJob(node->typeOfJob());
  this->setJobUID(node->jobUID());
  this->setPatientID(node->patientID());
  this->setStudyInstanceUID(node->studyInstanceUID());
  this->setSeriesInstanceUID(node->seriesInstanceUID());
  this->setSOPInstanceUID(node->sopInstanceUID());
  this->setConnectionName(node->connectionName());

  d->Datasets.clear();

  QMap<QString, ctkDICOMItem *> nodeDatasets = node->datasets();
  for(QString key : nodeDatasets.keys())
    {
    ctkDICOMItem* nodeDataset = nodeDatasets.value(key);
    if (!nodeDataset)
      {
      continue;
      }

    DcmItem* nodedcmItem = nodeDataset->GetDcmItemPointer();
    DcmDataset* nodedcmDataset = dynamic_cast<DcmDataset*>(nodedcmItem);
    DcmItem* dcmItem = nullptr;
    if (nodedcmDataset)
      {
      dcmItem = new DcmDataset();
      dcmItem->copyFrom(*nodedcmDataset);
      }
    else
      {
      dcmItem = new DcmItem();
      dcmItem->copyFrom(*nodedcmItem);
      }

    QSharedPointer<ctkDICOMItem> dataset =
      QSharedPointer<ctkDICOMItem>(new ctkDICOMItem);
    dataset->InitializeFromItem(dcmItem, true);
    d->Datasets.insert(key, dataset);
    }
}

//------------------------------------------------------------------------------
QVariant ctkDICOMJobResponseSet::jobResponseSetToDetail()
{
  ctkJobDetail td;
  td.TypeOfJob = this->typeOfJob();
  td.JobUID = this->jobUID();
  td.PatientID = this->patientID();
  td.StudyInstanceUID = this->studyInstanceUID();
  td.SeriesInstanceUID = this->seriesInstanceUID();
  td.SOPInstanceUID = this->sopInstanceUID();
  td.ConnectionName = this->connectionName();
  td.NumberOfDataSets = this->datasets().count();

  QVariant data;
  data.setValue(td);

  return data;
}

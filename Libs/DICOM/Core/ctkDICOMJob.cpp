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
#include "ctkDICOMJob.h"
#include "ctkDICOMJobResponseSet.h"
#include "ctkLogger.h"

static ctkLogger logger ( "org.commontk.dicom.ctkDICOMJob" );

//------------------------------------------------------------------------------
// ctkDICOMJob methods

//------------------------------------------------------------------------------
ctkDICOMJob::ctkDICOMJob()
{
  this->DICOMLevel = DICOMLevels::Patients;
  this->PatientID = "";
  this->StudyInstanceUID = "";
  this->SeriesInstanceUID = "";
  this->SOPInstanceUID = "";
}

//------------------------------------------------------------------------------
ctkDICOMJob::~ctkDICOMJob()
{
}

//------------------------------------------------------------------------------
void ctkDICOMJob::setDICOMLevel(const DICOMLevels& dicomLevel)
{
  this->DICOMLevel = dicomLevel;
}

//------------------------------------------------------------------------------
ctkDICOMJob::DICOMLevels ctkDICOMJob::dicomLevel() const
{
  return this->DICOMLevel;
}

//----------------------------------------------------------------------------
void ctkDICOMJob::setPatientID(const QString &patientID)
{
  this->PatientID = patientID;
}

//----------------------------------------------------------------------------
QString ctkDICOMJob::patientID() const
{
  return this->PatientID;
}

//----------------------------------------------------------------------------
void ctkDICOMJob::setStudyInstanceUID(const QString& studyInstanceUID)
{
  this->StudyInstanceUID = studyInstanceUID;
}

//------------------------------------------------------------------------------
QString ctkDICOMJob::studyInstanceUID() const
{
  return this->StudyInstanceUID;
}

//----------------------------------------------------------------------------
void ctkDICOMJob::setSeriesInstanceUID(const QString& seriesInstanceUID)
{
  this->SeriesInstanceUID = seriesInstanceUID;
}

//------------------------------------------------------------------------------
QString ctkDICOMJob::seriesInstanceUID() const
{
  return this->SeriesInstanceUID;
}

//----------------------------------------------------------------------------
void ctkDICOMJob::setSOPInstanceUID(const QString& sopInstanceUID)
{
  this->SOPInstanceUID = sopInstanceUID;
}

//------------------------------------------------------------------------------
QString ctkDICOMJob::sopInstanceUID() const
{
  return this->SOPInstanceUID;
}

//----------------------------------------------------------------------------
static void skipDelete(QObject* obj)
{
  Q_UNUSED(obj);
  // this deleter does not delete the object from memory
  // useful if the pointer is not owned by the smart pointer
}

//------------------------------------------------------------------------------
QList<ctkDICOMJobResponseSet *> ctkDICOMJob::jobResponseSets() const
{
  QList<ctkDICOMJobResponseSet *> jobResponseSets;
  foreach(QSharedPointer<ctkDICOMJobResponseSet> jobResponseSet, this->JobResponseSets)
    {
    jobResponseSets.append(jobResponseSet.data());
    }

  return jobResponseSets;
}

//------------------------------------------------------------------------------
QList<QSharedPointer<ctkDICOMJobResponseSet>> ctkDICOMJob::jobResponseSetsShared() const
{
  return this->JobResponseSets;
}

//------------------------------------------------------------------------------
void ctkDICOMJob::setJobResponseSets(QList<ctkDICOMJobResponseSet*> jobResponseSets)
{
  this->JobResponseSets.clear();
  foreach(ctkDICOMJobResponseSet* jobResponseSet, jobResponseSets)
    {
    this->JobResponseSets.append(QSharedPointer<ctkDICOMJobResponseSet>(jobResponseSet, skipDelete));
    }
}

//------------------------------------------------------------------------------
void ctkDICOMJob::setJobResponseSets(QList<QSharedPointer<ctkDICOMJobResponseSet>> jobResponseSets)
{
  this->JobResponseSets = jobResponseSets;
}

//------------------------------------------------------------------------------
void ctkDICOMJob::copyJobResponseSets(QList<QSharedPointer<ctkDICOMJobResponseSet>> jobResponseSets)
{
  this->JobResponseSets.clear();
  foreach(QSharedPointer<ctkDICOMJobResponseSet> jobResponseSet, jobResponseSets)
    {
    QSharedPointer<ctkDICOMJobResponseSet> jobResponseSetCopy =
      QSharedPointer<ctkDICOMJobResponseSet>(new ctkDICOMJobResponseSet);
    jobResponseSetCopy->deepCopy(jobResponseSet.data());
    this->JobResponseSets.append(jobResponseSetCopy);
    }
}

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
#include "ctkDICOMRetrieveJob_p.h"
#include "ctkDICOMRetrieveWorker.h"
#include "ctkDICOMServer.h"
#include "ctkLogger.h"

static ctkLogger logger ( "org.commontk.dicom.ctkDICOMRetrieveJob" );

//------------------------------------------------------------------------------
// ctkDICOMRetrieveJobPrivate methods

//------------------------------------------------------------------------------
ctkDICOMRetrieveJobPrivate::ctkDICOMRetrieveJobPrivate(ctkDICOMRetrieveJob* object)
 : q_ptr(object)
{
  this->Server = nullptr;
}

//------------------------------------------------------------------------------
ctkDICOMRetrieveJobPrivate::~ctkDICOMRetrieveJobPrivate()
{
}

//------------------------------------------------------------------------------
// ctkDICOMRetrieveJob methods

//------------------------------------------------------------------------------
ctkDICOMRetrieveJob::ctkDICOMRetrieveJob()
  : d_ptr(new ctkDICOMRetrieveJobPrivate(this))
{
}

//------------------------------------------------------------------------------
ctkDICOMRetrieveJob::ctkDICOMRetrieveJob(ctkDICOMRetrieveJobPrivate* pimpl)
  : d_ptr(pimpl)
{
}

//------------------------------------------------------------------------------
ctkDICOMRetrieveJob::~ctkDICOMRetrieveJob()
{
}

//----------------------------------------------------------------------------
static void skipDelete(QObject* obj)
{
  Q_UNUSED(obj);
  // this deleter does not delete the object from memory
  // useful if the pointer is not owned by the smart pointer
}

//----------------------------------------------------------------------------
ctkDICOMServer* ctkDICOMRetrieveJob::server() const
{
  Q_D(const ctkDICOMRetrieveJob);
  return d->Server.data();
}

//----------------------------------------------------------------------------
QSharedPointer<ctkDICOMServer> ctkDICOMRetrieveJob::serverShared() const
{
  Q_D(const ctkDICOMRetrieveJob);
  return d->Server;
}

//----------------------------------------------------------------------------
void ctkDICOMRetrieveJob::setServer(ctkDICOMServer &server)
{
  Q_D(ctkDICOMRetrieveJob);
  d->Server = QSharedPointer<ctkDICOMServer>(&server, skipDelete);
}

//----------------------------------------------------------------------------
void ctkDICOMRetrieveJob::setServer(QSharedPointer<ctkDICOMServer> server)
{
  Q_D(ctkDICOMRetrieveJob);
  d->Server = server;
}

//----------------------------------------------------------------------------
QString ctkDICOMRetrieveJob::loggerReport(const QString &status) const
{
  switch (this->dicomLevel())
    {
    case ctkDICOMJob::DICOMLevels::Patients:
      return QString("ctkDICOMRetrieveJob: retrieve task at patients level %1.\n"
                     "JobUID: %2\n"
                     "Server: %3\n"
                     "PatientID: %4")
          .arg(status)
          .arg(this->jobUID())
          .arg(this->server()->connectionName())
          .arg(this->patientID());
    case ctkDICOMJob::DICOMLevels::Studies:
      return QString("ctkDICOMRetrieveJob: retrieve task at studies level %1.\n"
                     "JobUID: %2\n"
                     "Server: %3\n"
                     "PatientID: %4\n"
                     "StudyInstanceUID: %5")
          .arg(status)
          .arg(this->jobUID())
          .arg(this->server()->connectionName())
          .arg(this->patientID())
          .arg(this->studyInstanceUID());
    case ctkDICOMJob::DICOMLevels::Series:
      return QString("ctkDICOMRetrieveJob: retrieve task at series level %1.\n"
                     "JobUID: %2\n"
                     "Server: %3\n"
                     "PatientID: %4\n"
                     "StudyInstanceUID: %5\n"
                     "SeriesInstanceUID: %6")
          .arg(status)
          .arg(this->jobUID())
          .arg(this->server()->connectionName())
          .arg(this->patientID())
          .arg(this->studyInstanceUID())
          .arg(this->seriesInstanceUID());
    case ctkDICOMJob::DICOMLevels::Instances:
      return QString("ctkDICOMRetrieveJob: retrieve task at instances level %1.\n"
                     "JobUID: %2\n"
                     "Server: %3\n"
                     "PatientID: %4\n"
                     "StudyInstanceUID: %5\n"
                     "SeriesInstanceUID: %6\n"
                     "SOPInstanceUID: %7")
          .arg(status)
          .arg(this->jobUID())
          .arg(this->server()->connectionName())
          .arg(this->patientID())
          .arg(this->studyInstanceUID())
          .arg(this->seriesInstanceUID())
          .arg(this->sopInstanceUID());
    default:
      return QString("");
    }
}
//------------------------------------------------------------------------------
ctkDICOMJob* ctkDICOMRetrieveJob::generateCopy() const
{
  ctkDICOMRetrieveJob* newRetrieveJob = new ctkDICOMRetrieveJob;
  newRetrieveJob->setServer(this->serverShared());
  newRetrieveJob->setDICOMLevel(this->dicomLevel());
  newRetrieveJob->setPatientID(this->patientID());
  newRetrieveJob->setStudyInstanceUID(this->studyInstanceUID());
  newRetrieveJob->setSeriesInstanceUID(this->seriesInstanceUID());
  newRetrieveJob->setSOPInstanceUID(this->sopInstanceUID());
  newRetrieveJob->setMaximumNumberOfRetry(this->maximumNumberOfRetry());
  newRetrieveJob->setRetryDelay(this->retryDelay());
  newRetrieveJob->setRetryCounter(this->retryCounter());
  newRetrieveJob->setIsPersistent(this->isPersistent());
  newRetrieveJob->setMaximumConcurrentJobsPerType(this->maximumConcurrentJobsPerType());
  newRetrieveJob->setPriority(this->priority());

  return newRetrieveJob;
}

//------------------------------------------------------------------------------
ctkDICOMWorker *ctkDICOMRetrieveJob::createWorker()
{
  ctkDICOMRetrieveWorker* worker =
    new ctkDICOMRetrieveWorker;
  worker->setJob(*this);
  return worker;
}

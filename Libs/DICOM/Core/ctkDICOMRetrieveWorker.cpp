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
#include "ctkDICOMJobResponseSet.h"
#include "ctkDICOMRetrieveWorker_p.h"
#include "ctkDICOMRetrieveJob.h"
#include "ctkDICOMScheduler.h"
#include "ctkDICOMServer.h"
#include "ctkLogger.h"

static ctkLogger logger ("org.commontk.dicom.ctkDICOMRetrieveWorker");

//------------------------------------------------------------------------------
// ctkDICOMRetrieveWorkerPrivate methods

//------------------------------------------------------------------------------
ctkDICOMRetrieveWorkerPrivate::ctkDICOMRetrieveWorkerPrivate(ctkDICOMRetrieveWorker* object)
 : q_ptr(object)
{
  this->Retrieve = QSharedPointer<ctkDICOMRetrieve>(new ctkDICOMRetrieve);
}

//------------------------------------------------------------------------------
ctkDICOMRetrieveWorkerPrivate::~ctkDICOMRetrieveWorkerPrivate()
{
  Q_Q(ctkDICOMRetrieveWorker);

  QSharedPointer<ctkDICOMRetrieveJob> retrieveJob =
    qobject_cast<QSharedPointer<ctkDICOMRetrieveJob>>(q->Job);
  if (!retrieveJob)
  {
    return;
  }

  QObject::disconnect(this->Retrieve.data(), SIGNAL(progressJobDetail(QVariant)),
                      retrieveJob.data(), SIGNAL(progressJobDetail(QVariant)));
}

//------------------------------------------------------------------------------
void ctkDICOMRetrieveWorkerPrivate::setRetrieveParameters()
{
  Q_Q(ctkDICOMRetrieveWorker);

  QSharedPointer<ctkDICOMRetrieveJob> retrieveJob =
    qobject_cast<QSharedPointer<ctkDICOMRetrieveJob>>(q->Job);
  if (!retrieveJob)
    {
    return;
    }

  ctkDICOMServer* server = retrieveJob->server();
  if (!server)
    {
    return;
    }

  this->Retrieve->setConnectionName(server->connectionName());
  this->Retrieve->setCallingAETitle(server->callingAETitle());
  this->Retrieve->setCalledAETitle(server->calledAETitle());
  this->Retrieve->setHost(server->host());
  this->Retrieve->setPort(server->port());
  this->Retrieve->setConnectionTimeout(server->connectionTimeout());
  this->Retrieve->setMoveDestinationAETitle(server->moveDestinationAETitle());
  this->Retrieve->setKeepAssociationOpen(server->keepAssociationOpen());
  this->Retrieve->setJobUID(retrieveJob->jobUID());

  QObject::connect(this->Retrieve.data(), SIGNAL(progressJobDetail(QVariant)),
                   retrieveJob.data(), SIGNAL(progressJobDetail(QVariant)), Qt::DirectConnection);

}

//------------------------------------------------------------------------------
// ctkDICOMRetrieveWorker methods

//------------------------------------------------------------------------------
ctkDICOMRetrieveWorker::ctkDICOMRetrieveWorker()
  : d_ptr(new ctkDICOMRetrieveWorkerPrivate(this))
{
}

//------------------------------------------------------------------------------
ctkDICOMRetrieveWorker::ctkDICOMRetrieveWorker(ctkDICOMRetrieveWorkerPrivate* pimpl)
  : d_ptr(pimpl)
{
  Q_D(ctkDICOMRetrieveWorker);
}

//------------------------------------------------------------------------------
ctkDICOMRetrieveWorker::~ctkDICOMRetrieveWorker()
{
}

//----------------------------------------------------------------------------
void ctkDICOMRetrieveWorker::cancel()
{
  Q_D(const ctkDICOMRetrieveWorker);
  d->Retrieve->cancel();
}

//----------------------------------------------------------------------------
void ctkDICOMRetrieveWorker::run()
{
  Q_D(const ctkDICOMRetrieveWorker);
  QSharedPointer<ctkDICOMRetrieveJob> retrieveJob =
    qobject_cast<QSharedPointer<ctkDICOMRetrieveJob>>(this->Job);
  if (!retrieveJob)
    {
    return;
    }

  QSharedPointer<ctkDICOMScheduler> scheduler =
      qobject_cast<QSharedPointer<ctkDICOMScheduler>>(this->Scheduler);
  if (!scheduler)
    {
    emit retrieveJob->canceled();
    this->onJobCanceled();
    retrieveJob->setStatus(ctkAbstractJob::JobStatus::Finished);
    return;
    }

  ctkDICOMServer* server = retrieveJob->server();
  if (!server)
    {
    emit retrieveJob->canceled();
    this->onJobCanceled();
    retrieveJob->setStatus(ctkAbstractJob::JobStatus::Finished);
    return;
    }

  if (retrieveJob->status() == ctkAbstractJob::JobStatus::Stopped)
    {
    emit retrieveJob->canceled();
    this->onJobCanceled();
    retrieveJob->setStatus(ctkAbstractJob::JobStatus::Finished);
    return;
    }

  retrieveJob->setStatus(ctkAbstractJob::JobStatus::Running);
  emit retrieveJob->started();

  logger.debug("ctkDICOMRetrieveWorker : running job on thread id " +
               QString::number(reinterpret_cast<quint64>(QThread::currentThreadId()), 16));
  switch (server->retrieveProtocol())
    {
    case ctkDICOMServer::CGET:
      switch(retrieveJob->dicomLevel())
        {
        case ctkDICOMJob::DICOMLevels::Patients:
          logger.info("ctkDICOMRetrieveTask : get operation for a full patient is not implemented.");
          emit retrieveJob->canceled();
          this->onJobCanceled();
          retrieveJob->setStatus(ctkAbstractJob::JobStatus::Finished);
          return;
        case ctkDICOMJob::DICOMLevels::Studies:
          if (!d->Retrieve->getStudy(retrieveJob->studyInstanceUID(),
                                     retrieveJob->patientID()))
            {
            emit retrieveJob->canceled();
            this->onJobCanceled();
            retrieveJob->setStatus(ctkAbstractJob::JobStatus::Finished);
            return;
            }
          break;
        case ctkDICOMJob::DICOMLevels::Series:
          if (!d->Retrieve->getSeries(retrieveJob->studyInstanceUID(),
                                      retrieveJob->seriesInstanceUID(),
                                      retrieveJob->patientID()))
            {
            emit retrieveJob->canceled();
            this->onJobCanceled();
            retrieveJob->setStatus(ctkAbstractJob::JobStatus::Finished);
            return;
            }
          break;
        case ctkDICOMJob::DICOMLevels::Instances:
          if (!d->Retrieve->getSOPInstance(retrieveJob->studyInstanceUID(),
                                           retrieveJob->seriesInstanceUID(),
                                           retrieveJob->sopInstanceUID(),
                                           retrieveJob->patientID()))
            {
            emit retrieveJob->canceled();
            this->onJobCanceled();
            retrieveJob->setStatus(ctkAbstractJob::JobStatus::Finished);
            return;
            }
          break;
        }
      break;
    case ctkDICOMServer::CMOVE:
      switch(retrieveJob->dicomLevel())
        {
        case ctkDICOMJob::DICOMLevels::Patients:
          logger.info("ctkDICOMRetrieveTask : move operation for a full patient is not implemented.");
          retrieveJob->setStatus(ctkAbstractJob::JobStatus::Finished);
          emit retrieveJob->failed();
          return;
        case ctkDICOMJob::DICOMLevels::Studies:
          if (!d->Retrieve->moveStudy(retrieveJob->studyInstanceUID(),
                                      retrieveJob->patientID()))
            {
            emit retrieveJob->canceled();
            this->onJobCanceled();
            retrieveJob->setStatus(ctkAbstractJob::JobStatus::Finished);
            return;
            }
          break;
        case ctkDICOMJob::DICOMLevels::Series:
          if (!d->Retrieve->moveSeries(retrieveJob->studyInstanceUID(),
                                       retrieveJob->seriesInstanceUID(),
                                       retrieveJob->patientID()))
            {
            emit retrieveJob->canceled();
            this->onJobCanceled();
            retrieveJob->setStatus(ctkAbstractJob::JobStatus::Finished);
            return;
            }
          break;
        case ctkDICOMJob::DICOMLevels::Instances:
          if (!d->Retrieve->moveSOPInstance(retrieveJob->studyInstanceUID(),
                                            retrieveJob->seriesInstanceUID(),
                                            retrieveJob->sopInstanceUID(),
                                            retrieveJob->patientID()))
            {
            emit retrieveJob->canceled();
            this->onJobCanceled();
            retrieveJob->setStatus(ctkAbstractJob::JobStatus::Finished);
            return;
            }
          break;
        }
      break;
      //case ctkDICOMServer::WADO: // To Do
    }

  if (retrieveJob->status() == ctkAbstractJob::JobStatus::Stopped)
    {
    emit retrieveJob->canceled();
    this->onJobCanceled();
    retrieveJob->setStatus(ctkAbstractJob::JobStatus::Finished);
    return;
    }

  ctkDICOMServer* proxyServer = server->proxyServer();
  if (proxyServer && proxyServer->queryRetrieveEnabled())
    {
    ctkDICOMRetrieveJob* newJob = qobject_cast<ctkDICOMRetrieveJob*>(retrieveJob->generateCopy());
    newJob->setRetryCounter(0);
    newJob->setServer(*proxyServer);
    scheduler->addJob(newJob);
    }
  else if (d->Retrieve->jobResponseSetsShared().count() > 0)
    {
    // To Do: this insert should happen in batch of 10 frames (configurable),
    // instead of at the end of operation (all frames requested)).
    // This would avoid memory usage spikes when requesting a series or study with a lot of frames.
    // NOTE: the memory release should happen as soon as we insert the response.
    scheduler->insertJobResponseSets(d->Retrieve->jobResponseSetsShared());
    }

  retrieveJob->setStatus(ctkAbstractJob::JobStatus::Finished);
  emit retrieveJob->finished();
}

//----------------------------------------------------------------------------
static void skipDelete(QObject* obj)
{
  Q_UNUSED(obj);
  // this deleter does not delete the object from memory
  // useful if the pointer is not owned by the smart pointer
}

//----------------------------------------------------------------------------
void ctkDICOMRetrieveWorker::setJob(ctkAbstractJob &job)
{
  this->setJob(QSharedPointer<ctkAbstractJob>(&job, skipDelete));
}

//----------------------------------------------------------------------------
void ctkDICOMRetrieveWorker::setJob(QSharedPointer<ctkAbstractJob> job)
{
  Q_D(ctkDICOMRetrieveWorker);

  QSharedPointer<ctkDICOMRetrieveJob> retrieveJob =
    qobject_cast<QSharedPointer<ctkDICOMRetrieveJob>>(job);
  if (!retrieveJob)
    {
    return;
    }

  Superclass::setJob(job);
  d->setRetrieveParameters();
}

//----------------------------------------------------------------------------
ctkDICOMRetrieve *ctkDICOMRetrieveWorker::retriever() const
{
  Q_D(const ctkDICOMRetrieveWorker);
  return d->Retrieve.data();
}

//------------------------------------------------------------------------------
QSharedPointer<ctkDICOMRetrieve> ctkDICOMRetrieveWorker::retrieverShared() const
{
  Q_D(const ctkDICOMRetrieveWorker);
  return d->Retrieve;
}

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

// Qt includes
#include <QThread>

// ctkDICOMCore includes
#include "ctkDICOMJobResponseSet.h"
#include "ctkDICOMScheduler.h"
#include "ctkDICOMStorageListenerWorker_p.h"
#include "ctkDICOMStorageListenerJob.h"
#include "ctkLogger.h"

static ctkLogger logger ("org.commontk.dicom.ctkDICOMStorageListenerWorker");

//------------------------------------------------------------------------------
// ctkDICOMStorageListenerWorkerPrivate methods

//------------------------------------------------------------------------------
ctkDICOMStorageListenerWorkerPrivate::ctkDICOMStorageListenerWorkerPrivate(ctkDICOMStorageListenerWorker* object)
 : q_ptr(object)
{
  this->StorageListener = QSharedPointer<ctkDICOMStorageListener>(new ctkDICOMStorageListener);
}

//------------------------------------------------------------------------------
ctkDICOMStorageListenerWorkerPrivate::~ctkDICOMStorageListenerWorkerPrivate()
{
}

//------------------------------------------------------------------------------
void ctkDICOMStorageListenerWorkerPrivate::setStorageListenerParameters()
{
  Q_Q(ctkDICOMStorageListenerWorker);

  QSharedPointer<ctkDICOMStorageListenerJob> storageListenerJob =
    qobject_cast<QSharedPointer<ctkDICOMStorageListenerJob>>(q->Job);
  if (!storageListenerJob)
    {
    return;
    }

  this->StorageListener->setAETitle(storageListenerJob->AETitle());
  this->StorageListener->setPort(storageListenerJob->port());
  this->StorageListener->setConnectionTimeout(storageListenerJob->connectionTimeout());
  this->StorageListener->setJobUID(storageListenerJob->jobUID());

  QObject::connect(this->StorageListener.data(), SIGNAL(progressJobDetail(QVariant)),
                   storageListenerJob.data(), SIGNAL(progressJobDetail(QVariant)),
                   Qt::DirectConnection);
}

//------------------------------------------------------------------------------
void ctkDICOMStorageListenerWorkerPrivate::init()
{
  Q_Q(ctkDICOMStorageListenerWorker);
  // To Do: this insert should happen in batch of 10 frames (configurable),
  // instead of every 1 sec.
  // This would avoid memory usage spikes when requesting a series or study with a lot of frames.
  // i.e. the slot should be connected to progressJobDetail from this->StorageListener.
  // The slot should have a counter. When the counter > batchLimit -> insert
  // NOTE: the memory release should happen as soon as we insert the response.
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), q, SLOT(onInsertJobDetail()));
  timer->start(1000);
}

//------------------------------------------------------------------------------
// ctkDICOMStorageListenerWorker methods

//------------------------------------------------------------------------------
ctkDICOMStorageListenerWorker::ctkDICOMStorageListenerWorker()
  : d_ptr(new ctkDICOMStorageListenerWorkerPrivate(this))
{
  Q_D(ctkDICOMStorageListenerWorker);
  d->init();
}

//------------------------------------------------------------------------------
ctkDICOMStorageListenerWorker::ctkDICOMStorageListenerWorker(ctkDICOMStorageListenerWorkerPrivate* pimpl)
  : d_ptr(pimpl)
{
}

//------------------------------------------------------------------------------
ctkDICOMStorageListenerWorker::~ctkDICOMStorageListenerWorker()
{
}

//----------------------------------------------------------------------------
void ctkDICOMStorageListenerWorker::cancel()
{
  Q_D(const ctkDICOMStorageListenerWorker);
  d->StorageListener->cancel();
}

//----------------------------------------------------------------------------
void ctkDICOMStorageListenerWorker::run()
{
  Q_D(const ctkDICOMStorageListenerWorker);
  QSharedPointer<ctkDICOMStorageListenerJob> storageListenerJob =
      qobject_cast<QSharedPointer<ctkDICOMStorageListenerJob>>(this->Job);
  if (!storageListenerJob)
    {
    return;
    }

  QSharedPointer<ctkDICOMScheduler> scheduler =
    qobject_cast<QSharedPointer<ctkDICOMScheduler>>(this->Scheduler);
  if (!scheduler)
    {
    emit storageListenerJob->canceled();
    this->onJobCanceled();
    storageListenerJob->setStatus(ctkAbstractJob::JobStatus::Finished);
    return;
    }

  if (storageListenerJob->status() == ctkAbstractJob::JobStatus::Stopped)
    {
    emit storageListenerJob->canceled();
    this->onJobCanceled();
    storageListenerJob->setStatus(ctkAbstractJob::JobStatus::Finished);
    return;
    }

  storageListenerJob->setStatus(ctkAbstractJob::JobStatus::Running);
  emit storageListenerJob->started();

  logger.debug("ctkDICOMStorageListenerWorker : running job on thread id " +
               QString::number(reinterpret_cast<quint64>(QThread::currentThreadId()), 16));

  if (!d->StorageListener->listen())
    {
    emit storageListenerJob->canceled();
    this->onJobCanceled();
    storageListenerJob->setStatus(ctkAbstractJob::JobStatus::Finished);
    return;
    }

  storageListenerJob->setStatus(ctkAbstractJob::JobStatus::Finished);
  emit storageListenerJob->finished();
}

//----------------------------------------------------------------------------
static void skipDelete(QObject* obj)
{
  Q_UNUSED(obj);
  // this deleter does not delete the object from memory
  // useful if the pointer is not owned by the smart pointer
}

//----------------------------------------------------------------------------
void ctkDICOMStorageListenerWorker::setJob(ctkAbstractJob &job)
{
  this->setJob(QSharedPointer<ctkAbstractJob>(&job, skipDelete));
}

//----------------------------------------------------------------------------
void ctkDICOMStorageListenerWorker::setJob(QSharedPointer<ctkAbstractJob> job)
{
  Q_D(ctkDICOMStorageListenerWorker);

  QSharedPointer<ctkDICOMStorageListenerJob> storageListenerJob =
    qobject_cast<QSharedPointer<ctkDICOMStorageListenerJob>>(job);
  if (!storageListenerJob)
    {
    return;
    }

  Superclass::setJob(job);
  d->setStorageListenerParameters();
}

//----------------------------------------------------------------------------
ctkDICOMStorageListener *ctkDICOMStorageListenerWorker::storageListener() const
{
  Q_D(const ctkDICOMStorageListenerWorker);
  return d->StorageListener.data();
}

//------------------------------------------------------------------------------
QSharedPointer<ctkDICOMStorageListener> ctkDICOMStorageListenerWorker::storageListenerShared() const
{
  Q_D(const ctkDICOMStorageListenerWorker);
  return d->StorageListener;
}

//----------------------------------------------------------------------------
void ctkDICOMStorageListenerWorker::onInsertJobDetail()
{
  Q_D(ctkDICOMStorageListenerWorker);

  QSharedPointer<ctkDICOMScheduler> scheduler =
      qobject_cast<QSharedPointer<ctkDICOMScheduler>>(this->Scheduler);
  if (!scheduler)
    {
    return;
    }

  QList<QSharedPointer<ctkDICOMJobResponseSet>> jobResponseSets =
    d->StorageListener->jobResponseSetsShared();
  scheduler->insertJobResponseSets(jobResponseSets);
  foreach (QSharedPointer<ctkDICOMJobResponseSet> jobResponseSet, jobResponseSets)
    {
    d->StorageListener->removeJobResponseSet(jobResponseSet);
    }
}

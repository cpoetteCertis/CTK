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

#include "ctkAbstractJob.h"

// Qt includes
#include <QUuid>

// --------------------------------------------------------------------------
ctkAbstractJob::ctkAbstractJob()
{
  this->Status = JobStatus::Initialized;
  this->Persistent = false;
  this->JobUID = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
  this->RetryCounter = 0;
  this->RetryDelay = 100;
  this->MaximumNumberOfRetry = 3;
  this->MaximumConcurrentJobsPerType = 20;
  this->Priority = QThread::Priority::LowPriority;
}

//----------------------------------------------------------------------------
ctkAbstractJob::~ctkAbstractJob()
{
}

//----------------------------------------------------------------------------
void ctkAbstractJob::setJobUID(const QString &jobUID)
{
  this->JobUID = jobUID;
}

//----------------------------------------------------------------------------
QString ctkAbstractJob::className() const
{
  if (!this->metaObject())
    {
    return "";
    }
  return this->metaObject()->className();
}

//----------------------------------------------------------------------------
QString ctkAbstractJob::jobUID() const
{
  return this->JobUID;
}

//----------------------------------------------------------------------------
ctkAbstractJob::JobStatus ctkAbstractJob::status() const
{
  return this->Status;
}

//----------------------------------------------------------------------------
void ctkAbstractJob::setStatus(const JobStatus &status)
{
  this->Status = status;
}

//----------------------------------------------------------------------------
bool ctkAbstractJob::isPersistent() const
{
  return this->Persistent;
}

//----------------------------------------------------------------------------
void ctkAbstractJob::setIsPersistent(const bool &persistent)
{
  this->Persistent = persistent;
}

//----------------------------------------------------------------------------
int ctkAbstractJob::retryCounter() const
{
  return this->RetryCounter;
}

//----------------------------------------------------------------------------
void ctkAbstractJob::setRetryCounter(const int& retryCounter)
{
  this->RetryCounter = retryCounter;
}

//----------------------------------------------------------------------------
int ctkAbstractJob::maximumConcurrentJobsPerType() const
{
  return this->MaximumConcurrentJobsPerType;
}

//----------------------------------------------------------------------------
void ctkAbstractJob::setMaximumConcurrentJobsPerType(const int &maximumConcurrentJobsPerType)
{
  this->MaximumConcurrentJobsPerType = maximumConcurrentJobsPerType;
}

//----------------------------------------------------------------------------
int ctkAbstractJob::maximumNumberOfRetry() const
{
  return this->MaximumNumberOfRetry;
}

//----------------------------------------------------------------------------
void ctkAbstractJob::setMaximumNumberOfRetry(const int &maximumNumberOfRetry)
{
  this->MaximumNumberOfRetry = maximumNumberOfRetry;
}

//----------------------------------------------------------------------------
int ctkAbstractJob::retryDelay() const
{
  return this->RetryDelay;
}

//----------------------------------------------------------------------------
void ctkAbstractJob::setRetryDelay(const int &retryDelay)
{
  this->RetryDelay = retryDelay;
}

//----------------------------------------------------------------------------
QThread::Priority ctkAbstractJob::priority() const
{
  return this->Priority;
}

//----------------------------------------------------------------------------
void ctkAbstractJob::setPriority(const QThread::Priority &priority)
{
  this->Priority = priority;
}


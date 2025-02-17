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

#ifndef __ctkDICOMRetrieveJob_h
#define __ctkDICOMRetrieveJob_h

// Qt includes 
#include <QObject>
#include <QSharedPointer>

// ctkDICOMCore includes
#include "ctkDICOMCoreExport.h"
#include "ctkDICOMJob.h"

class ctkDICOMRetrieveJobPrivate;
class ctkDICOMServer;

/// \ingroup DICOM_Core
class CTK_DICOM_CORE_EXPORT ctkDICOMRetrieveJob : public ctkDICOMJob
{
  Q_OBJECT

public:
  typedef ctkDICOMJob Superclass;
  explicit ctkDICOMRetrieveJob();
  virtual ~ctkDICOMRetrieveJob();

  /// Server
  Q_INVOKABLE ctkDICOMServer* server() const;
  QSharedPointer<ctkDICOMServer> serverShared() const;
  Q_INVOKABLE void setServer(ctkDICOMServer& server);
  void setServer(QSharedPointer<ctkDICOMServer> server);

  /// Logger report string formatting for specific task
  Q_INVOKABLE QString loggerReport(const QString& status) const;

  /// Create a copy of the object
  Q_INVOKABLE ctkDICOMJob* generateCopy() const;

  /// Generate worker for job
  Q_INVOKABLE ctkDICOMWorker* createWorker();

protected:
  QScopedPointer<ctkDICOMRetrieveJobPrivate> d_ptr;

  /// Constructor allowing derived class to specify a specialized pimpl.
  ///
  /// \note You are responsible to call init() in the constructor of
  /// derived class. Doing so ensures the derived class is fully
  /// instantiated in case virtual method are called within init() itself.
  ctkDICOMRetrieveJob(ctkDICOMRetrieveJobPrivate* pimpl);

private:
  Q_DECLARE_PRIVATE(ctkDICOMRetrieveJob);
  Q_DISABLE_COPY(ctkDICOMRetrieveJob);
};

#endif

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

=========================================================================*/

#ifndef __ctkDICOMStorageListener_h
#define __ctkDICOMStorageListener_h

// Qt includes
#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>

// CTK includes
#include <ctkPimpl.h>

// ctkDICOMCore includes
#include "ctkDICOMCoreExport.h"
#include "ctkErrorLogLevel.h"

class ctkDICOMStorageListenerPrivate;
class ctkDICOMJobResponseSet;

/// \ingroup DICOM_Core
class CTK_DICOM_CORE_EXPORT ctkDICOMStorageListener : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString AETitle READ AETitle WRITE setAETitle);
  Q_PROPERTY(int port READ port WRITE setPort);
  Q_PROPERTY(int connectionTimeout READ connectionTimeout WRITE setConnectionTimeout);

public:
  explicit ctkDICOMStorageListener(QObject* parent = 0);
  virtual ~ctkDICOMStorageListener();

  /// Storage AE title
  /// "CTKSTORE" by default
  void setAETitle(const QString& AETitle);
  QString AETitle() const;
  /// Storage port
  /// 11112 by default
  void setPort(int port);
  int port() const;
  /// connection timeout
  /// 1 sec by default
  void setConnectionTimeout(const int timeout);
  int connectionTimeout();

  /// Access the list of datasets from the last operation.
  Q_INVOKABLE QList<ctkDICOMJobResponseSet*> jobResponseSets() const;
  QList<QSharedPointer<ctkDICOMJobResponseSet>> jobResponseSetsShared() const;
  Q_INVOKABLE void addJobResponseSet(ctkDICOMJobResponseSet& jobResponseSet);
  void addJobResponseSet(QSharedPointer<ctkDICOMJobResponseSet> jobResponseSet);
  void removeJobResponseSet(QSharedPointer<ctkDICOMJobResponseSet> jobResponseSet);
  Q_INVOKABLE void setJobUID(const QString& jobUID);
  Q_INVOKABLE QString jobUID() const;

  /// Log level for dcmtk. Default: Error.
  Q_INVOKABLE void setDCMTKLogLevel(const ctkErrorLogLevel::LogLevel& level);
  Q_INVOKABLE ctkErrorLogLevel::LogLevel DCMTKLogLevel() const;

  /// Start listen connection.
  bool listen();

  /// operation is canceled?
  Q_INVOKABLE bool wasCanceled();

Q_SIGNALS:
    /// Signal is emitted inside the listener() function. It ranges from 0 to 100.
    /// In case of an error, you are assured that the progress value 100 is fired
    void progress(int progress);
    /// Signal is emitted inside the listener() function. It sends the different step
    /// the function is at.
    void progress(const QString& message);
    /// Signal is emitted inside the listener() function. It sends
    /// detailed feedback for debugging
    void debug(const QString& message);
    /// Signal is emitted inside the listener() function. It send any error messages
    void error(const QString& message);
    /// Signal is emitted inside the listener() function when finished with value
    /// true for success or false for error
    void done(const bool& error);
    /// Signal is emitted inside the listener() function when a frame has been fetched
    void progressJobDetail(QVariant);

public Q_SLOTS:
  void cancel();

protected:
  bool initializeSCU();

  QScopedPointer<ctkDICOMStorageListenerPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(ctkDICOMStorageListener);
  Q_DISABLE_COPY(ctkDICOMStorageListener);

  friend class ctkDICOMStorageListenerSCUPrivate;
};

#endif

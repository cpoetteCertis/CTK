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

#ifndef __ctkDICOMEcho_h
#define __ctkDICOMEcho_h

// Qt includes
#include <QObject>
#include <QMap>
#include <QString>

// CTK includes
#include <ctkPimpl.h>

// ctkDICOMCore includes
#include "ctkDICOMCoreExport.h"
#include "ctkErrorLogLevel.h"

class ctkDICOMEchoPrivate;

/// \ingroup DICOM_Core
class CTK_DICOM_CORE_EXPORT ctkDICOMEcho : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString connectionName READ connectionName WRITE setConnectionName);
  Q_PROPERTY(QString callingAETitle READ callingAETitle WRITE setCallingAETitle);
  Q_PROPERTY(QString calledAETitle READ calledAETitle WRITE setCalledAETitle);
  Q_PROPERTY(QString host READ host WRITE setHost);
  Q_PROPERTY(int port READ port WRITE setPort);
  Q_PROPERTY(int connectionTimeout READ connectionTimeout WRITE setConnectionTimeout);

public:
  explicit ctkDICOMEcho(QObject* parent = 0);
  virtual ~ctkDICOMEcho();

  /// Set methods for connectivity.
  /// Empty by default
  void setConnectionName(const QString& connectionName);
  QString connectionName() const;
  /// Empty by default
  void setCallingAETitle(const QString& callingAETitle);
  QString callingAETitle()const;
  /// Empty by default
  void setCalledAETitle(const QString& calledAETitle);
  QString calledAETitle() const;
  /// Empty by default
  void setHost(const QString& host);
  QString host() const;
  /// Specify a port for the packet headers.
  /// \a port ranges from 0 to 65535.
  /// 80 by default.
  void setPort(int port);
  int port() const;
  /// connection timeout, default 3 sec.
  void setConnectionTimeout(const int timeout);
  int connectionTimeout() const;

  /// Log level for dcmtk. Default: Error.
  Q_INVOKABLE void setDCMTKLogLevel(const ctkErrorLogLevel::LogLevel& level);
  Q_INVOKABLE ctkErrorLogLevel::LogLevel DCMTKLogLevel() const;

  /// Echo connection.
  bool echo();

protected:
  QScopedPointer<ctkDICOMEchoPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(ctkDICOMEcho);
  Q_DISABLE_COPY(ctkDICOMEcho);
};

#endif

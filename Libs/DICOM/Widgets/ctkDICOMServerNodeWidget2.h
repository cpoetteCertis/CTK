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

#ifndef __ctkDICOMServerNodeWidget2_h
#define __ctkDICOMServerNodeWidget2_h

// Qt includes
#include <QWidget>
#include <QString>
#include <QList>
#include <QMap>

#include "ctkDICOMWidgetsExport.h"

class QTableWidgetItem;
class ctkAbstractTask;
class ctkDICOMServer;
class ctkDICOMServerNodeWidget2Private;
class ctkDICOMScheduler;

/// \ingroup DICOM_Widgets
class CTK_DICOM_WIDGETS_EXPORT ctkDICOMServerNodeWidget2 : public QWidget
{
  Q_OBJECT;
  Q_PROPERTY(QString storageAETitle READ storageAETitle WRITE setStorageAETitle);
  Q_PROPERTY(int storagePort READ storagePort WRITE setStoragePort);

public:
  typedef QWidget Superclass;
  explicit ctkDICOMServerNodeWidget2(QWidget* parent=0);
  virtual ~ctkDICOMServerNodeWidget2();

  /// Storage listener is enabled
  /// false by default
  void setStorageListenerEnabled(const bool enabled);
  bool storageListenerEnabled() const;

  /// Storage AE title
  /// "CTKSTORE" by default
  void setStorageAETitle(const QString& storageAETitle);
  QString storageAETitle() const;

  /// Storage port
  /// 11112 by default
  void setStoragePort(const int storagePort);
  int storagePort() const;

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

  /// Servers
  Q_INVOKABLE int getNumberOfServers();
  Q_INVOKABLE ctkDICOMServer* getNthServer(int id);
  Q_INVOKABLE ctkDICOMServer* getServer(const char* connectionName);
  Q_INVOKABLE void addServer(ctkDICOMServer* server);
  Q_INVOKABLE void removeServer(const char* connectionName);
  Q_INVOKABLE void removeNthServer(int id);
  Q_INVOKABLE void removeAllServers();
  Q_INVOKABLE QString getServerNameFromIndex(int id);
  Q_INVOKABLE int getServerIndexFromName(const char* connectionName);
  Q_INVOKABLE void stopAllJobs();

public Q_SLOTS:
  /// Add an empty server node and make it current
  /// Return the row index added into the table
  int onAddServerNode();
  /// Remove the current row (different from the checked rows)
  void onRemoveCurrentServerNode();
  /// Test the current row (different from the checked rows)
  void onTestCurrentServerNode();

  void readSettings();
  void saveSettings();
  void updateGUIState();
  void onSettingsModified();

protected:
  QScopedPointer<ctkDICOMServerNodeWidget2Private> d_ptr;
  enum ServerColumns{
    NameColumn = 0,
    QueryRetrieveColumn,
    StorageColumn,
    CallingAETitleColumn,
    CalledAETitleColumn,
    AddressColumn,
    PortColumn,
    TimeoutColumn,
    ProtocolColumn,
    ProxyColumn
  };
private:
  Q_DECLARE_PRIVATE(ctkDICOMServerNodeWidget2);
  Q_DISABLE_COPY(ctkDICOMServerNodeWidget2);
};

#endif

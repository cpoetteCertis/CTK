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

// Qt includes
#include <QDebug>
#include <QSettings>
#include <QString>
#include <QStringList>

// ctkDICOMCore includes
#include "ctkDICOMStorageListener.h"
#include "ctkDICOMJobResponseSet.h"
#include "ctkLogger.h"

// DCMTK includes
#include "dcmtk/dcmnet/dstorscp.h"   /* for DcmStorageSCP */

static ctkLogger logger ( "org.commontk.dicom.ctkDICOMStorageListener" );

//------------------------------------------------------------------------------
// A customized implementation so that Qt signals can be emitted
// when query results are obtained
class ctkDICOMStorageListenerSCUPrivate : public DcmStorageSCP
{
public:
    ctkDICOMStorageListener *listener;
    ctkDICOMStorageListenerSCUPrivate()
    {
      this->listener = 0;
    };
    ~ctkDICOMStorageListenerSCUPrivate() {};

    virtual OFCondition acceptAssociations()
    {
    return DcmSCP::acceptAssociations();
    }

    virtual OFBool stopAfterCurrentAssociation()
    {
      if (!this->listener || this->listener->wasCanceled())
        {
        return OFTrue;
        }

      return OFFalse;
    }

    virtual OFBool stopAfterConnectionTimeout()
    {
      if (!this->listener || this->listener->wasCanceled())
        {
        return OFTrue;
        }

      return OFFalse;
    }

    virtual OFCondition handleIncomingCommand(T_DIMSE_Message *incomingMsg,
                                              const DcmPresentationContextInfo &presInfo)
    {
      OFCondition status = EC_IllegalParameter;
      if (incomingMsg != NULL && !this->listener->wasCanceled())
        {
        // check whether we've received a supported command
        if (incomingMsg->CommandField == DIMSE_C_ECHO_RQ)
          {
          // handle incoming C-ECHO request
          status = handleECHORequest(incomingMsg->msg.CEchoRQ, presInfo.presentationContextID);
          }
        else if (incomingMsg->CommandField == DIMSE_C_STORE_RQ)
          {
          // handle incoming C-STORE request
          T_DIMSE_C_StoreRQ &storeReq = incomingMsg->msg.CStoreRQ;
          Uint16 rspStatusCode = STATUS_STORE_Error_CannotUnderstand;
          DcmDataset *reqDataset = new DcmDataset;
          // receive dataset in memory
          status = receiveSTORERequest(storeReq, presInfo.presentationContextID, reqDataset);
          if (status.good())
            {
            rspStatusCode = STATUS_Success;
            }

          OFString instanceUID;
          reqDataset->findAndGetOFString(DCM_SOPInstanceUID, instanceUID);
          OFString seriesUID;
          reqDataset->findAndGetOFString(DCM_SeriesInstanceUID, seriesUID);
          OFString studyUID;
          reqDataset->findAndGetOFString(DCM_StudyInstanceUID, studyUID);
          emit this->listener->progress(
            ctkDICOMStorageListener::tr("Got STORE request for %1").arg(instanceUID.c_str())
            );
          emit this->listener->progress(0);
          if (!this->listener->jobUID().isEmpty() && !this->listener->wasCanceled())
            {
            QSharedPointer<ctkDICOMJobResponseSet> jobResponseSet =
              QSharedPointer<ctkDICOMJobResponseSet>(new ctkDICOMJobResponseSet);
            jobResponseSet->setTypeOfJob(ctkDICOMJobResponseSet::JobType::StoreSOPInstance);
            jobResponseSet->setStudyInstanceUID(studyUID.c_str());
            jobResponseSet->setSeriesInstanceUID(seriesUID.c_str());
            jobResponseSet->setSOPInstanceUID(instanceUID.c_str());
            jobResponseSet->setConnectionName(this->listener->AETitle());
            jobResponseSet->setDataset(reqDataset);
            jobResponseSet->setJobUID(this->listener->jobUID());
            jobResponseSet->setCopyFile(true);

            this->listener->addJobResponseSet(jobResponseSet);

            emit this->listener->progressJobDetail(jobResponseSet->jobResponseSetToDetail());
            }
          // send C-STORE response (with DIMSE status code)
          if (status.good())
            {
            status = sendSTOREResponse(presInfo.presentationContextID, storeReq, rspStatusCode);
            }
          else if (status == DIMSE_OUTOFRESOURCES)
            {
            // do not overwrite the previous error status
            sendSTOREResponse(presInfo.presentationContextID, storeReq, STATUS_STORE_Refused_OutOfResources);
            }
          }
        else
          {
          // unsupported command
          OFString tempStr;
          DCMNET_ERROR("cannot handle this kind of DIMSE command (0x"
                       << STD_NAMESPACE hex << STD_NAMESPACE setfill('0') << STD_NAMESPACE setw(4)
                       << OFstatic_cast(unsigned int, incomingMsg->CommandField)
                       << "), we are a Storage SCP only");
          DCMNET_DEBUG(DIMSE_dumpMessage(tempStr, *incomingMsg, DIMSE_INCOMING));
          status = DIMSE_BADCOMMANDTYPE;
          }
        }
      return status;
    }
};

//------------------------------------------------------------------------------
class ctkDICOMStorageListenerPrivate
{
public:
  ctkDICOMStorageListenerPrivate();
  ~ctkDICOMStorageListenerPrivate();

  QString findFile(const QStringList& nameFilters, const QString& subDir)const;
  QString defaultConfigFile() const;

  QString ConnectionName;
  QString AETitle;
  int Port;
  QString JobUID;
  QList<QSharedPointer<ctkDICOMJobResponseSet>> JobResponseSets;

  ctkDICOMStorageListenerSCUPrivate SCU;
  bool Canceled;
};

//------------------------------------------------------------------------------
// ctkDICOMStorageListenerPrivate methods

//------------------------------------------------------------------------------
ctkDICOMStorageListenerPrivate::ctkDICOMStorageListenerPrivate()
{
  this->Port = 11112;
  this->AETitle = "CTKSTORE";
  this->Canceled = false;

  this->SCU.setConnectionBlockingMode(DUL_NOBLOCK);
  this->SCU.setACSETimeout(1);
  this->SCU.setConnectionTimeout(1);
  this->SCU.setRespondWithCalledAETitle(false);
  this->SCU.setHostLookupEnabled(true);
  this->SCU.setVerbosePCMode(false);
}

//------------------------------------------------------------------------------
ctkDICOMStorageListenerPrivate::~ctkDICOMStorageListenerPrivate()
{
}

//------------------------------------------------------------------------------
QString ctkDICOMStorageListenerPrivate::defaultConfigFile() const
{
  QString data;
  QString fileName(":/dicom/storescp.cfg");

  QFile readFile(fileName);
  if(readFile.open(QIODevice::ReadOnly))
    {
    data = readFile.readAll();
    }
    else
    {
    logger.error("Failed to find listener configuration file");
    return "";
    }

  readFile.close();

  QString tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
  QString configFile = tmpDir + "/storescp.cfg";
  QFile writeFile(configFile);

  if(writeFile.open(QFile::WriteOnly | QFile::Text))
    {
    QTextStream stream(&writeFile);
    stream << data;
    }
  else
    {
    logger.error("Failed to find listener configuration file");
    return "";
    }
  writeFile.close();

  return configFile;
}

//------------------------------------------------------------------------------
// ctkDICOMStorageListener methods

//------------------------------------------------------------------------------
ctkDICOMStorageListener::ctkDICOMStorageListener(QObject* parentObject)
  : QObject(parentObject)
  , d_ptr(new ctkDICOMStorageListenerPrivate)
{
  Q_D(ctkDICOMStorageListener);
  d->SCU.listener = this; // give the dcmtk level access to this for emitting signals

  this->setDCMTKLogLevel(logger.logLevel());
}

//------------------------------------------------------------------------------
ctkDICOMStorageListener::~ctkDICOMStorageListener()
{
  Q_D(ctkDICOMStorageListener);
  d->JobResponseSets.clear();
}

//-----------------------------------------------------------------------------
void ctkDICOMStorageListener::setDCMTKLogLevel(const ctkErrorLogLevel::LogLevel& level)
{
  OFLogger::LogLevel dcmtkLogLevel = OFLogger::OFF_LOG_LEVEL;
  if (level == ctkErrorLogLevel::LogLevel::Fatal)
    {
    dcmtkLogLevel = OFLogger::FATAL_LOG_LEVEL;
    }
  else if (level == ctkErrorLogLevel::LogLevel::Critical ||
           level == ctkErrorLogLevel::LogLevel::Error)
    {
    dcmtkLogLevel = OFLogger::ERROR_LOG_LEVEL;
    }
  else if (level == ctkErrorLogLevel::LogLevel::Warning)
    {
    dcmtkLogLevel = OFLogger::WARN_LOG_LEVEL;
    }
  else if (level == ctkErrorLogLevel::LogLevel::Info)
    {
    dcmtkLogLevel = OFLogger::INFO_LOG_LEVEL;
    }
  else if (level == ctkErrorLogLevel::LogLevel::Debug)
    {
    dcmtkLogLevel = OFLogger::DEBUG_LOG_LEVEL;
    }
  else if (level == ctkErrorLogLevel::LogLevel::Trace ||
           level == ctkErrorLogLevel::LogLevel::Status)
    {
    dcmtkLogLevel = OFLogger::TRACE_LOG_LEVEL;
    }

  OFLog::configure(dcmtkLogLevel);
}

//-----------------------------------------------------------------------------
ctkErrorLogLevel::LogLevel ctkDICOMStorageListener::DCMTKLogLevel() const
{
  return logger.logLevel();
}

//------------------------------------------------------------------------------
bool ctkDICOMStorageListener::listen()
{
  Q_D(ctkDICOMStorageListener);
  if (!this->initializeSCU())
    {
    return false;
    }

  OFCondition status = d->SCU.listen();
  if (status.bad() || d->Canceled)
    {
    logger.error(QString("SCP stopped, it was listening on port %1 : %2 ")
      .arg(QString::number(d->Port)).arg(status.text()));
    return false;
    }
  return true;
}

//------------------------------------------------------------------------------
bool ctkDICOMStorageListener::wasCanceled()
{
  Q_D(const ctkDICOMStorageListener);
  return d->Canceled;
}

//------------------------------------------------------------------------------
void ctkDICOMStorageListener::cancel()
{
  Q_D(ctkDICOMStorageListener);
  d->Canceled = true;
}

//------------------------------------------------------------------------------
bool ctkDICOMStorageListener::initializeSCU()
{
  Q_D(ctkDICOMStorageListener);
  d->SCU.setPort(this->port());
  d->SCU.setAETitle(OFString(this->AETitle().toStdString().c_str()));

  /* load association negotiation profile from configuration file (if specified) */
  OFCondition status = d->SCU.loadAssociationConfiguration(
    OFString(d->defaultConfigFile().toStdString().c_str()), "alldicom");
  if (status.bad())
    {
    logger.error(QString("Cannot load association configuration: %1").arg(status.text()));
    return false;
    }

  return true;
}

//------------------------------------------------------------------------------
void ctkDICOMStorageListener::setAETitle(const QString& AETitle)
{
  Q_D(ctkDICOMStorageListener);
  d->AETitle = AETitle;
}

//------------------------------------------------------------------------------
QString ctkDICOMStorageListener::AETitle() const
{
  Q_D(const ctkDICOMStorageListener);
  return d->AETitle;
}

//------------------------------------------------------------------------------
void ctkDICOMStorageListener::setPort (int port)
{
  Q_D(ctkDICOMStorageListener);
  d->Port = port;
}

//------------------------------------------------------------------------------
int ctkDICOMStorageListener::port()const
{
  Q_D(const ctkDICOMStorageListener);
  return d->Port;
}

//-----------------------------------------------------------------------------
void ctkDICOMStorageListener::setConnectionTimeout(const int timeout)
{
  Q_D(ctkDICOMStorageListener);
  d->SCU.setACSETimeout(timeout);
  d->SCU.setConnectionTimeout(timeout);
}

//-----------------------------------------------------------------------------
int ctkDICOMStorageListener::connectionTimeout()
{
  Q_D(const ctkDICOMStorageListener);
  return d->SCU.getConnectionTimeout();
}

//------------------------------------------------------------------------------
QList<ctkDICOMJobResponseSet *> ctkDICOMStorageListener::jobResponseSets() const
{
  Q_D(const ctkDICOMStorageListener);
  QList<ctkDICOMJobResponseSet *> jobResponseSets;
  foreach(QSharedPointer<ctkDICOMJobResponseSet> jobResponseSet, d->JobResponseSets)
  {
    jobResponseSets.append(jobResponseSet.data());
  }

  return jobResponseSets;
}

//------------------------------------------------------------------------------
QList<QSharedPointer<ctkDICOMJobResponseSet>> ctkDICOMStorageListener::jobResponseSetsShared() const
{
  Q_D(const ctkDICOMStorageListener);
  return d->JobResponseSets;
}

//------------------------------------------------------------------------------
static void skipDelete(QObject* obj)
{
  Q_UNUSED(obj);
  // this deleter does not delete the object from memory
  // useful if the pointer is not owned by the smart pointer
}

//------------------------------------------------------------------------------
void ctkDICOMStorageListener::addJobResponseSet(ctkDICOMJobResponseSet &jobResponseSet)
{
  this->addJobResponseSet(QSharedPointer<ctkDICOMJobResponseSet>(&jobResponseSet, skipDelete));
}

//------------------------------------------------------------------------------
void ctkDICOMStorageListener::addJobResponseSet(QSharedPointer<ctkDICOMJobResponseSet> jobResponseSet)
{
  Q_D(ctkDICOMStorageListener);
  d->JobResponseSets.append(jobResponseSet);
}

//------------------------------------------------------------------------------
void ctkDICOMStorageListener::removeJobResponseSet(QSharedPointer<ctkDICOMJobResponseSet> jobResponseSet)
{
  Q_D(ctkDICOMStorageListener);
  d->JobResponseSets.removeOne(jobResponseSet);
}

//------------------------------------------------------------------------------
void ctkDICOMStorageListener::setJobUID(const QString &jobUID)
{
  Q_D(ctkDICOMStorageListener);
  d->JobUID = jobUID;
}

//------------------------------------------------------------------------------
QString ctkDICOMStorageListener::jobUID() const
{
  Q_D(const ctkDICOMStorageListener);
  return d->JobUID;
}

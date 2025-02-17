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
#include "ctkDICOMEcho.h"
#include "ctkLogger.h"

// DCMTK includes
#include <dcmtk/dcmnet/scu.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/oflog/oflog.h>
#include <dcmtk/ofstd/ofcond.h>
#include <dcmtk/ofstd/ofstring.h>
#include <dcmtk/ofstd/oflist.h>
#include <dcmtk/ofstd/ofstd.h>        /* for class OFStandard */
#include <dcmtk/dcmdata/dcddirif.h>   /* for class DicomDirInterface */

static ctkLogger logger ( "org.commontk.dicom.DICOMEcho" );

//------------------------------------------------------------------------------
class ctkDICOMEchoPrivate
{
public:
  ctkDICOMEchoPrivate();
  ~ctkDICOMEchoPrivate();

  QString ConnectionName;
  QString CallingAETitle;
  QString CalledAETitle;
  QString Host;
  int Port;
  DcmSCU  SCU;
};

//------------------------------------------------------------------------------
// ctkDICOMEchoPrivate methods

//------------------------------------------------------------------------------
ctkDICOMEchoPrivate::ctkDICOMEchoPrivate()
{
  this->ConnectionName = "";
  this->CallingAETitle = "";
  this->CalledAETitle = "";
  this->Host = "";
  this->Port = 80;

  this->SCU.setACSETimeout(3);
  this->SCU.setConnectionTimeout(3);
}

//------------------------------------------------------------------------------
ctkDICOMEchoPrivate::~ctkDICOMEchoPrivate()
{
}

//------------------------------------------------------------------------------
// ctkDICOMEcho methods

//------------------------------------------------------------------------------
ctkDICOMEcho::ctkDICOMEcho(QObject* parentObject)
  : QObject(parentObject)
  , d_ptr(new ctkDICOMEchoPrivate)
{
  Q_D(ctkDICOMEcho);

  d->SCU.setVerbosePCMode(false);

  this->setDCMTKLogLevel(logger.logLevel());
}

//------------------------------------------------------------------------------
ctkDICOMEcho::~ctkDICOMEcho()
{
}

//-----------------------------------------------------------------------------
void ctkDICOMEcho::setDCMTKLogLevel(const ctkErrorLogLevel::LogLevel& level)
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
ctkErrorLogLevel::LogLevel ctkDICOMEcho::DCMTKLogLevel() const
{
  return logger.logLevel();
}

/// Set methods for connectivity
//------------------------------------------------------------------------------
void ctkDICOMEcho::setConnectionName(const QString& connectionName)
{
  Q_D(ctkDICOMEcho);
  d->ConnectionName = connectionName;
}

//------------------------------------------------------------------------------
QString ctkDICOMEcho::connectionName() const
{
  Q_D(const ctkDICOMEcho);
  return d->ConnectionName;
}

//------------------------------------------------------------------------------
void ctkDICOMEcho::setCallingAETitle(const QString& callingAETitle)
{
  Q_D(ctkDICOMEcho);
  d->CallingAETitle = callingAETitle;
}

//------------------------------------------------------------------------------
QString ctkDICOMEcho::callingAETitle() const
{
  Q_D(const ctkDICOMEcho);
  return d->CallingAETitle;
}

//------------------------------------------------------------------------------
void ctkDICOMEcho::setCalledAETitle(const QString& calledAETitle)
{
  Q_D(ctkDICOMEcho);
  d->CalledAETitle = calledAETitle;
}

//------------------------------------------------------------------------------
QString ctkDICOMEcho::calledAETitle()const
{
  Q_D(const ctkDICOMEcho);
  return d->CalledAETitle;
}

//------------------------------------------------------------------------------
void ctkDICOMEcho::setHost(const QString& host)
{
  Q_D(ctkDICOMEcho);
  d->Host = host;
}

//------------------------------------------------------------------------------
QString ctkDICOMEcho::host() const
{
  Q_D(const ctkDICOMEcho);
  return d->Host;
}

//------------------------------------------------------------------------------
void ctkDICOMEcho::setPort(int port)
{
  Q_D(ctkDICOMEcho);
  d->Port = port;
}

//------------------------------------------------------------------------------
int ctkDICOMEcho::port() const
{
  Q_D(const ctkDICOMEcho);
  return d->Port;
}

//-----------------------------------------------------------------------------
void ctkDICOMEcho::setConnectionTimeout(const int timeout)
{
  Q_D(ctkDICOMEcho);
  d->SCU.setACSETimeout(timeout);
  d->SCU.setConnectionTimeout(timeout);
}

//-----------------------------------------------------------------------------
int ctkDICOMEcho::connectionTimeout() const
{
  Q_D(const ctkDICOMEcho);
  return d->SCU.getConnectionTimeout();
}

//------------------------------------------------------------------------------
bool ctkDICOMEcho::echo()
{
  Q_D(ctkDICOMEcho);

  d->SCU.setPeerAETitle(OFString(this->calledAETitle().toStdString().c_str()));
  d->SCU.setPeerHostName(OFString(this->host().toStdString().c_str()));
  d->SCU.setPeerPort(this->port());

  logger.debug("Setting Transfer Syntaxes");

  OFList<OFString> transferSyntaxes;
  transferSyntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
  transferSyntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
  transferSyntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);

  d->SCU.addPresentationContext(UID_VerificationSOPClass, transferSyntaxes);
  if (!d->SCU.initNetwork().good())
    {
    logger.error("Error initializing the network");
    return false;
    }
  logger.debug("Negotiating Association");

  OFCondition result = d->SCU.negotiateAssociation();
  if (result.bad())
    {
    logger.error("Error negotiating the association: " + QString(result.text()));
    return false;
    }

  logger.debug("Seding Echo");
  // Issue ECHO request and let scu find presentation context itself (0)
  OFCondition status = d->SCU.sendECHORequest(0);
  if (!status.good())
    {
    logger.error("Echo failed");
    d->SCU.releaseAssociation();
    return false;
    }

  d->SCU.releaseAssociation();

  return true;
}

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

// CTK includes
#include <ctkLogger.h>
#include <iostream>

//-----------------------------------------------------------------------------
class ctkLoggerPrivate
{
public:
  ctkLoggerPrivate();
  ~ctkLoggerPrivate();
  ctkErrorLogLevel::LogLevel LogLevel;
};

//-----------------------------------------------------------------------------
ctkLoggerPrivate::ctkLoggerPrivate()
{
  this->LogLevel = ctkErrorLogLevel::LogLevel::Warning;
}
//-----------------------------------------------------------------------------
ctkLoggerPrivate::~ctkLoggerPrivate()
{

}

//-----------------------------------------------------------------------------
ctkLogger::ctkLogger(QString name, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new ctkLoggerPrivate)
{
  Q_UNUSED(name);
}

//-----------------------------------------------------------------------------
ctkLogger::~ctkLogger()
{
}

//-----------------------------------------------------------------------------
void ctkLogger::debug(const QString& s)
{
  Q_D(ctkLogger);
  if (d->LogLevel <= ctkErrorLogLevel::LogLevel::Debug)
  {
    qDebug().nospace() << qUtf8Printable(s);
  }
}

//-----------------------------------------------------------------------------
void ctkLogger::info(const QString& s)
{
  Q_D(ctkLogger);
  if (d->LogLevel <= ctkErrorLogLevel::LogLevel::Info)
  {
    qCritical().nospace() << qUtf8Printable(s);
  }
}

//-----------------------------------------------------------------------------
void ctkLogger::trace(const QString& s)
{
  Q_D(ctkLogger);
  if (d->LogLevel <= ctkErrorLogLevel::LogLevel::Trace)
  {
    qCritical().nospace() << qUtf8Printable(s);
  }
}

//-----------------------------------------------------------------------------
void ctkLogger::warn(const QString& s)
{
  Q_D(ctkLogger);
  if (d->LogLevel <= ctkErrorLogLevel::LogLevel::Warning)
  {
    qCritical().nospace() << qUtf8Printable(s);
  }
}

//-----------------------------------------------------------------------------
void ctkLogger::error(const QString& s)
{
  Q_D(ctkLogger);
  if (d->LogLevel <= ctkErrorLogLevel::LogLevel::Error)
  {
    qCritical().nospace() << qUtf8Printable(s);
  }
}

//-----------------------------------------------------------------------------
void ctkLogger::fatal(const QString& s)
{
  Q_D(ctkLogger);
  if (d->LogLevel <= ctkErrorLogLevel::LogLevel::Fatal)
  {
    qCritical().nospace() << qUtf8Printable(s);
  }
}

//-----------------------------------------------------------------------------
void ctkLogger::setLogLevel(const ctkErrorLogLevel::LogLevel& level)
{
  Q_D(ctkLogger);
  d->LogLevel = level;
}

//-----------------------------------------------------------------------------
ctkErrorLogLevel::LogLevel ctkLogger::logLevel() const
{
  Q_D(const ctkLogger);
  return d->LogLevel;
}


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
#include <QApplication>
#include <QTimer>

// ctk includes
#include "ctkCoreTestingMacros.h"
#include "ctkUtils.h"

// ctkDICOMWidget includes
#include "ctkDICOMStudyItemWidget.h"

// Test visual browser import functionality
int ctkDICOMStudyItemWidgetTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  ctkDICOMStudyItemWidget widget;
  // Test the default values
  CHECK_QSTRING(widget.studyItem(), "");
  CHECK_QSTRING(widget.patientID(), "");
  CHECK_QSTRING(widget.studyInstanceUID(), "");
  CHECK_QSTRING(widget.title(), "Study ID 1234 --- Date");
  CHECK_QSTRING(widget.description(), "");
  CHECK_QSTRING(widget.filteringSeriesDescription(), "");
  CHECK_BOOL(widget.collapsed(), false)
  CHECK_BOOL(widget.selection(), false)
  CHECK_INT(widget.numberOfSeriesPerRow(), 6);
  CHECK_INT(widget.thumbnailSize(), 300);

  // Test setting and getting
  widget.setStudyItem("1");
  CHECK_QSTRING(widget.studyItem(), "1");
  widget.setPatientID("123456");
  CHECK_QSTRING(widget.patientID(), "123456");
  widget.setStudyInstanceUID("123456.123");
  CHECK_QSTRING(widget.studyInstanceUID(), "123456.123");
  widget.setTitle("title");
  CHECK_QSTRING(widget.title(), "title");
  widget.setDescription("description");
  CHECK_QSTRING(widget.description(), "description");
  widget.setFilteringSeriesDescription("seriesDescription");
  CHECK_QSTRING(widget.filteringSeriesDescription(), "seriesDescription");
  widget.setCollapsed(true);
  CHECK_BOOL(widget.collapsed(), true);
  widget.setSelection(true);
  CHECK_BOOL(widget.selection(), true);
  widget.setNumberOfSeriesPerRow(12);
  CHECK_INT(widget.numberOfSeriesPerRow(), 12);
  widget.setThumbnailSize(150);
  CHECK_INT(widget.thumbnailSize(), 150);

  if (argc <= 2 || QString(argv[argc - 1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#include "pqCMBFileDialogEventTranslator.h"
#include "pqCoreTestUtility.h"

#include <pqCMBFileDialog.h>

#include <QEvent>
#include <QDir>
#include <QtDebug>

pqCMBFileDialogEventTranslator::pqCMBFileDialogEventTranslator(QObject* p) 
  : pqWidgetEventTranslator(p)
{
}

bool pqCMBFileDialogEventTranslator::translateEvent(QObject* Object, QEvent* Event, bool& /*Error*/)
{
  // Capture input for pqCMBFileDialog and all its children ...
  pqCMBFileDialog* object = 0;
  for(QObject* o = Object; o; o = o->parent())
    {
    object = qobject_cast<pqCMBFileDialog*>(o);
    if(object)
      break;
    }
  
  if(!object)
    return false;

  if(Event->type() == QEvent::FocusIn && !this->CurrentObject)
    {
    this->CurrentObject = object;
    connect(object, SIGNAL(fileAccepted(const QString&)), this, SLOT(onFilesSelected(const QString&)));
    connect(object, SIGNAL(rejected()), this, SLOT(onCancelled()));
    }
      
  return true;
}

void pqCMBFileDialogEventTranslator::onFilesSelected(const QString& file)
{
  QString data_directory = pqCoreTestUtility::DataRoot();
  data_directory = QDir::cleanPath(QDir::fromNativeSeparators(data_directory));
  if(data_directory.isEmpty())
    {
    qWarning() << "You must set the PARAVIEW_DATA_ROOT environment variable to play-back file selections.";
    }

  QString cleanedFile = QDir::cleanPath(QDir::fromNativeSeparators(file));
  
  if(cleanedFile.indexOf(data_directory, 0, Qt::CaseInsensitive) == 0)
    {
    cleanedFile.replace(data_directory, "$PARAVIEW_DATA_ROOT", Qt::CaseInsensitive);
    }
  else
    {
    qWarning() << "You must choose a file under the PARAVIEW_DATA_ROOT directory to record file selections.";
    }
  
  emit recordEvent(this->CurrentObject, "filesSelected", cleanedFile);
}

void pqCMBFileDialogEventTranslator::onCancelled()
{
  emit recordEvent(this->CurrentObject, "cancelled", "");
}

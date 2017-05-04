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

#include "pqCMBFileDialogEventPlayer.h"
#include "pqCoreTestUtility.h"

#include "pqCMBFileDialog.h"
#include "pqEventDispatcher.h"

#include <vtksys/SystemTools.hxx>

#include <QApplication>
#include <QtDebug>

//-----------------------------------------------------------------------------
pqCMBFileDialogEventPlayer::pqCMBFileDialogEventPlayer(QObject* p)
  : pqWidgetEventPlayer(p)
{
}

//-----------------------------------------------------------------------------
bool pqCMBFileDialogEventPlayer::playEvent(
  QObject* Object, const QString& Command, const QString& Arguments, bool& Error)
{
  // Handle playback for pqCMBFileDialog and all its children ...
  pqCMBFileDialog* object = 0;
  for (QObject* o = Object; o; o = o->parent())
  {
    object = qobject_cast<pqCMBFileDialog*>(o);
    if (object)
      break;
  }
  if (!object)
    return false;

  QString fileString = Arguments;

  const QString data_directory = pqCoreTestUtility::DataRoot();
  if (fileString.contains("PARAVIEW_DATA_ROOT") && data_directory.isEmpty())
  {
    qCritical()
      << "You must set the PARAVIEW_DATA_ROOT environment variable to play-back file selections.";
    Error = true;
    return true;
  }

  const QString test_directory = pqCoreTestUtility::TestDirectory();
  if (fileString.contains("PARAVIEW_TEST_ROOT") && test_directory.isEmpty())
  {
    qCritical() << "You must specify --test-directory in the command line options.";
    Error = true;
    return true;
  }

  if (Command == "filesSelected")
  {
    fileString.replace("$PARAVIEW_DATA_ROOT", data_directory);
    fileString.replace("$PARAVIEW_TEST_ROOT", test_directory);

    if (object->selectFile(fileString))
    {
      pqEventDispatcher::processEventsAndWait(0);
    }
    else
    {
      qCritical() << "Dialog couldn't accept " << fileString;
      Error = true;
    }

    return true;
  }

  if (Command == "cancelled")
  {
    object->reject();
    return true;
  }
  if (Command == "remove")
  {
    // Delete the file.
    fileString.replace("$PARAVIEW_DATA_ROOT", data_directory);
    fileString.replace("$PARAVIEW_TEST_ROOT", test_directory);
    vtksys::SystemTools::RemoveFile(fileString.toLatin1().data());
    return true;
  }

  qCritical() << "Unknown pqCMBFileDialog command: " << Object << " " << Command << " "
              << Arguments;
  Error = true;
  return true;
}

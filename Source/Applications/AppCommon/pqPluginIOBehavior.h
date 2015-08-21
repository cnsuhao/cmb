//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __qtPluginIOBehavior_h
#define __qtPluginIOBehavior_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include "cmbSystemConfig.h"

#include <QMap>
#include <QPair>

class vtkSMSession;
class vtkPVXMLElement;

/// This behavior handles readers and writers that are loaded from plugins.
class CMBAPPCOMMON_EXPORT pqPluginIOBehavior : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;
  typedef QMap<QString, QPair<QString, QString> > FileExtMap;
public:
  pqPluginIOBehavior(QObject* parent=0);
  ~pqPluginIOBehavior();

  // Description:
  // Returns a formatted string with all supported file types.
  // An example returned string would look like:
  // "LAS Files (*.las);;Moab Files (*.moab)"
  const char* supportedFileTypes(vtkSMSession* session);

  // Description:
  // Returns the map <file_extension, <reader_group, reader_name> >
  // Example <brep, <sources, CmbMoabSolidReader> >
  FileExtMap fileExtensionMap(vtkSMSession* session);

  // Description:
  // check if the proxy is from a plugin by examining the hints
  static bool isPluginReader(vtkPVXMLElement* hints);

protected slots:
  void updateResources();

private:
  Q_DISABLE_COPY(pqPluginIOBehavior)
  class cmbInternals;
  cmbInternals* Internals;
};

#endif

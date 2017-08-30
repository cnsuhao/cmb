//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBRenderLog - a render log for the log window in CMB applications.
// .SECTION Description
//  This class contains an SMTK logger that is connected to the main log window
//  of CMB applications via a qtEmittingStringBuffer (a string buffer that emits
// whenever it is updated). It also makes the one-time connection between Qt
// output and VTK's output and between VTK's output and its internal logger.
// .SECTION Caveats

#ifndef _pqCMBRenderLog_h
#define _pqCMBRenderLog_h

#include <QObject>

#include "pqOutputWidget.h"

#include "smtk/extension/qt/qtEmittingStringBuffer.h"
#include "smtk/io/Logger.h"

class pqCMBRenderLog : public QObject
{
  Q_OBJECT

public:
  pqCMBRenderLog();

  virtual ~pqCMBRenderLog();

public slots:
  void onUpdate();

signals:
  void renderLog(const smtk::io::Logger&);

private:
  smtk::io::Logger Logger;
  smtk::extension::qtEmittingStringBuffer Stringbuf;
};

#endif

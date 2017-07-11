//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _pqCMBRenderLog_h
#define _pqCMBRenderLog_h

#include <QObject>

#include "pqOutputWidget.h"

#include "smtk/extension/qt/qtEmittingStringBuffer.h"

#include "smtk/extension/vtk/io/RedirectOutput.h"

#include "smtk/io/Logger.h"

class pqCMBRenderLog : public QObject
{
  Q_OBJECT

public:
  pqCMBRenderLog()
  {
    // Redirect all VTK output to the logger.
    smtk::extension::vtk::io::RedirectVTKOutputTo(this->Logger);

    // Redirect all Qt output to the the VTK output window, which now gets sent
    // to the logger.
    pqOutputWidget::installQMessageHandler();

    // Pass the ostream to the logger, and set it to be owned by the logger.
    this->Logger.setFlushToStream(new std::ostream(&this->Stringbuf), true, false);

    // Connect the emitting string buffer's flush signal to our onUpdate slot,
    // which in turn emits our log and then clears its contents.
    QObject::connect(&this->Stringbuf, SIGNAL(flush()), this, SLOT(onUpdate()));
  }

  virtual ~pqCMBRenderLog() {}

public slots:
  // Emit the log and then clear its contents.
  void onUpdate()
  {
    emit renderLog(this->Logger);
    this->Logger.reset();
  }

signals:
  void renderLog(const smtk::io::Logger&);

private:
  smtk::io::Logger Logger;
  smtk::extension::qtEmittingStringBuffer Stringbuf;
};

#endif

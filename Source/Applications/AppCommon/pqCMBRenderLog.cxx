//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBRenderLog.h"

#include "smtk/extension/vtk/io/RedirectOutput.h"

pqCMBRenderLog::pqCMBRenderLog()
{
  // Redirect all VTK output to the logger.
  smtk::extension::vtk::io::RedirectVTKOutputTo(this->Logger);

  // Redirect all Qt output to the the VTK output window, which now gets sent
  // to the logger.
  MessageHandler::install();

  // Pass the ostream to the logger, and set it to be owned by the logger.
  this->Logger.setFlushToStream(new std::ostream(&this->Stringbuf), true, false);

  // Connect the emitting string buffer's flush signal to our onUpdate slot,
  // which in turn emits our log and then clears its contents.
  QObject::connect(&this->Stringbuf, SIGNAL(flush()), this, SLOT(onUpdate()));
}

pqCMBRenderLog::~pqCMBRenderLog()
{
}

void pqCMBRenderLog::onUpdate()
{
  // Emit the log and then clear its contents.

  std::cerr << this->Logger.convertToString(true) << std::endl;

  emit renderLog(this->Logger);
  this->Logger.reset();
}

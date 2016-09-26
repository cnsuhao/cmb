//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBLIDARSaveDialog - manages the node's texture information.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqCMBLIDARSaveDialog_h
#define __pqCMBLIDARSaveDialog_h

#include <QObject>
#include <QStringList>
#include <vector>
#include "cmbSystemConfig.h"

class QDialog;
class pqPipelineSource;
class pqServer;

namespace Ui
{
  class  qtSaveScatterData;
};

class pqCMBLIDARSaveDialog : public QObject
{
  Q_OBJECT

public:
  static int getFile(QWidget *parent, pqServer *server, bool enableSavePieces,
                     QString *name, bool *saveAsSinglePiece,
                     bool *loadAsDisplayed);

protected slots:
  void accept();
  void cancel();
  void displayFileBrowser();
  void filesSelected(const QStringList &files);

protected:
  pqCMBLIDARSaveDialog(QWidget *parent, pqServer *server, bool enableSavePieces);
  virtual ~pqCMBLIDARSaveDialog();
  int exec();
  int Status;
  Ui::qtSaveScatterData *SaveDialog;
  QDialog *MainDialog;
  bool SaveAsSinglePiece;
  bool SaveAsDisplayed;
  QString FileName;
  pqServer *Server;
};

#endif /* __pqCMBLIDARSaveDialog_h */

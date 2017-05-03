//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtJobsPanel - display jobs on remote system
// .SECTION Description

#ifndef __CmbJobsPanels_h
#define __CmbJobsPanels_h

#include <QDockWidget>

class qtJobsPanel : public QDockWidget
{
  Q_OBJECT

public:
  qtJobsPanel(QWidget* parent);
  virtual ~qtJobsPanel();

signals:
  void resultDownloaded(const QString& path);

public slots:

protected slots:
  void authenticateHPC();

private:
  class qtJobsPanelInternal;
  qtJobsPanelInternal* Internal;
};

#endif // __CmbJobsPanels_h

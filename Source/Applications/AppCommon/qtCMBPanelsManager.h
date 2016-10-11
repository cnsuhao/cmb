//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBPanelsManager -
// .SECTION Description

#ifndef __CmbPanelsManager_h
#define __CmbPanelsManager_h

#include <QObject>
#include <QPointer>
#include "cmbAppCommonExport.h"
#include "smtk/PublicPointerDefs.h"

class QMainWindow;
class QDockWidget;

class CMBAPPCOMMON_EXPORT qtCMBPanelsManager : public QObject
{
  Q_OBJECT

public:
      enum PanelType
      {
        ATTRIBUTE = 0,
        MODEL,
        MESH,
        SCENE,
        INFO,
        PROPERTIES,
        DISPLAY,
        RENDER,
        COLORMAP,
        JOBS,
        NUMBER_OF_KNOWN_TYPES,
        // User defined types
        USER_DEFINED=100,
      };

  qtCMBPanelsManager(QObject* p);
  ~qtCMBPanelsManager() override;

  QDockWidget* createDockWidget (QMainWindow* mw,
    QWidget* content, const std::string& title,
    Qt::DockWidgetArea dockarea, QDockWidget* lastdw);

  virtual void setPanelTypes(const QList<qtCMBPanelsManager::PanelType>&);
  const QList<qtCMBPanelsManager::PanelType>& panelTypes() const;

  static std::string type2String(qtCMBPanelsManager::PanelType t);
  static qtCMBPanelsManager::PanelType string2Type(const std::string &s);

signals :

public slots:

protected:

private:
  class Internal;
  Internal* const mgrInternal;

};

#endif

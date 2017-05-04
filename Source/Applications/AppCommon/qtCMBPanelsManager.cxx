//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCMBPanelsManager.h"

#include <QDockWidget>
#include <QList>
#include <QMainWindow>
#include <QMessageBox>
#include <QScrollArea>
#include <QSet>
#include <QVBoxLayout>

class qtCMBPanelsManager::Internal
{
public:
  Internal() {}
  ~Internal() {}
  QList<qtCMBPanelsManager::PanelType> PanelTypes;
};

//----------------------------------------------------------------------------
qtCMBPanelsManager::qtCMBPanelsManager(QObject* p)
  : QObject(p)
  , mgrInternal(new Internal())
{
}

//----------------------------------------------------------------------------
qtCMBPanelsManager::~qtCMBPanelsManager()
{
  delete this->mgrInternal;
}

//-----------------------------------------------------------------------------
QDockWidget* qtCMBPanelsManager::createDockWidget(QMainWindow* mw, QWidget* content,
  const std::string& title, Qt::DockWidgetArea dockarea, QDockWidget* lastdw)
{
  QDockWidget* dw = new QDockWidget(mw);
  QWidget* container = new QWidget();
  container->setObjectName("dockscrollWidget");
  container->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

  QScrollArea* s = new QScrollArea(dw);
  s->setWidgetResizable(true);
  s->setFrameShape(QFrame::NoFrame);
  s->setObjectName("scrollArea");
  s->setWidget(container);

  QVBoxLayout* vboxlayout = new QVBoxLayout(container);
  vboxlayout->setMargin(0);
  vboxlayout->addWidget(content);

  QString dockTitle(title.c_str());
  dw->setWindowTitle(dockTitle);
  dw->setObjectName(dockTitle.append("dockWidget"));
  dw->setWidget(s);
  mw->addDockWidget(dockarea, dw);
  if (lastdw)
  {
    mw->tabifyDockWidget(lastdw, dw);
  }
  return dw;
}

//----------------------------------------------------------------------------
void qtCMBPanelsManager::setPanelTypes(const QList<qtCMBPanelsManager::PanelType>& ptypes)
{
  this->mgrInternal->PanelTypes = ptypes;
}
//----------------------------------------------------------------------------
const QList<qtCMBPanelsManager::PanelType>& qtCMBPanelsManager::panelTypes() const
{
  return this->mgrInternal->PanelTypes;
}
//----------------------------------------------------------------------------
std::string qtCMBPanelsManager::type2String(qtCMBPanelsManager::PanelType t)
{
  switch (t)
  {
    case ATTRIBUTE:
      return "Attribute";
    case MODEL:
      return "Model";
    case PROPERTIES:
      return "Properties";
    case MESH:
      return "Mesh";
    case SCENE:
      return "Scene";
    case INFO:
      return "Info";
    case RENDER:
      return "Render";
    case DISPLAY:
      return "Display";
    case COLORMAP:
      return "Color Map";
    case JOBS:
      return "Jobs";
    default:
      return "";
  }
  return "Error!";
}
//----------------------------------------------------------------------------
qtCMBPanelsManager::PanelType qtCMBPanelsManager::string2Type(const std::string& s)
{
  if (s == "Attribute")
  {
    return ATTRIBUTE;
  }
  if (s == "Model")
  {
    return MODEL;
  }
  if (s == "Properties")
  {
    return PROPERTIES;
  }
  if (s == "Mesh")
  {
    return MESH;
  }
  if (s == "Scene")
  {
    return SCENE;
  }
  if (s == "Info")
  {
    return INFO;
  }
  if (s == "Render")
  {
    return RENDER;
  }
  if (s == "Display")
  {
    return DISPLAY;
  }
  if (s == "Color Map")
  {
    return COLORMAP;
  }
  if (s == "Jobs")
  {
    return JOBS;
  }
  return NUMBER_OF_KNOWN_TYPES;
}

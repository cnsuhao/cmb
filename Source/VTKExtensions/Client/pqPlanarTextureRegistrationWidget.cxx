//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqPlanarTextureRegistrationWidget.h"
#include "pqPlanarTextureRegistrationDialog.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqPipelineFilter.h"
#include "pqPropertiesPanel.h"
#include "pqPropertyLinks.h"
#include "pqProxyWidgetDialog.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServerManagerModel.h"
#include "vtkAlgorithm.h"
#include "vtkImageData.h"
#include "vtkPVXMLElement.h"
#include "vtkSMProperty.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyIterator.h"
#include "vtkSMProxyManager.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QtDebug>

#include "pqRepresentationHelperFunctions.h"
using namespace RepresentationHelperFunctions;

class pqPlanarTextureRegistrationWidget::pqImplementation
{
public:
  pqImplementation()
    : TextureControls(0)
  {
    this->ClearTexture();
  }
  ~pqImplementation()
  {
    this->ClearTexture();
    if (this->TextureControls)
    {
      delete this->TextureControls;
    }
  }
  void ClearTexture()
  {
    this->TextureFileName = "";
    if (this->ImageTexture)
    {
      vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
      pxm->UnRegisterProxy("textures", "ImageTexture", this->ImageTexture);
      this->ImageTexture = NULL;
    }
    this->NumberOfRegistrationPoints = 0;
    for (int i = 0; i < 12; i++)
    {
      this->RegistrationPoints[i] = 0.0;
    }
    this->isSettingTexture = false;
  }
  bool HasTexture() { return this->ImageTexture && !this->TextureFileName.isEmpty(); }

  /// Provides the Qt controls for the widget
  pqPlanarTextureRegistrationDialog* TextureControls;
  vtkSmartPointer<vtkSMProxy> ImageTexture;
  QString TextureFileName;
  int NumberOfRegistrationPoints;
  double RegistrationPoints[12];
  bool isSettingTexture;
};

pqPlanarTextureRegistrationWidget::pqPlanarTextureRegistrationWidget(
  vtkSMProxy* smproxy, vtkSMProperty*, QWidget* parentObject)
  : Superclass(smproxy, parentObject)
  , Implementation(new pqImplementation())
{
  this->setShowLabel(false);

  pqRenderView* view = qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView());

  pqProxy* object_proxy =
    pqApplicationCore::instance()->getServerManagerModel()->findItem<pqProxy*>(smproxy);

  pqPipelineFilter* filter = qobject_cast<pqPipelineFilter*>(object_proxy);
  pqDataRepresentation* inPlaneRep = filter->getInput(0)->getRepresentation(view);
  if (!inPlaneRep)
  {
    return;
  }

  double bounds[6];

  GetRepresentationTransformedBounds(inPlaneRep, bounds);

  this->Implementation->TextureControls = new pqPlanarTextureRegistrationDialog(
    pqActiveObjects::instance().activeServer(), view, "Texture Registration", parentObject);
  QObject::connect(this->Implementation->TextureControls, SIGNAL(removeCurrentTexture()), this,
    SLOT(unsetTextureMap()), Qt::QueuedConnection);
  QObject::connect(this->Implementation->TextureControls,
    SIGNAL(registerCurrentTexture(const QString&, int, double*)), this,
    SLOT(setTextureMap(const QString&, int, double*)), Qt::QueuedConnection);

  QStringList textureslist;
  this->fetchTextureFiles(textureslist);
  this->Implementation->TextureControls->initializeTexture(bounds, textureslist,
    this->Implementation->TextureFileName, this->Implementation->RegistrationPoints,
    this->Implementation->NumberOfRegistrationPoints, true);

  QVBoxLayout* const layout = new QVBoxLayout(this);
  layout->addWidget(this->Implementation->TextureControls->getMainDialog());
  layout->addStretch();

  this->connect(object_proxy, SIGNAL(producerChanged(const QString&)), SLOT(updateEnableState()),
    Qt::QueuedConnection);

  QObject::connect(this->Implementation->TextureControls, SIGNAL(dialogModified()), this,
    SIGNAL(changeAvailable()));

  this->updateEnableState();

  PV_DEBUG_PANELS() << "pqPlanarTextureRegistrationWidget.";
}

pqPlanarTextureRegistrationWidget::~pqPlanarTextureRegistrationWidget()
{
  delete this->Implementation;
}

void pqPlanarTextureRegistrationWidget::apply()
{
  this->Implementation->TextureControls->apply();
  this->pqPropertyWidget::apply();
}

void pqPlanarTextureRegistrationWidget::reset()
{
  this->Implementation->TextureControls->getLinks()->reset();
}

void pqPlanarTextureRegistrationWidget::updateEnableState()
{
}

void pqPlanarTextureRegistrationWidget::unsetTextureMap()
{
  this->Implementation->ClearTexture();
  if (vtkSMProperty* textureProp = this->getTextureProperty())
  {
    vtkSMPropertyHelper(textureProp).Set(static_cast<vtkSMProxy*>(0));
    this->getRepresentation()->getProxy()->UpdateVTKObjects();
  }

  vtkSMPropertyHelper(this->proxy(), "GenerateCoordinates").Set(0);
  this->proxy()->UpdateVTKObjects();
  if (this->getRepresentation())
  {
    this->Implementation->TextureControls->currentView()->forceRender();
  }
}

void pqPlanarTextureRegistrationWidget::setTextureMap(
  const QString& tmpfilename, int numberOfRegistrationPoints, double* points)
{
  if (this->Implementation->isSettingTexture)
  {
    return;
  }
  this->Implementation->isSettingTexture = true;

  if (this->Implementation->HasTexture() &&
    !this->Implementation->TextureFileName.compare(tmpfilename))
  {
    this->unsetTextureMap();
  }
  this->Implementation->TextureFileName = tmpfilename;
  this->Implementation->NumberOfRegistrationPoints = numberOfRegistrationPoints;
  int i, n = 4 * numberOfRegistrationPoints;
  for (i = 0; i < n; i++)
  {
    this->Implementation->RegistrationPoints[i] = points[i];
  }

  if (!this->Implementation->ImageTexture)
  {
    vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
    this->Implementation->ImageTexture.TakeReference(pxm->NewProxy("textures", "ImageTexture"));
    pqSMAdaptor::setElementProperty(this->Implementation->ImageTexture->GetProperty("FileName"),
      this->Implementation->TextureFileName);
    this->Implementation->ImageTexture->UpdateVTKObjects();
  }

  if (vtkSMProperty* textureProp = this->getTextureProperty())
  {
    pqSMAdaptor::setProxyProperty(textureProp, this->Implementation->ImageTexture);
  }

  if (pqPipelineSource* imgSourc = this->Implementation->TextureControls->getTextureImageSource())
  {
    vtkAlgorithm* source = vtkAlgorithm::SafeDownCast(imgSourc->getProxy()->GetClientSideObject());
    vtkImageData* image = vtkImageData::SafeDownCast(source->GetOutputDataObject(0));

    int extents[6];
    double ev[2];
    image->GetExtent(extents);
    ev[0] = extents[0];
    ev[1] = extents[1];
    vtkSMPropertyHelper(this->proxy(), "SRange").Set(ev, 2);
    ev[0] = extents[2];
    ev[1] = extents[3];
    vtkSMPropertyHelper(this->proxy(), "TRange").Set(ev, 2);
  }

  if (numberOfRegistrationPoints == 2)
  {
    vtkSMPropertyHelper(this->proxy(), "TwoPointRegistration")
      .Set(this->Implementation->RegistrationPoints, 8);
  }
  else
  {
    vtkSMPropertyHelper(this->proxy(), "ThreePointRegistration")
      .Set(this->Implementation->RegistrationPoints, 12);
  }

  vtkSMPropertyHelper(this->proxy(), "GenerateCoordinates").Set(1);
  this->proxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(this->proxy())->UpdatePipeline();
  if (this->getRepresentation())
  {
    this->getRepresentation()->getProxy()->UpdateVTKObjects();
    this->Implementation->TextureControls->currentView()->forceRender();
  }

  this->Implementation->isSettingTexture = false;
}

vtkSMProperty* pqPlanarTextureRegistrationWidget::getTextureProperty()
{
  vtkSMProperty* textureProperty(0);
  pqDataRepresentation* thisRep = this->getRepresentation();
  if (thisRep)
  {
    vtkSMProperty* lti = thisRep->getProxy()->GetProperty("LargeTextureInput");
    textureProperty = lti ? lti : thisRep->getProxy()->GetProperty("Texture");
  }
  return textureProperty;
}

pqDataRepresentation* pqPlanarTextureRegistrationWidget::getRepresentation()
{
  pqRenderView* view = this->Implementation->TextureControls->currentView();

  pqProxy* object_proxy =
    pqApplicationCore::instance()->getServerManagerModel()->findItem<pqProxy*>(this->proxy());

  pqPipelineFilter* filter = qobject_cast<pqPipelineFilter*>(object_proxy);

  pqDataRepresentation* selfRep = filter->getRepresentation(view);
  if (!selfRep)
  {
    pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
    selfRep = builder->createDataRepresentation(filter->getOutputPort(0), view);
  }
  return selfRep;
}

void pqPlanarTextureRegistrationWidget::fetchTextureFiles(QStringList& textureslist)
{
  textureslist.clear();
  // Get all proxies under "textures" group and add them to the menu.
  vtkNew<vtkSMProxyIterator> proxyIter;
  proxyIter->SetModeToOneGroup();
  // Look for proxy that in the current active server/session
  proxyIter->SetSession(pqApplicationCore::instance()->getActiveServer()->session());
  for (proxyIter->Begin("textures"); !proxyIter->IsAtEnd(); proxyIter->Next())
  {
    QString filename =
      pqSMAdaptor::getElementProperty(proxyIter->GetProxy()->GetProperty("FileName")).toString();
    if (!textureslist.contains(filename))
    {
      textureslist.push_back(filename);
    }
  }
}

void pqPlanarTextureRegistrationWidget::updateTextureList()
{
  QStringList textureslist;
  this->fetchTextureFiles(textureslist);
  this->Implementation->TextureControls->updateTextureList(
    textureslist, this->Implementation->TextureFileName);
}

void pqPlanarTextureRegistrationWidget::proxyRegistered(const QString& group)
{
  if (group == "textures")
  {
    this->updateTextureList();
  }
}

void pqPlanarTextureRegistrationWidget::proxyUnRegistered(
  const QString& group, const QString&, vtkSMProxy* /*proxy*/)
{
  if (group == "textures")
  {
    pqTimer::singleShot(0, this, SLOT(updateTextureList()));
  }
}

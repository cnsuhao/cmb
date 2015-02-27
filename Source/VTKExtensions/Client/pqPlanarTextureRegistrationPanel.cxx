/*=========================================================================

   Program: ParaView
   Module:    pqPlanarTextureRegistrationPanel.cxx

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "pqPlanarTextureRegistrationPanel.h"
#include "pqPlanarTextureRegistrationDialog.h"

#include "pqApplicationCore.h"
#include "pqCollapsedGroup.h"
#include "pqNamedWidgets.h"
#include "pqOutputPort.h"
#include "pqPipelineFilter.h"
#include "pqPropertyManager.h"
#include "pqActiveObjects.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServerManagerObserver.h"
#include "pqPropertyLinks.h"

#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMSourceProxy.h"
#include "vtkPVDataInformation.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSMProxyIterator.h"
#include "vtkNew.h"
#include "vtkAlgorithm.h"
#include "vtkImageData.h"
#include "vtkSMProxyManager.h"
#include <vtksys/SystemTools.hxx>

#include <QCheckBox>
#include <QVBoxLayout>
#include <QDialog>
#include <QFileInfo>

#include "pqRepresentationHelperFunctions.h"
using namespace RepresentationHelperFunctions;

//////////////////////////////////////////////////////////////////////////////
// pqPlanarTextureRegistrationPanel::pqImplementation

class pqPlanarTextureRegistrationPanel::pqImplementation
{
public:
  pqImplementation() : TextureControls(0)
    {
    this->ClearTexture();
   }
  ~pqImplementation()
    {
    this->ClearTexture();
    if(this->TextureControls)
      {
      delete this->TextureControls;
      }
    }
  void ClearTexture()
    {
    this->TextureFileName="";
    if(this->ImageTexture)
      {
      vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
      pxm->UnRegisterProxy("textures", "ImageTexture", this->ImageTexture);
      this->ImageTexture = NULL;
      }
    this->NumberOfRegistrationPoints=0;
    for(int i=0; i<12; i++)
      {
      this->RegistrationPoints[i]=0.0;
      }
    this->isSettingTexture = false;
    }
  bool HasTexture()
    {
    return this->ImageTexture && !this->TextureFileName.isEmpty();
    }

  /// Provides the Qt controls for the panel
  pqPlanarTextureRegistrationDialog* TextureControls;
  vtkSmartPointer<vtkSMProxy> ImageTexture;
  QString TextureFileName;
  int NumberOfRegistrationPoints;
  double RegistrationPoints[12];
  bool isSettingTexture;
};

pqPlanarTextureRegistrationPanel::pqPlanarTextureRegistrationPanel(pqProxy* object_proxy, QWidget* p) :
  Superclass(object_proxy, p),
  Implementation(new pqImplementation())
{
  pqRenderView* view = qobject_cast<pqRenderView*>(
    pqActiveObjects::instance().activeView());

  pqPipelineFilter* filter = qobject_cast<pqPipelineFilter*>(object_proxy);
  pqDataRepresentation* inPlaneRep = filter->getInput(0)->getRepresentation(view);
  if(!inPlaneRep)
    {
    return;
    }

  double bounds[6];
  GetRepresentationTransformedBounds(inPlaneRep, bounds);

  this->Implementation->TextureControls = new pqPlanarTextureRegistrationDialog(
    object_proxy->getServer(), view, "Texture Registration", p);
  QObject::connect(this->Implementation->TextureControls, SIGNAL(removeCurrentTexture()),
    this, SLOT(unsetTextureMap()), Qt::QueuedConnection);
  QObject::connect(this->Implementation->TextureControls,
    SIGNAL(registerCurrentTexture(const QString&, int, double *)),
    this, SLOT(setTextureMap(const QString&, int, double*)), Qt::QueuedConnection);

  //updateTextureList()

  QStringList textureslist;
  this->fetchTextureFiles(textureslist);
  this->Implementation->TextureControls->initializeTexture(bounds,
    textureslist, this->Implementation->TextureFileName,
    this->Implementation->RegistrationPoints,
    this->Implementation->NumberOfRegistrationPoints, true);

  //this->Implementation->ControlsContainer.layout()->setMargin(0);
  QVBoxLayout* const panel_layout = new QVBoxLayout(this);
  //panel_layout->addWidget(&this->Implementation->ControlsContainer);
  panel_layout->addWidget(this->Implementation->TextureControls->getMainDialog());
  panel_layout->addStretch();

  QObject::connect(this->propertyManager(), SIGNAL(accepted()),
    this, SLOT(onAccepted()), Qt::QueuedConnection);
  QObject::connect(this->propertyManager(), SIGNAL(rejected()),
    this, SLOT(onRejected()), Qt::QueuedConnection);

/*
  pqServerManagerObserver* observer =
    pqApplicationCore::instance()->getServerManagerObserver();
  QObject::connect(observer,
    SIGNAL(proxyRegistered(const QString&, const QString&, vtkSMProxy*)),
    this,
    SLOT(proxyRegistered(const QString&)), Qt::QueuedConnection);
  QObject::connect(observer,
    SIGNAL(proxyUnRegistered(const QString&, const QString&, vtkSMProxy*)),
    this,
    SLOT(proxyUnRegistered(const QString&, const QString&, vtkSMProxy*)),
    Qt::QueuedConnection);
*/
  pqNamedWidgets::link(
    this->Implementation->TextureControls->getMainDialog(), this->proxy(), this->propertyManager());
  QObject::connect(this->Implementation->TextureControls->getLinks(),
    SIGNAL(qtWidgetChanged()), this, SLOT(setModified()), Qt::QueuedConnection);
  QObject::connect(this->Implementation->TextureControls->getLinks(),
    SIGNAL(smPropertyChanged()), this, SLOT(setModified()), Qt::QueuedConnection);

  QObject::connect(object_proxy, SIGNAL(producerChanged(const QString&)),
    this, SLOT(updateEnableState()), Qt::QueuedConnection);
  this->updateEnableState();

}

pqPlanarTextureRegistrationPanel::~pqPlanarTextureRegistrationPanel()
{
  delete this->Implementation;
}
//-----------------------------------------------------------------------------
void pqPlanarTextureRegistrationPanel::onAccepted()
{
  this->Implementation->TextureControls->apply();
}

//-----------------------------------------------------------------------------
void pqPlanarTextureRegistrationPanel::onRejected()
{
  this->Implementation->TextureControls->getLinks()->reset();
}

void pqPlanarTextureRegistrationPanel::updateEnableState()
{

}
//-----------------------------------------------------------------------------
void pqPlanarTextureRegistrationPanel::unsetTextureMap()
{
  this->Implementation->ClearTexture();
  if (vtkSMProperty* textureProp = this->getTextureProperty())
    {
    vtkSMPropertyHelper(textureProp).Set(static_cast<vtkSMProxy*>(0));
    this->getRepresentation()->getProxy()->UpdateVTKObjects();
    }

  vtkSMPropertyHelper(this->referenceProxy()->getProxy(),
                      "GenerateCoordinates").Set(0);
  this->referenceProxy()->getProxy()->UpdateVTKObjects();
  if(this->getRepresentation())
    {
    this->Implementation->TextureControls->currentView()->forceRender();
    }
}

//-----------------------------------------------------------------------------
void pqPlanarTextureRegistrationPanel::setTextureMap(
  const QString& tmpfilename, int numberOfRegistrationPoints, double *points)
{
  if(this->Implementation->isSettingTexture)
    {
    return;
    }
  this->Implementation->isSettingTexture = true;

  if (this->Implementation->HasTexture() &&
     !this->Implementation->TextureFileName.compare( tmpfilename ))
    {
    this->unsetTextureMap();
    }
  this->Implementation->TextureFileName = tmpfilename;
  this->Implementation->NumberOfRegistrationPoints = numberOfRegistrationPoints;
  int i, n = 4*numberOfRegistrationPoints;
  for (i = 0; i < n; i++)
    {
    this->Implementation->RegistrationPoints[i] = points[i];
    }

  if(!this->Implementation->ImageTexture)
    {
    vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
    this->Implementation->ImageTexture.TakeReference(
      pxm->NewProxy("textures", "ImageTexture"));
    pqSMAdaptor::setElementProperty(
      this->Implementation->ImageTexture->GetProperty("FileName"),
      this->Implementation->TextureFileName);
//    pqSMAdaptor::setEnumerationProperty(
//      this->Implementation->ImageTexture->GetProperty("SourceProcess"), "Client");
    this->Implementation->ImageTexture->UpdateVTKObjects();
//    QFileInfo finfo(this->Implementation->TextureFileName);
//    pxm->RegisterProxy("textures", finfo.fileName().toAscii().data(),
//      this->Implementation->ImageTexture);
    }

  if (vtkSMProperty* textureProp = this->getTextureProperty())
    {
    pqSMAdaptor::setProxyProperty(textureProp, this->Implementation->ImageTexture);
    }

  if(pqPipelineSource* imgSourc = this->Implementation->TextureControls->getTextureImageSource())
    {
    vtkAlgorithm* source = vtkAlgorithm::SafeDownCast(
      imgSourc->getProxy()->GetClientSideObject());
    vtkImageData* image = vtkImageData::SafeDownCast(source->GetOutputDataObject(0));

    int extents[6];
    double ev[2];
    image->GetExtent(extents);
    ev[0] = extents[0];
    ev[1] = extents[1];
    vtkSMPropertyHelper(this->referenceProxy()->getProxy(),
      "SRange").Set(ev, 2);
    ev[0] = extents[2];
    ev[1] = extents[3];
    vtkSMPropertyHelper(this->referenceProxy()->getProxy(),
      "TRange").Set(ev, 2);
    }

  if (numberOfRegistrationPoints == 2)
    {
    vtkSMPropertyHelper(this->referenceProxy()->getProxy(),
                        "TwoPointRegistration").Set(
                        this->Implementation->RegistrationPoints, 8);
    }
  else
    {
    vtkSMPropertyHelper(this->referenceProxy()->getProxy(),
                        "ThreePointRegistration").Set(
                        this->Implementation->RegistrationPoints, 12);
    }

  vtkSMPropertyHelper(this->referenceProxy()->getProxy(),
                      "GenerateCoordinates").Set(1);
  this->referenceProxy()->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(
    this->referenceProxy()->getProxy())->UpdatePipeline();
  if(this->getRepresentation())
    {
    this->getRepresentation()->getProxy()->UpdateVTKObjects();
    this->Implementation->TextureControls->currentView()->forceRender();
    }

  this->Implementation->isSettingTexture = false;
}

//-----------------------------------------------------------------------------
vtkSMProperty* pqPlanarTextureRegistrationPanel::getTextureProperty()
  {
  vtkSMProperty* textureProperty(0);
  pqDataRepresentation* thisRep = this->getRepresentation();
  if(thisRep)
    {
    vtkSMProperty* lti = thisRep->getProxy()->GetProperty("LargeTextureInput");
    textureProperty = lti ? lti : thisRep->getProxy()->GetProperty("Texture");
    }
  return textureProperty;
}
//-----------------------------------------------------------------------------
pqDataRepresentation* pqPlanarTextureRegistrationPanel::getRepresentation()
{
  pqRenderView* view = this->Implementation->TextureControls->currentView();
  pqPipelineFilter* filter = qobject_cast<pqPipelineFilter*>(
    this->referenceProxy());
  pqDataRepresentation* selfRep = filter->getRepresentation(view);
  if(!selfRep)
    {
    pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
    selfRep = builder->createDataRepresentation(filter->getOutputPort(0), view);
    }
  return selfRep;
}

//-----------------------------------------------------------------------------
void pqPlanarTextureRegistrationPanel::fetchTextureFiles(
  QStringList & textureslist)
{
  textureslist.clear();
  // Get all proxies under "textures" group and add them to the menu.
  vtkNew<vtkSMProxyIterator> proxyIter;
  proxyIter->SetModeToOneGroup();
  // Look for proxy that in the current active server/session
  proxyIter->SetSession(pqApplicationCore::instance()->getActiveServer()->session());
  for (proxyIter->Begin("textures"); !proxyIter->IsAtEnd(); proxyIter->Next())
    {
    QString filename = pqSMAdaptor::getElementProperty(
      proxyIter->GetProxy()->GetProperty("FileName")).toString();
    if (!textureslist.contains(filename))
      {
      textureslist.push_back(filename);
      }
    }
}

//-----------------------------------------------------------------------------
void pqPlanarTextureRegistrationPanel::updateTextureList()
{
  QStringList textureslist;
  this->fetchTextureFiles(textureslist);
  this->Implementation->TextureControls->updateTextureList(
    textureslist, this->Implementation->TextureFileName);
}
//-----------------------------------------------------------------------------
void pqPlanarTextureRegistrationPanel::proxyRegistered(const QString& group)
{
  if (group == "textures")
    {
    this->updateTextureList();
    }
}

//-----------------------------------------------------------------------------
void pqPlanarTextureRegistrationPanel::proxyUnRegistered(
  const QString& group, const QString&, vtkSMProxy* /*proxy*/)
{
  if (group == "textures")
    {
    pqTimer::singleShot(0, this, SLOT(updateTextureList()));
    }
}

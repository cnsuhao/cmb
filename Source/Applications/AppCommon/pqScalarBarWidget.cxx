#include "pqScalarBarWidget.h"

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqProxyWidget.h"
#include "pqSMAdaptor.h"
#include "pqScalarBarRepresentation.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqView.h"

#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSmartPointer.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMNewWidgetRepresentationProxy.h"

#include "vtkScalarBarRepresentation.h"
#include "vtkTuple.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>

class pqScalarBarWidget::cmbInternals
{
public:
  QPointer<pqScalarBarRepresentation> ScalarBarRep;
  QPointer<pqDataRepresentation> CurrentRep;
  vtkSmartPointer<vtkSMProxy> LookupTableProxy;
  vtkSmartPointer<vtkSMNewWidgetRepresentationProxy> ScalarBarProxy;
  cmbInternals(pqDataRepresentation* rep) :
    CurrentRep(rep)
    {
    }

  ~cmbInternals()
    {
    }
};
//-----------------------------------------------------------------------------
/// constructor
pqScalarBarWidget::pqScalarBarWidget(pqDataRepresentation* repr, QWidget* /*p*/)
  : Internals(new pqScalarBarWidget::cmbInternals(repr))
{
  this->init();
}

//-----------------------------------------------------------------------------
/// destructor
pqScalarBarWidget::~pqScalarBarWidget()
{
  delete this->Internals;
}
//-----------------------------------------------------------------------------
void pqScalarBarWidget::setIndexedColors( const QList<QColor>& colors )
{
  vtkSMProxy* proxy = this->Internals->LookupTableProxy;
  std::vector<vtkTuple<double, 3> > rgbColors;
  rgbColors.resize(colors.count());
  int cc=0;
  foreach (QColor color, colors)
    {
    rgbColors[cc].GetData()[0] = color.redF();
    rgbColors[cc].GetData()[1] = color.greenF();
    rgbColors[cc].GetData()[2] = color.blueF();
    cc++;
    }
  vtkSMPropertyHelper indexedColors(proxy->GetProperty("IndexedColors"));
  if(cc==0)
    {
    indexedColors.SetNumberOfElements(0);
    }
  else
    {
    indexedColors.Set(rgbColors[0].GetData(),
      static_cast<unsigned int>(rgbColors.size() * 3));
    }
  proxy->UpdateVTKObjects();

}
//-----------------------------------------------------------------------------
void pqScalarBarWidget::setAnnotations( const QList<QVariant>& annotations )
{
  vtkSMProxy* proxy = this->Internals->LookupTableProxy;

  pqSMAdaptor::setMultipleElementProperty(
    proxy->GetProperty( "Annotations" ), annotations );
  proxy->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void pqScalarBarWidget::setPositionToLeft()
{
  vtkSMProxy* proxy = this->Internals->ScalarBarProxy;
  vtkSMPropertyHelper smTextPos(proxy->GetProperty("TextPosition"));
  smTextPos.Set(0); // PrecedeScalarBar
  vtkSMPropertyHelper smVBorder(proxy->GetProperty("ShowVerticalBorder"));
  smVBorder.Set(2); // active
  vtkSMPropertyHelper smHBorder(proxy->GetProperty("ShowHorizontalBorder"));
  smHBorder.Set(2); // active

  double tlPos[2]={0.03, 0.25};
  vtkSMPropertyHelper smPos(proxy->GetProperty("Position"));
  smPos.Set(tlPos, 2);
  proxy->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void pqScalarBarWidget::setPositionToRight()
{
  vtkSMProxy* proxy = this->Internals->ScalarBarProxy;
  vtkSMPropertyHelper smVBorder(proxy->GetProperty("ShowVerticalBorder"));
  smVBorder.Set(2); // active
  vtkSMPropertyHelper smHBorder(proxy->GetProperty("ShowHorizontalBorder"));
  smHBorder.Set(2); // active

  double tlPos[2]={0.9, 0.25};
  vtkSMPropertyHelper smPos(proxy->GetProperty("Position"));
  smPos.Set(tlPos, 2);
  proxy->UpdateVTKObjects();
}
//-----------------------------------------------------------------------------
void pqScalarBarWidget::setTitle(const QString& title)
{
  vtkSMProxy* proxy = this->Internals->ScalarBarProxy;
  vtkSMPropertyHelper smTitle(proxy->GetProperty("Title"));
  smTitle.Set(title.toStdString().c_str());
  proxy->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void pqScalarBarWidget::setVisible(bool visible)
{
  this->Internals->ScalarBarRep->setVisible(visible);
  vtkSMPropertyHelper(this->Internals->ScalarBarProxy, "Enabled").Set(visible);
}

//-----------------------------------------------------------------------------
void pqScalarBarWidget::init()
{
  if(this->Internals->CurrentRep)
    {
    pqServer* server = this->Internals->CurrentRep->getServer();
    vtkSMSessionProxyManager* pxm = server->proxyManager();
    vtkSMProxy* lutProxy =
      pxm->NewProxy("lookup_tables", "PVLookupTable");
    pqSMAdaptor::setElementProperty(lutProxy->GetProperty("IndexedLookup"),1);
    lutProxy->UpdateVTKObjects();

    vtkSMProxy* scalarBarProxy =
      pxm->NewProxy("representations", "ScalarBarWidgetRepresentation");
    scalarBarProxy->SetPrototype(true);
    pqSMAdaptor::setProxyProperty(scalarBarProxy->GetProperty("LookupTable"),
      lutProxy);

    QString actual_regname = this->objectName();
    actual_regname.append(scalarBarProxy->GetXMLName());

    pxm->RegisterProxy("scalar_bars",
      actual_regname.toAscii().data(), scalarBarProxy);

    this->Internals->ScalarBarRep =
      pqApplicationCore::instance()->getServerManagerModel()->
      findItem<pqScalarBarRepresentation*>(scalarBarProxy);
    pqView *view = this->Internals->CurrentRep->getView();
    pqSMAdaptor::addProxyProperty(view->getProxy()->GetProperty("Representations"),
      scalarBarProxy);
    view->getProxy()->UpdateVTKObjects();

//    this->Internals->ScalarBarRep->setDefaultPropertyValues();
    pqSMAdaptor::setElementProperty(scalarBarProxy->GetProperty("TitleFontSize"), 10);
    pqSMAdaptor::setElementProperty(scalarBarProxy->GetProperty("LabelFontSize"), 8);


//    pqObjectBuilder* const builder = core->getObjectBuilder();
//    emit builder->scalarBarDisplayCreated(this->Internals->ScalarBarRep);
//    emit builder->proxyCreated(this->Internals->ScalarBarRep);

    this->Internals->LookupTableProxy.TakeReference(lutProxy);
    this->Internals->ScalarBarProxy.TakeReference(
      vtkSMNewWidgetRepresentationProxy::SafeDownCast(scalarBarProxy));
    this->setVisible(false);
    }
}

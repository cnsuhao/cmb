#include "KMLToolbar.h"

#include "ui_KMLAnimationSettings.h"

// QT includes.
#include <QApplication>
#include <QStyle>
#include <QDebug>

// ParaView includes.
#include "pqActiveObjects.h"
#include "pqAnimationScene.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqFileDialog.h"
#include "pqObjectBuilder.h"
#include "pqRenderView.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqUndoStack.h"

// VTK includes.
#include "vtkAnimationScene.h"
#include "vtkDateTime.h"
#include "vtkPVOptions.h"
#include "vtkSMProxy.h"
#include "vtkSMAnimationSceneKMLWriter.h"

// STL includes.
#include <sstream>

//-----------------------------------------------------------------------------
KMLToolbar::KMLToolbar(QObject* p) : QActionGroup(p)
{
  QIcon icon = qApp->style()->standardIcon(QStyle::SP_MessageBoxCritical);
  QAction* a = new QAction(icon, "Save Animation", this);
  a->setData("KML");
  this->addAction(a);
  QObject::connect(this, SIGNAL(triggered(QAction*)), this,
                   SLOT(onAction(QAction*)));
}

//-----------------------------------------------------------------------------
void KMLToolbar::onAction(QAction* vtkNotUsed(a))
{
  QDialog dialog;
  Ui_KMLAnimationSettingsDialog dialogUI;
  dialogUI.setupUi(&dialog);

  if(dialog.exec() == QDialog::Rejected)
    {
    return;
    }

  QString filters = "KML files (*.kml);;All files (*)";
  pqFileDialog fileDialog (pqActiveObjects::instance().activeServer(),
                           pqCoreUtilities::mainWidget(),
                           tr("Save KML Animation"),
                           QString(),
                           filters);

  fileDialog.setObjectName("FileSaveKMLAnimationDialog");
  fileDialog.setFileMode(pqFileDialog::AnyFile);

  QString fileName;
  if (fileDialog.exec() == QDialog::Accepted)
    {
       fileName = fileDialog.getSelectedFiles()[0];
    }
  else
    {
    return;
    }

  if(fileName.isEmpty())
    {
    return;
    }

  // Get the values.
  int dateTimeUnit = dialogUI.comboBox->currentIndex();

  if(dateTimeUnit > 0)
    {
    ++dateTimeUnit;
    }

  QDate date = dialogUI.dateTimeEdit->date();
  QTime time = dialogUI.dateTimeEdit->time();

  // Get the values.
  int altitudeMode = dialogUI.altitudeMode->currentIndex();

  bool  keepAspectRatio (false);
  if(dialogUI.keepAspectRatio->checkState() == Qt::Checked)
  {
    keepAspectRatio = true;
  }

  // Set validators.
  QIntValidator* intVdtr (new QIntValidator());
  dialogUI.magnificationFactor->setValidator(intVdtr);

  float legendScale = dialogUI.legendScale->text().toFloat();

  int   magFactor   = dialogUI.magnificationFactor->text().toInt();

  pqApplicationCore* core = pqApplicationCore::instance();
  pqServerManagerModel* sm = core->getServerManagerModel();

 QString anime ("pqAnimationScene");
 QList<pqAnimationScene*> scenes =
     sm->findItems<pqAnimationScene*>(core->getActiveServer());

 if(!scenes.size())
   {
   qCritical() << "Error: No scene found.";
   return;
   }

 vtkSMProxy* proxy = scenes.at(0)->getProxy();

 if(!proxy)
   {
   qCritical() <<"Error: No Scene Proxy found.";
   return;
   }

 // Lets turn off the center and coordinate axis first.
 pqRenderView* renView =
  qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView());
if(renView)
  {
  vtkSmartPointer<vtkDateTime> dateTime (vtkSmartPointer<vtkDateTime>::New());
  dateTime->SetYear(date.year());
  dateTime->SetMonth(date.month());
  dateTime->SetDay(date.day());
  dateTime->SetHour(time.hour());
  dateTime->SetMinute(time.minute());
  dateTime->SetSecond(time.second());

  bool ctrAxsSt = renView->getCenterAxesVisibility();
  bool ortAxsSt = renView->getOrientationAxesVisibility();
  renView->setCenterAxesVisibility(false);
  renView->setOrientationAxesVisibility(false);

  vtkSmartPointer<vtkSMAnimationSceneKMLWriter> writer
  (vtkSmartPointer<vtkSMAnimationSceneKMLWriter>::New());
  writer->SetAnimationScene(proxy);
  writer->SetFileName(fileName.toStdString().c_str());
  writer->GetKMLExporter()->SetDeltaDateTimeUnit(dateTimeUnit);
  writer->GetKMLExporter()->SetStartDateTime(dateTime);
  writer->GetKMLExporter()->SetLegendScale(legendScale);
  writer->GetKMLExporter()->SetKeepLegendAspectRatio(keepAspectRatio);
  writer->GetKMLExporter()->SetAltitudeMode(altitudeMode);
  writer->GetKMLExporter()->SetRenderScene(
    dialogUI.renderSceneAsImageCheckBox->isChecked());
  writer->GetKMLExporter()->SetRenderPolyDataAsImage(
        dialogUI.renderPolydataAsImageCheckBox->isChecked());
  writer->GetKMLExporter()->SetCellCountThreshold(dialogUI.cellCountThreshold->value());
  writer->SetMagnification(magFactor);
  writer->Save();

  renView->setCenterAxesVisibility(ctrAxsSt);
  renView->setOrientationAxesVisibility(ortAxsSt);
 }
else
  {
  qCritical() << "Error: Unable to get render view.";
  }
}

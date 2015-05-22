/*=========================================================================

 Program:   Visualization Toolkit
 Module:    $RCSfile: pqSMTKMeshPanel.h,v $

 =========================================================================*/
// .NAME pqSMTKMeshPanel - Custom object panel for vtkSMTKModelReader
// .SECTION Description
#include <QDockWidget>
#include <QPointer>

class pqCMBModelManager;
class qtCMBMeshingMonitor;
class qtRemusMesherSelector;

namespace smtk { namespace  attribute { class qtUIManager; } }

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/Model.h"

#ifndef Q_MOC_RUN
# include <remus/client/Client.h>
#endif

class pqSMTKMeshPanel : public QDockWidget
{
  Q_OBJECT
public:
  pqSMTKMeshPanel(QPointer<pqCMBModelManager> mmgr,
                  QPointer<qtCMBMeshingMonitor> monitor,
                  QWidget* p);
  ~pqSMTKMeshPanel();

  QPointer<pqCMBModelManager> modelManager();


  void updateModel( QPointer<pqCMBModelManager> mmgr,
                     QPointer<qtCMBMeshingMonitor> monitor);

protected slots:
  void displayRequirements(const std::vector<smtk::model::Model>& models,
                           const QString & workerName,
                           const remus::proto::JobRequirements& reqs);

  void clearActiveModel();

  bool submitMeshJob();

signals:
  void meshingPossible( bool );

private:
  QPointer<pqCMBModelManager> ModelManager;
  QPointer<qtCMBMeshingMonitor> MeshMonitor;
  QPointer<qtRemusMesherSelector> MeshSelector;

  QPointer<QWidget> RequirementsWidget;
  QPointer<QWidget> SubmitterWidget;

  smtk::attribute::SystemPtr AttSystem;
  smtk::shared_ptr<smtk::attribute::qtUIManager> AttUIManager;

  std::vector<smtk::model::Model> ActiveModels;
  remus::proto::JobRequirements ActiveRequirements;
};

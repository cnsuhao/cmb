
// .NAME SimBuilderMeshManager -
// .SECTION Description

#ifndef __SimBuilderMeshManager_h
#define __SimBuilderMeshManager_h

#include <QObject>
#include <QPointer>
#include "cmbSystemConfig.h"

class vtkCMBMeshClient;
class qtCMBPanelWidget;
class pqCMBModel;
class qtCMBModelTree;
class qtCMBTree;
class QTreeWidgetItem;
class vtkCollection;
class pqOutputPort;

class SimBuilderMeshManager : public QObject
{
  Q_OBJECT

public:
  SimBuilderMeshManager();
  virtual ~SimBuilderMeshManager();

  void setUIPanel(qtCMBPanelWidget* uiPanel);
  void setCMBModel(pqCMBModel* cmbModel);
  void setModelTree(qtCMBModelTree* modelTree);
  void clearMesh();
  void getModelFaceMeshes(QList<pqOutputPort*>& meshes);
  bool analysisMeshIsCurrent();
  bool useAsAnalysisMesh();
  bool hasMesh();

signals :

public slots:
  void onStartMeshing();
  void onModelSelectionChanged(qtCMBTree*);
  void onMeshItemChanged(QTreeWidgetItem* item, int col);
  void saveAsAnalysisMesh();
  void exportAnalysisMesh();

protected:
  void Initialize();
  void setupUIConnection();
  void clearUIConnection();
  void updateLocalEdgeMesh(vtkCollection* selEdges,
    vtkCollection* selFaces=NULL);
  void saveAnalysisMesh(QList<pqOutputPort*>& meshes, bool isAnalysisMesh);

private:
  bool AnalysisMeshIsCurrent;
  QPointer<pqCMBModel> CMBModel;
  QPointer<qtCMBPanelWidget> UIPanel;
  QPointer<qtCMBModelTree> ModelTree;

  vtkCMBMeshClient* MeshClient;
};

#endif

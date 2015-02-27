
// .NAME SimBuilderCore -
// .SECTION Description

#ifndef __SimBuilderCore_h
#define __SimBuilderCore_h

#include <QObject>
#include <QPointer>
#include <string>
#include "cmbSystemConfig.h"

class pqCMBSceneTree;
class SimBuilderUIPanel;
class pqServer;
class pqPipelineSource;
class pqCMBModel;
class vtkModelEntity;
class vtkDiscreteModel;
class vtkSMProxy;
class pqRenderView;
class SimBuilderMeshManager;
class smtkUIManager;
class SimBuilderCustomExportDialog;

class SimBuilderCore : public QObject
{
  Q_OBJECT

public:
  SimBuilderCore(pqServer*, pqRenderView* view);
  virtual ~SimBuilderCore();

  bool isSimModelLoaded(){return this->IsSimModelLoaded;}
  void clearSimulationModel();
  void clearCMBModel();
  void updateCMBModelWithScenario(bool emitSignal=true);

  void setServer(pqServer* server);
  void setRenderView(pqRenderView* view);

  // Description:
  // Load/Save Simulation.
  // Return 1 on success, 0 otherwise.
  int LoadSimulation(bool templateOnly = false, bool isScenario = false);
  int LoadSimulation(const char *filename);
  int LoadSimulation(pqPipelineSource* reader, pqCMBSceneTree* sceneTree);
  int SaveSimulation(const char *filename, bool writeScenario=false);
  int SaveSimulation(bool writeScenario=false);
  int LoadSimulationTemplate()
    {return this->LoadSimulation(true);}
  int LoadSimulationTemplate(const char *filename)
    {return this->LoadSimulation(filename, true);}
  int SaveSimulationTemplate(const char *filename)
    {return this->SaveSimulation(filename, true);}
  int SaveSimulationTemplate()
    {return this->SaveSimulation(true);}

  // Load CMB resource file, which may contain multiple resources
  int LoadResources(const char *filename);
  int LoadResources(pqPipelineSource* reader, pqCMBSceneTree* sceneTree);
  bool setDefaultExportTemplate();

  void ExportSimFile();

  smtkUIManager* attributeUIManager();
  SimBuilderUIPanel* GetUIPanel();

  void setCMBModel(pqCMBModel* cmbModel);
  pqCMBModel* getCMBModel();
  bool isTemplateOnly(){return this->LoadTemplateOnly;}
  bool isLoadingScenario(){return this->LoadingScenario;}
  bool hasScenarioModelEntities(){return this->ScenarioEntitiesCreated;}

  SimBuilderMeshManager* getMeshManager();

  // update model related Qt attribute panels
  void updateCMBModelItems(vtkDiscreteModel* model,
    vtkSMProxy* modelwrapper);

  // Description:
  // Initialize the SimBuilder core
  virtual void Initialize();
  void updateSimBuilder(pqCMBSceneTree* sceneTree);

signals :
  void newSimFileLoaded(const char* filename);

public slots:
  void onModelEntityNameChanged(vtkModelEntity*);
  //void onModelBCGroupChanged();
  void updateSimulationModel();

protected:

  vtkDiscreteModel* GetEntityModel(vtkModelEntity* refEntity);

private:

  // Description:
  // Manages all the UI
  QPointer<SimBuilderUIPanel> UIPanel;
  pqServer* ActiveServer;
  pqRenderView* RenderView;
  QPointer<pqCMBModel> CMBModel;
  bool IsSimModelLoaded;
  SimBuilderCustomExportDialog *ExportDialog;

  std::string CurrentSimFile;
  std::string CurrentTemplateFile;
  std::string SimFileVersion;

  bool LoadTemplateOnly;
  bool LoadingScenario;
  bool ScenarioEntitiesCreated;

  SimBuilderMeshManager* MeshManager;
  QPointer<smtkUIManager> m_attUIManager;

};

#endif




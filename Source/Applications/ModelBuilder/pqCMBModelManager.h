
// .NAME pqCMBModelManager -
// .SECTION Description

#ifndef __qtModelManager_h
#define __qtModelManager_h

#include <QObject>
#include <QStringList>

#include "vtkSmartPointer.h"
#include <QPointer>
#include <set>
#include "cmbSystemConfig.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/model/StringData.h"

class vtkPVSMTKModelInformation;
class vtkSMModelManagerProxy;
class vtkSMProxy;
class pqDataRepresentation;
class pqPipelineSource;
class pqRenderView;
class pqServer;

//The object to keep smtk model related info:
// pqSource, pvModelInfo, smSelectionSource
struct cmbSMTKModelInfo
  {
  public:
    cmbSMTKModelInfo(){}
    cmbSMTKModelInfo(const cmbSMTKModelInfo& other);
    void init(pqPipelineSource*, pqDataRepresentation*,
      const std::string& filename, smtk::model::ManagerPtr);
    void updateBlockInfo(smtk::model::ManagerPtr mgr);

    vtkSmartPointer<vtkSMProxy> SelectionSource;
    vtkSmartPointer<vtkPVSMTKModelInformation> Info;
    QPointer<pqPipelineSource> Source;
    QPointer<pqDataRepresentation> Representation;
    std::string FileName;
    smtk::model::SessionPtr Session;
  };

class pqCMBModelManager : public QObject
{
  Q_OBJECT

public:
  pqCMBModelManager(pqServer*);
  virtual ~pqCMBModelManager();
  vtkSMModelManagerProxy* managerProxy();
  smtk::model::StringData fileModelSessions(const std::string& filename);
  std::set<std::string> supportedFileTypes(
    const std::string& bridgeName = std::string());

  cmbSMTKModelInfo* modelInfo(const smtk::model::EntityRef& entity);
  cmbSMTKModelInfo* modelInfo(pqDataRepresentation* rep);
  QList<cmbSMTKModelInfo*> selectedModels();
  int numberOfModels();

  QList<pqDataRepresentation*> modelRepresentations();
  pqDataRepresentation* activeModelRepresentation();
  bool DetermineFileReader(
    const std::string& filename,
    std::string& bridgeType,
    std::string& engineType,
    const smtk::model::StringData& bridgeTypes);
  pqServer* server();

signals:
  void currentModelCleared();
  void newSessionLoaded(const QStringList& bridgeNames);
  void newFileTypesAdded(const QStringList& fileTypes);
  void operationFinished(const smtk::model::OperatorResult&, bool hasNewModels);

public slots:
  void clear();
  bool startSession(const std::string& bridgeName);
  void clearModelSelections();
  bool loadModel(const std::string& filename,
    pqRenderView* view);
  bool startOperation(const smtk::model::OperatorPtr&);
  bool handleOperationResult(
    const smtk::model::OperatorResult& result,
    const smtk::common::UUID& bridgeSessionId,
    bool &hadNewModels);

protected slots:
  void onPluginLoaded();

protected:
  void initialize();
  void initOperator(smtk::model::OperatorPtr brOp);

private:
  class qInternal;
  qInternal* Internal;

};

#endif

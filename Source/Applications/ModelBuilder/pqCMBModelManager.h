
// .NAME pqCMBModelManager -
// .SECTION Description

#ifndef __qtModelManager_h
#define __qtModelManager_h

#include <QObject>
#include <QStringList>
#include <QPointer>
#include <QColor>

#include "vtkSmartPointer.h"
#include <set>
#include "cmbSystemConfig.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/model/StringData.h"

namespace smtk
{
  namespace attribute
  {
    class qtMeshSelectionItem;
  }
}

class vtkPVSMTKModelInformation;
class vtkSMModelManagerProxy;
class vtkSMProxy;
class pqDataRepresentation;
class pqOutputPort;
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

    vtkSmartPointer<vtkSMProxy> BlockSelectionSource;
    vtkSmartPointer<vtkSMProxy> CompositeDataIdSelectionSource;
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

  void supportedColorByModes(QStringList& types);

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
  void operationFinished(const smtk::model::OperatorResult&,
    const smtk::model::SessionRef& sref, bool hasNewModels);
  void requestMeshSelectionUpdate(
    const smtk::attribute::MeshSelectionItemPtr&, cmbSMTKModelInfo*);

public slots:
  void clear();
  bool startNewSession(const std::string& bridgeName);
  void clearModelSelections();
  smtk::model::OperatorPtr createFileOperator(const std::string& filename);
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

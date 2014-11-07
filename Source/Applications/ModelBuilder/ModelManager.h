
// .NAME ModelManager -
// .SECTION Description

#ifndef __ModelManager_h
#define __ModelManager_h

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
    void init(pqPipelineSource*, pqDataRepresentation*, const std::string& filename);

    vtkSmartPointer<vtkSMProxy> SelectionSource;
    vtkSmartPointer<vtkPVSMTKModelInformation> Info;
    QPointer<pqPipelineSource> Source;
    QPointer<pqDataRepresentation> Representation;
    std::string FileName;
  };

class ModelManager : public QObject
{
  Q_OBJECT

public:
  ModelManager(pqServer*);
  virtual ~ModelManager();
  vtkSMModelManagerProxy* managerProxy();
  smtk::model::StringData fileModelBridges(const std::string& filename);
  std::set<std::string> supportedFileTypes(
    const std::string& bridgeName = std::string());

  cmbSMTKModelInfo* modelInfo(const smtk::common::UUID& uid);
  QList<cmbSMTKModelInfo*> selectedModels();
  int numberOfModels();

  pqDataRepresentation* activeModelRepresentation();
  bool DetermineFileReader(
    const std::string& filename, 
    std::string& bridgeType,
    std::string& engineType,
    const smtk::model::StringData& bridgeTypes);

signals:
  void currentModelCleared();
  void newBridgeLoaded(const QStringList& fileTypes);

public slots:
  void clear();
  bool loadModel(const std::string& filename,
    pqRenderView* view);

protected slots:
  void onPluginLoaded();

protected:
  void initialize();

private:
  class qInternal;
  qInternal* Internal;

};

#endif

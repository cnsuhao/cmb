
// .NAME ModelManager -
// .SECTION Description

#ifndef __ModelManager_h
#define __ModelManager_h

#include <QObject>
#include <QStringList>

#include "smtk/PublicPointerDefs.h"
#include <vector>
#include "cmbSystemConfig.h"

class vtkSMModelManagerProxy;
class pqDataRepresentation;
class pqPipelineSource;
class pqRenderView;
class pqServer;

class ModelManager : public QObject
{
  Q_OBJECT

public:
  ModelManager(pqServer*);
  virtual ~ModelManager();
  vtkSMModelManagerProxy* managerProxy();
  std::string fileModelBridge(const std::string& filename);
  std::vector<std::string> supportedFileTypes(
    const std::string& bridgeName = std::string());
//  std::set<pqPipelineSource*> modelSources(const smtk::common::UUID&);
  const std::string& currentFile() const;

  pqPipelineSource* activeModelSource();
  pqDataRepresentation* activeModelRepresentation();

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

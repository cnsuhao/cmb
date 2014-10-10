
// .NAME ModelManager -
// .SECTION Description

#ifndef __ModelManager_h
#define __ModelManager_h

#include <QObject>
#include <QStringList>
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
  std::string fileSupportBridge(const std::string& filename);
  std::vector<std::string> supportedFileTypes(
    const std::string& bridgeName = std::string());
  pqPipelineSource* modelSource();
  pqDataRepresentation* modelRepresentation();
  const std::string& currentFile() const
    { return this->m_CurrentFile; }

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

  vtkSMModelManagerProxy* m_ManagerProxy;
  pqPipelineSource* m_modelSource;
  pqDataRepresentation* m_modelRepresentation;
  pqServer* m_Server;
  std::string m_CurrentFile;

};

#endif

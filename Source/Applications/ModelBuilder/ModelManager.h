
// .NAME ModelManager -
// .SECTION Description

#ifndef __ModelManager_h
#define __ModelManager_h

#include <QObject>
#include <map>
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
  std::vector<std::string> supportedFileTypes();
  pqPipelineSource* modelSource();
  pqDataRepresentation* modelRepresentation();
  const std::string& currentFile() const
    { return this->m_CurrentFile; }

signals:
//  void currentModelLoaded();
  void currentModelCleared();

public slots:
  void clear();
  bool loadModel(const std::string& filename,
    pqRenderView* view);

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

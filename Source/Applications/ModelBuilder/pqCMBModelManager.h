//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBModelManager -
// .SECTION Description

#ifndef __qtModelManager_h
#define __qtModelManager_h

#include "vtkSmartPointer.h"
#include "vtkNew.h"
#include "cmbSystemConfig.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/model/StringData.h"

#include <QObject>
#include <QStringList>
#include <QPointer>
#include <QColor>
#include <set>
#include <vector>

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
class cmbSMTKModelInfo
  {
  public:
    cmbSMTKModelInfo(){
      this->ShowMesh = false;
    }
    cmbSMTKModelInfo(const cmbSMTKModelInfo& other);
    void init(pqPipelineSource* modelSource, pqPipelineSource* repSource,
      pqDataRepresentation*, const std::string& filename, smtk::model::ManagerPtr);
    void updateBlockInfo(smtk::model::ManagerPtr mgr);
    bool hasAnalysisMesh() const;

    vtkSmartPointer<vtkSMProxy> BlockSelectionSource;
    vtkSmartPointer<vtkSMProxy> CompositeDataIdSelectionSource;
    vtkSmartPointer<vtkSMProxy> EntityLUT;
    vtkSmartPointer<vtkSMProxy> GroupLUT;
    vtkSmartPointer<vtkSMProxy> VolumeLUT;
    vtkSmartPointer<vtkSMProxy> AttributeLUT;
    QString ColorMode;
    bool ShowMesh;

    vtkSmartPointer<vtkPVSMTKModelInformation> Info;
    QPointer<pqPipelineSource> ModelSource;    
    QPointer<pqPipelineSource> RepSource;
    QPointer<pqDataRepresentation> Representation;
    std::string FileName;
    smtk::model::SessionPtr Session;
    std::vector<std::string> ent_annotations;
    std::vector<std::string> vol_annotations;
    std::vector<std::string> grp_annotations;

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
  void updateEntityColorTable(pqDataRepresentation* rep,
    const QMap<smtk::model::EntityRef, QColor >& colorEntities,
    const QString& colorByMode);
  void updateAttributeColorTable(
    pqDataRepresentation* rep,
    vtkSMProxy* lutProxy,
    const QMap<std::string, QColor >& colorAtts,
    const std::vector<std::string>& annList);
  void colorRepresentationByEntity(
    pqDataRepresentation* rep, const QString& entityMode);
  void colorRepresentationByAttribute(
    pqDataRepresentation* rep, smtk::attribute::SystemPtr attSys,
    const QString& attDef, const QString& attItem);
  void syncDisplayColorTable(pqDataRepresentation* rep);

  cmbSMTKModelInfo* modelInfo(const smtk::model::EntityRef& entity);
  cmbSMTKModelInfo* modelInfo(pqDataRepresentation* rep);
  QList<cmbSMTKModelInfo*> selectedModels();
  QList<cmbSMTKModelInfo*> allModels();

  int numberOfModels();

  QList<pqDataRepresentation*> modelRepresentations();
  pqDataRepresentation* activeModelRepresentation();
  bool DetermineFileReader(
    const std::string& filename,
    std::string& bridgeType,
    std::string& engineType,
    const smtk::model::StringData& bridgeTypes);
  pqServer* server();
  void updateModelRepresentation(const smtk::model::EntityRef& model);
  void updateModelRepresentation(cmbSMTKModelInfo* minfo);

signals:
  void currentModelCleared();
  void newSessionLoaded(const QStringList& bridgeNames);
  void newFileTypesAdded(const QStringList& fileTypes);
  void operationFinished(const smtk::model::OperatorResult&,
    const smtk::model::SessionRef& sref, bool hasNewModels, bool bGeometryChanged);
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
    bool &hadNewModels, bool& bGeometryChanged);

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

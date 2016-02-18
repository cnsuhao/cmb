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
#include "smtk/mesh/MeshSet.h"

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
class pqSMTKModelInfo;
class pqSMTKMeshInfo;

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
  void colorRepresentationByAttribute(
    pqDataRepresentation* rep, smtk::attribute::SystemPtr attSys,
    const QString& attDef, const QString& attItem);

  pqSMTKModelInfo* modelInfo(const smtk::model::EntityRef& entity);
  pqSMTKModelInfo* modelInfo(pqDataRepresentation* rep);
  QList<pqSMTKModelInfo*> selectedModels() const;
  QList<pqSMTKModelInfo*> allModels() const;

  int numberOfModels();

  QList<pqDataRepresentation*> modelRepresentations() const;
  pqDataRepresentation* activeRepresentation() const;
  bool DetermineFileReader(
    const std::string& filename,
    std::string& bridgeType,
    std::string& engineType,
    const smtk::model::StringData& bridgeTypes);
  pqServer* server();
  void updateModelRepresentation(const smtk::model::EntityRef& model);
  void updateModelRepresentation(pqSMTKModelInfo* minfo);
  void updateModelMeshRepresentations(const smtk::model::Model& model);

  pqSMTKMeshInfo* meshInfo(const smtk::mesh::MeshSet& mesh);
  pqSMTKMeshInfo* meshInfo(pqDataRepresentation* rep);
  QList<pqSMTKMeshInfo*> selectedMeshes() const;
  QList<pqDataRepresentation*> meshRepresentations() const;
  QList<pqSMTKMeshInfo*> allMeshes() const;

signals:
  void currentModelCleared();
  void newSessionLoaded(const QStringList& bridgeNames);
  void newFileTypesAdded(const QStringList& fileTypes);
  void operationFinished(const smtk::model::OperatorResult&,
    const smtk::model::SessionRef& sref,
    bool hasNewModels, bool bModelGeometryChanged, bool hasNewMeshes);
  void requestMeshSelectionUpdate(
    const smtk::attribute::MeshSelectionItemPtr&, pqSMTKModelInfo*);

public slots:
  void clear();
  bool startNewSession(const std::string& bridgeName);
  void clearModelSelections();
  void clearMeshSelections();
  smtk::model::OperatorPtr createFileOperator(const std::string& filename);
  bool startOperation(const smtk::model::OperatorPtr&);
  bool handleOperationResult(
    const smtk::model::OperatorResult& result,
    const smtk::common::UUID& bridgeSessionId,
    bool &hadNewModels, bool& bModelGeometryChanged,
    bool &hasNewMeshes);

protected slots:
  void onPluginLoaded();

protected:
  void initialize();
  void initOperator(smtk::model::OperatorPtr brOp);
  std::string getNativeModelFile(const std::string& filename) const;

private:
  class qInternal;
  qInternal* Internal;

};

#endif

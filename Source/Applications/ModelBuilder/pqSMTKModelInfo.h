//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqSMTKModelInfo -
// .SECTION Description

#ifndef __pqSMTKModelInfo_h
#define __pqSMTKModelInfo_h

#include <QObject>

#include "cmbSystemConfig.h"
#include "smtk/PublicPointerDefs.h"
#include "vtkSmartPointer.h"

#include "pqSMTKMeshInfo.h"
#include "smtk/common/UUID.h"
#include <QPointer>
#include <map>
#include <vector>

class vtkPVSMTKModelInformation;
class vtkSMModelManagerProxy;
class vtkSMProxy;
class pqDataRepresentation;
class pqPipelineSource;

class smtkAuxGeoInfo
{
public:
  smtkAuxGeoInfo() {}
  QPointer<pqPipelineSource> AuxGeoSource;
  QPointer<pqPipelineSource> ImageSource;
  QPointer<pqDataRepresentation> Representation;
  std::string URL;
  smtk::common::UUIDs RelatedAuxes;
};

//The object to keep smtk model related info:
// pqSource, pvModelInfo, smSelectionSource
class pqSMTKModelInfo : public QObject
{
  Q_OBJECT

public:
  pqSMTKModelInfo() { this->ShowMesh = false; }
  pqSMTKModelInfo(const pqSMTKModelInfo& other);
  void init(pqPipelineSource* modelSource, pqPipelineSource* repSource, pqDataRepresentation*,
    const std::string& filename, smtk::model::ManagerPtr);
  void updateBlockInfo(smtk::model::ManagerPtr mgr);
  bool hasAnalysisMesh() const;
  void setGlyphRep(pqDataRepresentation* rep);

  std::size_t numberOfTessellatedEntities() const;

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
  QPointer<pqDataRepresentation> GlyphRepresentation;
  std::string FileName;
  smtk::common::UUID SessionId;
  std::vector<std::string> ent_annotations;
  std::vector<std::string> vol_annotations;
  std::vector<std::string> grp_annotations;

  // Information for related meshes <collectionId, meshinfo>
  std::map<smtk::common::UUID, pqSMTKMeshInfo> MeshInfos;
};

#endif

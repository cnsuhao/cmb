//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqSMTKMeshInfo -
// .SECTION Description

#ifndef __pqSMTKMeshInfo_h
#define __pqSMTKMeshInfo_h

#include <QObject>

#include "cmbSystemConfig.h"
#include "smtk/PublicPointerDefs.h"
#include "vtkSmartPointer.h"

#include <QPointer>
#include <vector>

class vtkPVSMTKMeshInformation;
class vtkSMModelManagerProxy;
class vtkSMProxy;
class pqDataRepresentation;
class pqPipelineSource;
class pqSMTKModelInfo;
class vtkSMPropertyLink;

//The object to keep smtk mesh related info:
// pqSource, pvMeshInfo, smSelectionSource
class pqSMTKMeshInfo : public QObject
{
  Q_OBJECT

public:
  pqSMTKMeshInfo() {}
  ~pqSMTKMeshInfo() override;
  pqSMTKMeshInfo(const pqSMTKMeshInfo& other);
  void init(pqPipelineSource* meshSource, pqPipelineSource* repSource, pqDataRepresentation*,
    pqDataRepresentation*, const std::string& filename, smtk::model::ManagerPtr,
    pqSMTKModelInfo* modinfo);
  void updateBlockInfo(smtk::model::ManagerPtr mgr);
  void clearLinks();

  vtkSmartPointer<vtkSMProxy> BlockSelectionSource;
  vtkSmartPointer<vtkSMProxy> CompositeDataIdSelectionSource;

  vtkSmartPointer<vtkPVSMTKMeshInformation> Info;
  QPointer<pqPipelineSource> MeshSource;
  QPointer<pqPipelineSource> RepSource;
  QPointer<pqDataRepresentation> Representation;
  QPointer<pqDataRepresentation> PointsRepresentation;
  std::string FileName;
  QString ColorMode;
  QPointer<pqSMTKModelInfo> ModelInfo;

  vtkSmartPointer<vtkSMPropertyLink> PositionLink;
  vtkSmartPointer<vtkSMPropertyLink> OrientationLink;
  vtkSmartPointer<vtkSMPropertyLink> ScaleLink;
  vtkSmartPointer<vtkSMPropertyLink> OriginLink;
};

#endif

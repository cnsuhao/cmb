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

#include "smtk/PublicPointerDefs.h"
#include "vtkSmartPointer.h"
#include "cmbSystemConfig.h"

#include <QPointer>
#include <vector>
#include <map>
#include "smtk/common/UUID.h"
#include "pqSMTKMeshInfo.h"

class vtkPVSMTKModelInformation;
class vtkSMModelManagerProxy;
class vtkSMProxy;
class pqDataRepresentation;
class pqPipelineSource;

//The object to keep smtk model related info:
// pqSource, pvModelInfo, smSelectionSource
class pqSMTKModelInfo: public QObject
{
  Q_OBJECT

  public:
    pqSMTKModelInfo(){
      this->ShowMesh = false;
    }
    pqSMTKModelInfo(const pqSMTKModelInfo& other);
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
    smtk::common::UUID SessionId;
    std::vector<std::string> ent_annotations;
    std::vector<std::string> vol_annotations;
    std::vector<std::string> grp_annotations;

    // Information for related meshes <collectionId, meshinfo>
    std::map<smtk::common::UUID, pqSMTKMeshInfo> MeshInfos;
};

#endif

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

#include "smtk/PublicPointerDefs.h"
#include "vtkSmartPointer.h"
#include "cmbSystemConfig.h"

#include <QPointer>
#include <vector>

class vtkPVSMTKMeshInformation;
class vtkSMModelManagerProxy;
class vtkSMProxy;
class pqDataRepresentation;
class pqPipelineSource;

//The object to keep smtk mesh related info:
// pqSource, pvMeshInfo, smSelectionSource
class pqSMTKMeshInfo: public QObject
{
  Q_OBJECT

  public:
    pqSMTKMeshInfo(){
    }
    pqSMTKMeshInfo(const pqSMTKMeshInfo& other);
    void init(pqPipelineSource* meshSource, pqPipelineSource* repSource,
      pqDataRepresentation*, const std::string& filename, smtk::model::ManagerPtr);
    void updateBlockInfo(smtk::model::ManagerPtr mgr);

    vtkSmartPointer<vtkSMProxy> BlockSelectionSource;
    vtkSmartPointer<vtkSMProxy> CompositeDataIdSelectionSource;

    vtkSmartPointer<vtkPVSMTKMeshInformation> Info;
    QPointer<pqPipelineSource> MeshSource;    
    QPointer<pqPipelineSource> RepSource;
    QPointer<pqDataRepresentation> Representation;
    std::string FileName;
};

#endif

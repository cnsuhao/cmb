//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __pqCMBContextMenuHelper_h
#define __pqCMBContextMenuHelper_h

#include "cmbSystemConfig.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/model/EntityRef.h"
#include "vtkType.h"
#include <QColor>
#include <QMap>

class pqSMTKModelInfo;
class pqSMTKMeshInfo;
class pqCMBModelManager;
class pqDataRepresentation;

class pqCMBContextMenuHelper
{
public:
  pqCMBContextMenuHelper() {}
  static bool getBlockIndex(const smtk::model::EntityRef& eref, vtkIdType& flatIndex);
  /// Fetch children for volum and group entities.
  static void accumulateChildGeometricEntities(
    QSet<vtkIdType>& blockIds, const smtk::model::EntityRef& toplevel);

  // Use valid color specified by user, the rest will be colored by default
  // color in settings
  // Call entref.hasColor() beforehand to stop querying default color
  static bool getValidEntityColor(QColor& color, const smtk::model::EntityRef& entref);

  // only use valid color, the rest will be colored
  // randomly with CTF
  static bool getValidMeshColor(QColor& color, const smtk::mesh::MeshSet& mesh);

  // only use valid color, the rest will be colored
  // randomly with CTF
  static bool validMeshColorMode(const QString& colorMode, const smtk::mesh::MeshSet& mesh);

  // return total number of blocks selected
  static int getSelectedRepBlocks(const QList<pqSMTKModelInfo*>& selModels,
    const QList<pqSMTKMeshInfo*>& selMeshes, QMap<pqSMTKModelInfo*, QList<vtkIdType> >& modelresult,
    QMap<pqSMTKMeshInfo*, QList<vtkIdType> >& meshresult);

  static bool hasSessionOp(const smtk::model::SessionPtr& brSession, const std::string& opname);

  static bool startGroupOp(pqCMBModelManager* modMgr, pqSMTKModelInfo* minfo,
    const std::string& optype, const QList<vtkIdType>& addblocks,
    const smtk::model::Group& modifyGroup);

  static void setRepresentationType(pqDataRepresentation* repr, const QString& repType);

  static void getAllEntityIds(
    pqSMTKModelInfo* minfo, smtk::model::ManagerPtr modelMan, smtk::common::UUIDs& entids);

  static void getAllMeshSets(
    pqSMTKMeshInfo* minfo, smtk::mesh::ManagerPtr meshMgr, smtk::mesh::MeshSets& meshes);

  static void updateColorForAuxiliary(pqDataRepresentation* rep, const QColor& color);
  static void updateVisibilityForAuxiliary(pqDataRepresentation* rep, bool visible);

private:
  static const std::string s_internal_groupOpName;
};

#endif // !__pqCMBContextMenuHelper_h

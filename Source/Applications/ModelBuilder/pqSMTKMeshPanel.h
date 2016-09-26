//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqSMTKMeshPanel - Custom object panel for vtkSMTKModelReader
// .SECTION Description
#include <QDockWidget>
#include <QPointer>

class pqCMBModelManager;
class qtCMBMeshingMonitor;
class qtRemusMesherSelector;

namespace smtk { namespace  extension { class qtUIManager; } }

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/Model.h"

#ifndef Q_MOC_RUN
# include <remus/client/Client.h>
#endif

class pqSMTKMeshPanel : public QDockWidget
{
  Q_OBJECT
public:
  pqSMTKMeshPanel(QPointer<pqCMBModelManager> mmgr,
                  QPointer<qtCMBMeshingMonitor> monitor,
                  QWidget* p);
  ~pqSMTKMeshPanel() override;

  QPointer<pqCMBModelManager> modelManager();


  void updateModel( QPointer<pqCMBModelManager> mmgr,
                    QPointer<qtCMBMeshingMonitor> monitor);

protected slots:
  void displayRequirements(const smtk::model::Model& model,
                           const QString & workerName,
                           const remus::proto::JobRequirements& reqs);

  void clearActiveModel();

  void clearActiveMesh();

  bool submitMeshJob();

signals:
  void meshingPossible( bool );
  void entitiesSelected(const smtk::common::UUIDs&);

private:
  void cacheAtts(const std::string& atts);
  const std::string& fetchCachedAtts() const;
  bool hasCachedAtts(const remus::proto::JobRequirements& reqs) const;

  QPointer<pqCMBModelManager> ModelManager;
  QPointer<qtCMBMeshingMonitor> MeshMonitor;
  QPointer<qtRemusMesherSelector> MeshSelector;

  QPointer<QWidget> RequirementsWidget;
  QPointer<QWidget> SubmitterWidget;

  smtk::attribute::SystemPtr AttSystem;
  smtk::shared_ptr<smtk::extension::qtUIManager> AttUIManager;

  smtk::model::Model ActiveModel;
  remus::proto::JobRequirements ActiveRequirements;

  struct AttCacheKey
  {
    smtk::model::Model m_model;
    std::string m_workerName;

    bool operator<( const AttCacheKey& other ) const
    { return m_model < other.m_model ||
             (m_model == other.m_model && m_workerName < other.m_workerName);
    }
  };
  std::map< AttCacheKey, std::string > CachedAttributes;
};

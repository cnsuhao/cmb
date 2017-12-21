//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBTexturedObject - represents a 3D Scene object that can be textured mapped.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBTexturedObject_h
#define __pqCMBTexturedObject_h

#include "cmbSystemConfig.h"
#include "pqCMBSceneObjectBase.h"

class CMBAPPCOMMON_EXPORT pqCMBTexturedObject : public pqCMBSceneObjectBase
{
public:
  pqCMBTexturedObject();
  pqCMBTexturedObject(pqPipelineSource* source, pqRenderView* view, pqServer* server);
  ~pqCMBTexturedObject() override;

  /// Returns the Bounds of the data. - Returns the output of the Elevation
  void getDataBounds(double bounds[6]) const override;
  pqPipelineSource* getSelectionSource() const override;
  void setSelectionInput(vtkSMSourceProxy* selectionInput) override;
  vtkSMSourceProxy* getSelectionInput() const override;

  void showElevation(bool flag);
  void toggleElevation() { this->showElevation(!this->ShowElevation); }
  bool showingElevation() const { return this->ShowElevation; }
  const double* getRegistrationPoints() { return this->RegistrationPoints; }

  bool hasTexture() const { return (this->NumberOfRegistrationPoints > 0); }

  const QString& getTextureFileName() { return this->TextureFileName; }

  int getNumberOfRegistrationPoints() const { return this->NumberOfRegistrationPoints; }

  void getRegistrationPointPair(int i, double xy[2], double st[2]) const;

  double getTextureIntensityAtPoint(double pt[3]);
  pqServer* getTextureRegistrationServer(void);

  void setTextureMap(const char* filename, int numberOfRegistrationPoints, double* points);
  void unsetTextureMap();

protected:
  QPointer<pqPipelineSource> RegisterTextureFilter;
  QPointer<pqPipelineSource> TexturePointIntensityFilter;
  QPointer<pqPipelineSource> ElevationFilter;
  QPointer<pqPipelineSource> TextureImageSource;
  QString TextureFileName;
  int NumberOfRegistrationPoints;
  double RegistrationPoints[12];

  bool ShowElevation;
  void prepTexturedObject(pqServer* server, pqRenderView* view);
  void duplicateInternals(pqCMBSceneObjectBase* obj) override;
};

#endif /* __pqCMBTexturedObject_h */

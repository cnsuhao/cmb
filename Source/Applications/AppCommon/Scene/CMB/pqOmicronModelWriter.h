//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef _pqOmicronModelWriter_h
#define _pqOmicronModelWriter_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <QList>
#include <QString>
#include <vector>

class pqCMBSceneObjectBase;

class CMBAPPCOMMON_EXPORT pqOmicronModelWriter
{
public:
  pqOmicronModelWriter();
  ~pqOmicronModelWriter();

  void SetVOI(pqCMBSceneObjectBase *voi);
  void SetSurfaceFileName(const char *fileName)
    { this->SurfaceFileName = fileName; }
  const char *GetSurfaceFileName()
    {
    return this->SurfaceFileName.c_str();
    }
  void SetSurfacePosition(double x, double y, double z)
    {
    this->SurfacePosition[0] = x;
    this->SurfacePosition[1] = y;
    this->SurfacePosition[2] = z;
    }
  void SetSurfacePosition(double position[3])
    {
    this->SetSurfacePosition(position[0], position[1], position[2]);
    }
  void SetSurfaceOrientation(double orientX, double orientY, double orientZ)
    {
    this->SurfaceOrientation[0] = orientX;
    this->SurfaceOrientation[1] = orientY;
    this->SurfaceOrientation[2] = orientZ;
    }
  void SetSurfaceOrientation(double orientation[3])
    {
    this->SetSurfaceOrientation(orientation[0], orientation[1], orientation[2]);
    }
  void SetSurfaceScale(double scale)
    {
    this->SurfaceScale = scale;
    }
  void SetSurfaceInitialTranslation(double initialTranslation[3])
    {
    this->SurfaceInitialTranslation[0] = initialTranslation[0];
    this->SurfaceInitialTranslation[1] = initialTranslation[1];
    this->SurfaceInitialTranslation[2] = initialTranslation[2];
    }

  void AddSolid(pqCMBSceneObjectBase *solid);

  void SetAreaConstraint(double areaConstraint)
    { this->AreaConstraint = areaConstraint; }

  void SetVolumeConstraint(double volumeConstraint)
    { this->VolumeConstraint = volumeConstraint; }

  void SetDiscConstraint(double discConstraint)
    { this->DiscConstraint = discConstraint; }

  void SetFileName(const char *fileName)
    { this->FileName = fileName; }

  void SetHeaderComment(const char *comment)
    { this->HeaderComment = comment; }

  void SetOutputBaseName(const char *baseName)
    { this->OutputBaseName = baseName; }

  void Write();

private:
  pqCMBSceneObjectBase *VOI;
  pqCMBSceneObjectBase *Surface;
  std::vector<pqCMBSceneObjectBase *> Solids;

  double SurfacePosition[3];
  double SurfaceOrientation[3];
  double SurfaceScale;
  double SurfaceInitialTranslation[3];
  double AreaConstraint;
  double VolumeConstraint;
  double DiscConstraint;
  std::string FileName;
  std::string SurfaceFileName;
  std::string HeaderComment;
  std::string OutputBaseName;
};


#endif // !_pqOmicronModelWriter_h

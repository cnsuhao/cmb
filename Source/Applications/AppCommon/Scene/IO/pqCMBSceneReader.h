//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBSceneReader - reads a XML representation of a Scene.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBSceneReader_h
#define __pqCMBSceneReader_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <string>
#include <QStringList>

class pqCMBSceneTree;
class CMBAPPCOMMON_EXPORT pqCMBSceneReader
{
public:
  pqCMBSceneReader();
  virtual ~pqCMBSceneReader();

  void setTree(pqCMBSceneTree *tree)
    {this->Tree = tree;}

  pqCMBSceneTree *getTree() const
    {return this->Tree;}

  std::string getStatusMessage() const
    {return this->Status;}

  void setFileName(const char *name)
    { this->FileName = name;}
  std::string getFileName() const
    {return this->FileName;}

  void setUseBoundsConstraint(int val)
  { this->UseBoundsConstraint = val; }
  void setFilterObjectByType(int val)
  { this->FilterObjectByType = val; }
  void setBoundsConstraint(double bounds[6])
    {
    for(int i=0; i<6; i++)
      {
      this->BoundsConstraint[i] = bounds[i];
      }
    }
  void setFilterObjectTypes(QStringList& objTypes)
    {
    this->FilterObjectTypes.clear();
    this->FilterObjectTypes << objTypes;
    }

  int getUserDefinedObjectTypes(
    const char *data, QStringList& objTypes);
  int process(const char *data);

protected:
  pqCMBSceneTree *Tree;
  std::string Status;
  std::string FileName;

  int UseBoundsConstraint;
  double BoundsConstraint[6];
  int FilterObjectByType;
  QStringList FilterObjectTypes;
};

#endif /* __pqCMBSceneReader_h */

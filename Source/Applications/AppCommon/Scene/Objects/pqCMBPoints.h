//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBPoints - represents a set of Points that can be used to represent Scatter Point Data.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqCMBPoints_h
#define __pqCMBPoints_h

#include "pqCMBTexturedObject.h"
#include "cmbSystemConfig.h"

class  CMBAPPCOMMON_EXPORT pqCMBPoints : public pqCMBTexturedObject
{
public:

  pqCMBPoints();
  pqCMBPoints(pqPipelineSource *source,
                 pqRenderView *view,
                 pqServer *server,
                 const char *filename);
  pqCMBPoints(pqPipelineSource *source,
                 pqRenderView *view,
                 pqServer *server,
                 bool updateRep=true);
  pqCMBPoints(const char *filename,
                 pqServer *server, pqRenderView *view,
                 int maxNumberOfPoints,
                 bool updateRep = true);
  pqCMBPoints(pqServer *server, pqRenderView *view,
                 pqPipelineSource* source, int pieceIndex,
                 int onRatio, bool doublePrecision);

  virtual ~pqCMBPoints();
  virtual pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                bool updateRep = true);

  pqPipelineSource * getTransformedSource(pqServer *server) const;

  void setFileName(const char *type)
    {this->FileName = type;}
  virtual enumObjectType getType() const;
  std::string getFileName() const
    {return this->FileName;}

  static bool isPointsFile(const char *fileName);

  void setReaderSource(pqPipelineSource *source);
  pqPipelineSource * getReaderSource() const;

  void setPieceTotalNumberOfPoints(int numberOfPoints)
    { this->PieceTotalNumberOfPoints = numberOfPoints; }
  int getPieceTotalNumberOfPoints()
    { return this->PieceTotalNumberOfPoints; }

  void setPieceId(int pieceId)
    { this->PieceId = pieceId; }
  int getPieceId()
    { return this->PieceId; }

  void setPieceOnRatio(int onRatio)
    { this->PieceOnRatio = onRatio; }
  int getPieceOnRatio()
    { return this->PieceOnRatio; }

  void setDoubleDataPrecision(bool state)
    { this->DoubleDataPrecision = state; }
  bool getDoubleDataPrecision() const
    { return this->DoubleDataPrecision; }
  void setInitialSurfaceTranslation(double translation[3]);
  void getInitialSurfaceTranslation(double translation[3]) const;



protected:
  void prepPoints(pqServer *server, pqRenderView *view);
  void initialize(pqPipelineSource* source,
                  pqServer *server,
                  pqRenderView *view,
                  bool updateRep);
  std::string FileName;
  QPointer<pqPipelineSource> ReaderSource;
  int PieceTotalNumberOfPoints;
  int PieceId;
  int PieceOnRatio; // could calculate, but here for when we save
  bool DoubleDataPrecision;
  double InitialSurfaceTranslation[3];
};

#endif /* __pqCMBPoints_h */

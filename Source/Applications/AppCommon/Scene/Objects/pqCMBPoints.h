/*=========================================================================

  Program:   CMB
  Module:    pqCMBPoints.h

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
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

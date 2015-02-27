/*=========================================================================

  Program:   CMB
  Module:    pqCMBLIDARReaderManager.h

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
// .NAME pqCMBLIDARReaderManager - .
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBLIDARReaderManager_h
#define __pqCMBLIDARReaderManager_h

#include <QObject>
#include <QList>
#include <QMap>
#include "vtkWeakPointer.h"
#include "cmbSystemConfig.h"

class pqCMBPointsBuilderMainWindowCore;
class pqObjectBuilder;
class pqPipelineSource;
class pqCMBLIDARPieceTable;
class pqCMBLIDARPieceObject;
class vtkSMSourceProxy;
class pqCMBModifierArcManager;

class pqCMBLIDARReaderManager :  public QObject
{
  Q_OBJECT

public:
  pqCMBLIDARReaderManager(pqCMBPointsBuilderMainWindowCore*,pqObjectBuilder*);
  virtual ~pqCMBLIDARReaderManager();

  bool userRequestsDoubleData();
  bool setOutputDataTypeToDouble();
  void convertFromLatLongToXYZ(int convertFromLatLong);

  void limitReadToBounds(bool limitRead);
  void setReadBounds(QList<QVariant> &values);

  void getDataBounds(double bounds[6]);

  int importData(const char* filename, pqCMBLIDARPieceTable*, pqCMBModifierArcManager*,
    QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >&);

  vtkSMSourceProxy *getReaderSourceProxy(const char* filename);

  void setReaderTransform(vtkSMSourceProxy* readerProxy,
    pqCMBLIDARPieceObject *dataObj);
  void setReaderTransformData(vtkSMSourceProxy* readerProxy,bool transformData);
  void clearTransform(vtkSMSourceProxy* readerProxy);

  void readPieces(pqPipelineSource* reader,
    QList<QVariant> &pieces, QList<pqPipelineSource*> &pdSources);

  void updatePieces(const char* filename,
    QList<pqCMBLIDARPieceObject*> &pieces,
    bool forceRead, pqCMBLIDARPieceTable *table, bool clipData,
    double *clipBounds);

  bool getSourcesForOutput(bool atDisplayRatio,
    QList<pqCMBLIDARPieceObject*> &Pieces,
    QList<pqPipelineSource*> &outputSources, bool forceUpdate = false);

  void destroyTemporarySources();

  vtkIdType scanFileNumberOfPoints(pqPipelineSource* reader);
  vtkIdType scanLASPiecesInfo(pqPipelineSource* reader);
  void destroyAllReaders();
  QString getFileTitle() const;
  bool isValidFile(const char *filename);
  bool isFileLoaded(QString& filename);
  bool hasReaderSources()
  {return this->ReaderSourceMap.count()>0;}
  vtkIdType scanTotalNumPointsInfo(const QStringList& files, pqPipelineSource* reader = NULL);
  QMap<QString, pqPipelineSource*>& readerSourceMap()
    { return this->ReaderSourceMap;}
  vtkSMSourceProxy* activeReader();
  QList<pqCMBLIDARPieceObject *> getFilePieceObjects(
    const char* filename, QList<pqCMBLIDARPieceObject *> &sourcePieces);

protected:

  int computeApproximateRepresentingFloatDigits(double min, double max);
  int importLIDARData(const char* filenmame,pqCMBLIDARPieceTable*,
                      pqCMBModifierArcManager*,
    QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >&);
  int importLASData(const char* filenmame,pqCMBLIDARPieceTable*,
                    pqCMBModifierArcManager*,
    QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >&);
  int importDEMData(const char* filenmame,pqCMBLIDARPieceTable*,
                    pqCMBModifierArcManager*,
    QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >&);
  vtkIdType getPieceNumPointsInfo(const char* filename, QList<vtkIdType> &pieceInfo);

  int readData(vtkSMSourceProxy* readerProxy,QList<QVariant> &pieces);
  void updateLIDARPieces(const char* filename,
    QList<pqCMBLIDARPieceObject*> &pieces,
    bool forceRead, pqCMBLIDARPieceTable *table, bool clipData,
    double *clipBounds);
  void updateLASPieces(const char* filename,
    QList<pqCMBLIDARPieceObject*> &pieces,
    bool forceRead, pqCMBLIDARPieceTable *table, bool clipData,
    double *clipBounds);
  bool getSourcesForOutputLIDAR(pqPipelineSource* reader,bool atDisplayRatio,
    QList<pqCMBLIDARPieceObject*> &Pieces,
    QList<pqPipelineSource*> &outputSources, bool forceUpdate);
  bool getSourcesForOutputLAS(pqPipelineSource* reader,bool atDisplayRatio,
    QList<pqCMBLIDARPieceObject*> &Pieces,
    QList<pqPipelineSource*> &outputSources, bool forceUpdate);


  pqCMBPointsBuilderMainWindowCore *Core;
  pqObjectBuilder    *Builder;
  // Map for <Filename, readersource >
  QMap<QString, pqPipelineSource*> ReaderSourceMap;
  vtkWeakPointer<vtkSMSourceProxy> ActiveReader;

  double CurrentReaderBounds[6];
  QList<pqPipelineSource*> TemporaryPDSources;
  QList<pqPipelineSource*> TemporaryThresholdFilters;
  QList<pqPipelineSource*> TemporaryContourFilters;

private:
  struct LASPieceInfo
    {
    std::string ClassificationName;
    vtkIdType NumberOfPointsInClassification;
    };
  std::map<unsigned char, LASPieceInfo> CurrentLASPieces;
  double DEMTransformForZOrigin[2];
  double DEMZRotationAngle;
};

#endif /* __pqCMBLIDARReaderManager_h */

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBLIDARReaderManager.h"

#include "pqCMBLIDARPieceObject.h"
#include "pqCMBLIDARPieceTable.h"
#include "pqCMBPointsBuilderMainWindowCore.h"

#include "pqCMBModifierArcManager.h"

// Qt headers
#include <QFileInfo>
#include <QMessageBox>
#include <QStringList>
#include <QtDebug>

#include "pqFileDialogModel.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"

#include "vtkNew.h"
#include "vtkPVCompositeDataInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkPVLASOutputBlockInformation.h"
#include "vtkProcessModule.h"
#include "vtkSMDataSourceProxy.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMOutputPort.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMSourceProxy.h"
#include "vtkTransform.h"
#include <vtksys/SystemTools.hxx>

#include "pqPluginIOBehavior.h"

namespace
{
const char* SM_LIDAR_READER_NAME = "LIDARReader";
const char* SM_LAS_READER_NAME = "LASReader";
const char* SM_DEM_READER_NAME = "RawDEMReader";
const char* SM_GDAL_READER_NAME = "GDALRasterPolydataWrapper";
const char* SM_VTP_READER_NAME = "XMLPolyDataReader";
};

//-----------------------------------------------------------------------------
pqCMBLIDARReaderManager::pqCMBLIDARReaderManager(
  pqCMBPointsBuilderMainWindowCore* core, pqObjectBuilder* builder)
{
  this->Core = core;
  this->Builder = builder;
  this->ActiveReader = NULL;
  this->DEMTransformForZOrigin[0] = this->DEMTransformForZOrigin[1] = 90.0;
  this->DEMZRotationAngle = 0.0;
  this->ChangeOrigin = false;
}

//-----------------------------------------------------------------------------
pqCMBLIDARReaderManager::~pqCMBLIDARReaderManager()
{
}
//-----------------------------------------------------------------------------
vtkSMSourceProxy* pqCMBLIDARReaderManager::activeReader()
{
  return this->ActiveReader;
}

//-----------------------------------------------------------------------------
vtkSMSourceProxy* pqCMBLIDARReaderManager::getReaderSourceProxy(const char* filename)
{
  if (this->ReaderSourceMap.contains(filename))
  {
    return vtkSMSourceProxy::SafeDownCast(this->ReaderSourceMap[filename]->getProxy());
  }
  return NULL;
}
//-----------------------------------------------------------------------------
QString pqCMBLIDARReaderManager::getFileTitle() const
{
  QString filename("");
  int numFiles = this->ReaderSourceMap.count();
  if (numFiles == 0)
  {
    return filename;
  }
  filename = this->ReaderSourceMap.keys()[0];
  if (numFiles > 1)
  {
    filename.append(QString(" (... %1 files)").arg(numFiles));
  }
  return filename;
}
//-----------------------------------------------------------------------------
bool pqCMBLIDARReaderManager::isFileLoaded(QString& filename)
{
  return this->ReaderSourceMap.contains(filename);
}
//-----------------------------------------------------------------------------
bool pqCMBLIDARReaderManager::isValidFile(const char* filename)
{
  if (!(filename && *filename))
  {
    return false;
  }
  pqFileDialogModel model(this->Core->getActiveServer());
  QString fullpath;

  if (!model.fileExists(filename, fullpath))
  {
    QMessageBox::information(this->Core->parentWidget(), "LIDAR", "The File does not exist!");
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
vtkIdType pqCMBLIDARReaderManager::scanTotalNumPointsInfo(
  const QStringList& files, pqPipelineSource* reader)
{
  vtkIdType totNumPts = 0;
  foreach (QString filename, files)
  {
    if (this->isValidFile(filename.toLatin1().constData()))
    {
      if (reader)
      {
        this->ReaderSourceMap[filename] = reader;
      }
      else
      {
        QString message("No reader created for file, ");
        message.append(filename);
        QMessageBox::warning(NULL, tr("File Open Warning"), message, QMessageBox::Ok);
        continue;
      }

      vtkSMSourceProxy* readerProxy = this->getReaderSourceProxy(filename.toLatin1().constData());
      if (!readerProxy)
      {
        continue;
      }
      totNumPts += this->scanFileNumberOfPoints(this->ReaderSourceMap[filename]);
    }
  }
  return totNumPts;
}

//-----------------------------------------------------------------------------

bool pqCMBLIDARReaderManager::setOrigin()
{
  if (!this->ReaderSourceMap.count())
  {
    return false;
  }
  Origin[0] = (this->CurrentReaderBounds[1] + this->CurrentReaderBounds[0]) * 0.5;
  Origin[1] = (this->CurrentReaderBounds[3] + this->CurrentReaderBounds[2]) * 0.5;
  Origin[2] = (this->CurrentReaderBounds[5] + this->CurrentReaderBounds[4]) * 0.5;
  this->ChangeOrigin = true;
  vtkSMProxy* readerProxy;
  foreach (QString filename, this->ReaderSourceMap.uniqueKeys())
  {
    readerProxy = this->ReaderSourceMap[filename]->getProxy();
    QString readerName(readerProxy->GetXMLName());
    if (readerName.compare(SM_LIDAR_READER_NAME) == 0 ||
      readerName.compare(SM_LAS_READER_NAME) == 0 || readerName.compare(SM_DEM_READER_NAME) == 0 ||
      readerName.compare(SM_GDAL_READER_NAME) == 0 ||
      readerProxy->GetProperty("Origin")) // Could be from plugin
    {
      vtkSMPropertyHelper(readerProxy, "Origin").Set(Origin, 3);
      readerProxy->UpdateProperty("Origin");
    }
  }
  this->CurrentReaderBounds[0] -= Origin[0];
  this->CurrentReaderBounds[1] -= Origin[0];
  this->CurrentReaderBounds[2] -= Origin[1];
  this->CurrentReaderBounds[3] -= Origin[1];
  this->CurrentReaderBounds[4] -= Origin[2];
  this->CurrentReaderBounds[5] -= Origin[2];
  return true;
}

bool pqCMBLIDARReaderManager::setOutputDataTypeToDouble()
{
  if (!this->ReaderSourceMap.count())
  {
    return false;
  }
  vtkSMProxy* readerProxy;
  foreach (QString filename, this->ReaderSourceMap.uniqueKeys())
  {
    readerProxy = this->ReaderSourceMap[filename]->getProxy();
    QString readerName(readerProxy->GetXMLName());
    if (readerName.compare(SM_LIDAR_READER_NAME) == 0 ||
      readerName.compare(SM_LAS_READER_NAME) == 0 || readerName.compare(SM_DEM_READER_NAME) == 0 ||
      readerProxy->GetProperty("OutputDataTypeIsDouble")) // Could be from plugin
    {
      vtkSMPropertyHelper(readerProxy, "OutputDataTypeIsDouble").Set(true);
      readerProxy->UpdateProperty("OutputDataTypeIsDouble");
    }
  }

  return true;
}

//-----------------------------------------------------------------------------
void pqCMBLIDARReaderManager::convertFromLatLongToXYZ(int convertFromLatLong)
{
  if (!this->ReaderSourceMap.count())
  {
    return;
  }
  vtkSMProxy* readerProxy;
  foreach (QString filename, this->ReaderSourceMap.uniqueKeys())
  {
    readerProxy = this->ReaderSourceMap[filename]->getProxy();
    QString readerName(readerProxy->GetXMLName());
    if (readerName.compare(SM_LIDAR_READER_NAME) == 0 ||
      readerName.compare(SM_LAS_READER_NAME) == 0 || readerName.compare(SM_DEM_READER_NAME) == 0 ||
      readerProxy->GetProperty("ConvertFromLatLongToXYZ")) // Could be from plugin
    {
      vtkSMPropertyHelper(readerProxy, "ConvertFromLatLongToXYZ").Set(convertFromLatLong);
      readerProxy->UpdateProperty("ConvertFromLatLongToXYZ");
    }
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARReaderManager::limitReadToBounds(bool limitRead)
{
  if (!this->ReaderSourceMap.count())
  {
    return;
  }
  vtkSMProxy* readerProxy;
  foreach (QString filename, this->ReaderSourceMap.uniqueKeys())
  {
    readerProxy = this->ReaderSourceMap[filename]->getProxy();
    QString readerName(readerProxy->GetXMLName());
    if (readerName.compare(SM_LIDAR_READER_NAME) == 0 ||
      readerName.compare(SM_LAS_READER_NAME) == 0 || readerName.compare(SM_DEM_READER_NAME) == 0 ||
      readerName.compare(SM_GDAL_READER_NAME) == 0 ||
      readerProxy->GetProperty("LimitReadToBounds")) // Could be from plugin
    {
      vtkSMPropertyHelper(readerProxy, "LimitReadToBounds").Set(limitRead);
      readerProxy->UpdateProperty("LimitReadToBounds");
    }
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARReaderManager::setReadBounds(QList<QVariant>& values)
{
  if (!this->ReaderSourceMap.count())
  {
    return;
  }
  vtkSMProxy* readerProxy;
  foreach (QString filename, this->ReaderSourceMap.uniqueKeys())
  {
    readerProxy = this->ReaderSourceMap[filename]->getProxy();
    QString readerName(readerProxy->GetXMLName());
    if (readerName.compare(SM_LIDAR_READER_NAME) == 0 ||
      readerName.compare(SM_LAS_READER_NAME) == 0 || readerName.compare(SM_DEM_READER_NAME) == 0 ||
      readerName.compare(SM_GDAL_READER_NAME) == 0 ||
      readerProxy->GetProperty("ReadBounds")) // Could be from plugin
    {
      pqSMAdaptor::setMultipleElementProperty(readerProxy->GetProperty("ReadBounds"), values);
      readerProxy->UpdateProperty("ReadBounds");
    }
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARReaderManager::getDataBounds(double bounds[6])
{
  if (!this->ReaderSourceMap.count())
  {
    return;
  }

  vtkSMProxy* readerProxy;
  vtkBoundingBox bb;
  foreach (QString filename, this->ReaderSourceMap.uniqueKeys())
  {
    readerProxy = this->ReaderSourceMap[filename]->getProxy();
    QString readerName(readerProxy->GetXMLName());
    if (readerName.compare(SM_LIDAR_READER_NAME) == 0 ||
      readerName.compare(SM_LAS_READER_NAME) == 0 || readerName.compare(SM_DEM_READER_NAME) == 0 ||
      readerProxy->GetProperty("DataBounds")) // Could be from plugin
    {
      readerProxy->UpdatePropertyInformation();
      double* dataBounds =
        vtkSMDoubleVectorProperty::SafeDownCast(readerProxy->GetProperty("DataBounds"))
          ->GetElements();
      bb.AddBounds(dataBounds);
    }
    else if (readerName.compare(SM_VTP_READER_NAME) == 0)
    {
      readerProxy->UpdatePropertyInformation();
      vtkPVDataInformation* dataInfo =
        this->ReaderSourceMap[filename]->getOutputPort(0)->getDataInformation();
      double dbounds[6];
      dataInfo->GetBounds(dbounds);
      bb.AddBounds(dbounds);
    }
  }
  if (bb.IsValid())
  {
    double totBounds[6];
    bb.GetBounds(totBounds);
    memcpy(bounds, totBounds, sizeof(double) * 6);
  }
}

//----------------------------------------------------------------------------
int pqCMBLIDARReaderManager::computeApproximateRepresentingFloatDigits(double min, double max)
{
  double maxComponent = fabs(min) > max ? fabs(min) : max;
  double logMaxComponent = maxComponent != 0 ? log10(maxComponent) : 0;
  double logRange = max - min != 0 ? log10(max - min) : 0;
  // not rounding, but  throw in 0.2 offset so that can be close to 4 digits and
  // be ok
  return static_cast<int>(7.0 - ceil(logMaxComponent - logRange - 0.2));
}

bool pqCMBLIDARReaderManager::userRequestsOrigin()
{
  if (this->CurrentReaderBounds[1] < this->CurrentReaderBounds[0] || ChangeOrigin)
  {
    return false;
  }
  double dx = this->CurrentReaderBounds[1] - this->CurrentReaderBounds[0];
  double dy = this->CurrentReaderBounds[3] - this->CurrentReaderBounds[2];
  double dz = this->CurrentReaderBounds[5] - this->CurrentReaderBounds[4];
  double rx = this->CurrentReaderBounds[0] / dx, ry = this->CurrentReaderBounds[2] / dy,
         rz = this->CurrentReaderBounds[4] / dz;
  double max = std::max(rx, std::max(ry, rz));
  if (max > 10000 &&
    QMessageBox::question(this->Core->parentWidget(), "Data might be too far from origin",
      tr("Data might be too far from 0,0,0 for proper rendering.  "
         "Do you want to change the origin of the data"),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
  {
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARReaderManager::userRequestsDoubleData()
{
  if (this->CurrentReaderBounds[1] < this->CurrentReaderBounds[0])
  {
    return false;
  }

  // all default to float right now, but need to be able to check at some point
  int xDigits = this->computeApproximateRepresentingFloatDigits(
    this->CurrentReaderBounds[0], this->CurrentReaderBounds[1]);
  int yDigits = this->computeApproximateRepresentingFloatDigits(
    this->CurrentReaderBounds[2], this->CurrentReaderBounds[3]);
  int zDigits = this->computeApproximateRepresentingFloatDigits(
    this->CurrentReaderBounds[4], this->CurrentReaderBounds[5]);

  int minDigits = xDigits < yDigits ? xDigits : yDigits;
  minDigits = zDigits < minDigits ? zDigits : minDigits;

  if (minDigits < 4 &&
    QMessageBox::question(this->Core->parentWidget(), "Questionable float precision!",
      tr("Potentially insufficient precision with float representation (< ~")
        .append(QString::number(minDigits + 1))
        .append(" digits).  Use double instead?"),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
  {
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
int pqCMBLIDARReaderManager::importData(const char* filename, pqCMBLIDARPieceTable* table,
  pqCMBModifierArcManager* arcManager,
  QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >& FilePieceIdMap, bool showElevation)
{
  if (!this->getReaderSourceProxy(filename))
  {
    return 0;
  }

  QString readerName = this->ReaderSourceMap[filename]->getProxy()->GetXMLName();
  int res = 0;
  if (readerName.compare(SM_LIDAR_READER_NAME) == 0)
  {
    res = this->importLIDARData(filename, table, arcManager, FilePieceIdMap, showElevation);
  }
  else if (readerName.compare(SM_LAS_READER_NAME) == 0)
  {
    res = this->importLASData(filename, table, arcManager, FilePieceIdMap, showElevation);
  }
  else if (readerName.compare(SM_DEM_READER_NAME) == 0)
  {
    res = this->importDEMData(filename, table, arcManager, FilePieceIdMap, showElevation);
  }
  else if (readerName.compare(SM_GDAL_READER_NAME) == 0)
  {
    res = importGDALData(filename, table, arcManager, FilePieceIdMap, showElevation);
  }
  else if (readerName.compare(SM_VTP_READER_NAME) == 0)
  {
    res = importVTPData(filename, table, arcManager, FilePieceIdMap, showElevation);
  }
  else if (pqPluginIOBehavior::isPluginReader(
             this->ReaderSourceMap[filename]->getProxy()->GetHints()))
  {
    // for now, we just use the lidar pts as an example.
    // Eventually, the plugin should have its own ui components.
    res = this->importLIDARData(filename, table, arcManager, FilePieceIdMap, showElevation);
  }

  return res;
}
//-----------------------------------------------------------------------------
vtkIdType pqCMBLIDARReaderManager::scanFileNumberOfPoints(pqPipelineSource* reader)
{
  vtkSMSourceProxy* readerProxy = vtkSMSourceProxy::SafeDownCast(reader->getProxy());
  if (!readerProxy)
  {
    return 0;
  }
  QString readerName(readerProxy->GetXMLName());
  vtkIdType totNumPts = 0;
  this->Core->enableAbort(true); //make sure progress bar is enabled
  std::string tmp = readerName.toStdString();
  if (readerName.compare(SM_LIDAR_READER_NAME) == 0)
  {
    QList<QVariant> pieceOnRatioList;
    pieceOnRatioList << 0 << VTK_INT_MAX;
    pqSMAdaptor::setMultipleElementProperty(
      readerProxy->GetProperty("RequestedPiecesForRead"), pieceOnRatioList);
    pqSMAdaptor::setElementProperty(readerProxy->GetProperty("AbortExecute"), 0);
    readerProxy->UpdateVTKObjects();
    readerProxy->UpdatePipeline();
    int aborted = pqSMAdaptor::getElementProperty(readerProxy->GetProperty("AbortExecute")).toInt();
    if (aborted)
    {
      this->Core->enableAbort(false);
      return totNumPts;
    }
    readerProxy->UpdatePropertyInformation();
    totNumPts =
      pqSMAdaptor::getElementProperty(readerProxy->GetProperty("TotalNumberOfPoints")).toInt();
  }
  else if (readerName.compare(SM_LAS_READER_NAME) == 0)
  {
    totNumPts += this->scanLASPiecesInfo(reader);
  }
  else if (readerName.compare(SM_DEM_READER_NAME) == 0)
  {
    // For DEM reader, the output has to be polydata
    vtkSMPropertyHelper(readerProxy, "OutputImageData").Set(0);
    readerProxy->UpdateVTKObjects();
    readerProxy->UpdatePropertyInformation();
    //    readerProxy->UpdatePipeline();
    totNumPts =
      pqSMAdaptor::getElementProperty(readerProxy->GetProperty("TotalNumberOfPoints")).toInt();
  }
  else if (readerName.compare(SM_GDAL_READER_NAME) == 0)
  {
    readerProxy->UpdateVTKObjects();
    readerProxy->UpdatePipeline();
    readerProxy->UpdatePropertyInformation();
    totNumPts =
      pqSMAdaptor::getElementProperty(readerProxy->GetProperty("TotalNumberOfPoints")).toInt();
  }
  else if (readerName.compare(SM_VTP_READER_NAME) == 0)
  {
    readerProxy->UpdateVTKObjects();
    readerProxy->UpdatePipeline();
    readerProxy->UpdatePropertyInformation();
    vtkPVDataInformation* dataInfo = reader->getOutputPort(0)->getDataInformation();
    totNumPts = dataInfo->GetNumberOfPoints();
  }
  else if (pqPluginIOBehavior::isPluginReader(readerProxy->GetHints()) &&
    readerProxy->GetProperty("TotalNumberOfPoints")) // Could be from plugin
  {
    if (readerProxy->GetProperty("RequestedPiecesForRead"))
    {
      QList<QVariant> pieceOnRatioList;
      pieceOnRatioList << 0 << VTK_INT_MAX;
      pqSMAdaptor::setMultipleElementProperty(
        readerProxy->GetProperty("RequestedPiecesForRead"), pieceOnRatioList);
    }
    if (readerProxy->GetProperty("AbortExecute"))
    {
      pqSMAdaptor::setElementProperty(readerProxy->GetProperty("AbortExecute"), 0);
    }
    readerProxy->UpdateVTKObjects();
    readerProxy->UpdatePipeline();
    if (readerProxy->GetProperty("AbortExecute"))
    {
      int aborted =
        pqSMAdaptor::getElementProperty(readerProxy->GetProperty("AbortExecute")).toInt();
      if (aborted)
      {
        this->Core->enableAbort(false);
        return totNumPts;
      }
    }
    readerProxy->UpdatePropertyInformation();
    totNumPts =
      pqSMAdaptor::getElementProperty(readerProxy->GetProperty("TotalNumberOfPoints")).toInt();
  }

  this->Core->enableAbort(false);
  return totNumPts;
}

//-----------------------------------------------------------------------------
vtkIdType pqCMBLIDARReaderManager::scanLASPiecesInfo(pqPipelineSource* reader)
{
  vtkIdType totNumPts = 0;
  vtkSMSourceProxy* readerProxy = vtkSMSourceProxy::SafeDownCast(reader->getProxy());
  if (!readerProxy)
  {
    return totNumPts;
  }
  pqSMAdaptor::setElementProperty(readerProxy->GetProperty("AbortExecute"), 0);
  pqSMAdaptor::setElementProperty(readerProxy->GetProperty("ScanMode"), 1);
  readerProxy->UpdateVTKObjects();
  readerProxy->UpdatePipeline();
  int aborted = pqSMAdaptor::getElementProperty(readerProxy->GetProperty("AbortExecute")).toInt();
  if (aborted)
  {
    return totNumPts;
  }

  vtkSMOutputPort* outputPort = readerProxy->GetOutputPort(static_cast<unsigned int>(0));
  vtkPVCompositeDataInformation* compositeInformation =
    outputPort->GetDataInformation()->GetCompositeDataInformation();
  int numBlocks = compositeInformation->GetNumberOfChildren();
  this->CurrentLASPieces.clear();
  unsigned char classification;
  for (int i = 0; i < numBlocks; i++)
  {
    pqPipelineSource* extract = this->Builder->createFilter("filters", "ExtractLeafBlock", reader);

    pqSMAdaptor::setElementProperty(extract->getProxy()->GetProperty("BlockIndex"), i);
    extract->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy::SafeDownCast(extract->getProxy())->UpdatePipeline();

    vtkNew<vtkPVLASOutputBlockInformation> info;
    extract->getProxy()->GatherInformation(info.GetPointer());
    this->Builder->destroy(extract);

    LASPieceInfo piece;
    piece.ClassificationName = info->GetClassificationName();
    piece.NumberOfPointsInClassification = info->GetNumberOfPointsInClassification();
    totNumPts += piece.NumberOfPointsInClassification;

    classification = info->GetClassification();
    this->CurrentLASPieces[classification] = piece;
  }
  return totNumPts;
}

//-----------------------------------------------------------------------------
int pqCMBLIDARReaderManager::importLIDARData(const char* filename, pqCMBLIDARPieceTable* table,
  pqCMBModifierArcManager* arcManager,
  QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >& FilePieceIdMap, bool showElevation)
{
  vtkSMSourceProxy* readerProxy = this->getReaderSourceProxy(filename);
  if (!readerProxy)
  {
    return 0;
  }

  if (ChangeOrigin)
  {
    vtkSMPropertyHelper(readerProxy, "Origin").Set(Origin, 3);
  }

  // 1, -1 used to indicate CurrentReaderBounds not yet set
  this->CurrentReaderBounds[0] = 1;
  this->CurrentReaderBounds[1] = -1;
  QList<vtkIdType> pieceInfo;
  int totalNumberOfPoints = this->getPieceNumPointsInfo(filename, pieceInfo);
  int onRatio, mainOnRatio = this->Core->calculateMainOnRatio(totalNumberOfPoints);

  this->Core->enableAbort(true); //make sure progress bar is enabled
  vtkBoundingBox bbox;
  int aborted;
  for (int pieceId = 0; pieceId < pieceInfo.count(); pieceId++)
  {
    onRatio = this->Core->calculateOnRatioForPiece(mainOnRatio, pieceInfo[pieceId]);

    QList<QVariant> pieceOnRatioList;
    pieceOnRatioList << pieceId << onRatio;

    QList<pqPipelineSource*> pdSource;
    this->readPieces(this->ReaderSourceMap[filename], pieceOnRatioList, pdSource);

    aborted = pqSMAdaptor::getElementProperty(readerProxy->GetProperty("AbortExecute")).toInt();
    if (aborted)
    {
      break;
    }

    if (!pdSource.count() || !pdSource[0])
    {
      if (QMessageBox::question(this->Core->parentWidget(), "LIDAR File Reading",
            tr("Failed to load LIDAR piece: ")
              .append(QString::number(pieceId))
              .append(". Continue to load other pieces?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
      {
        continue;
      }
      else
      {
        break;
      }
    }
    int visible = pieceInfo[pieceId] > this->Core->getMinimumNumberOfPointsPerPiece() ? 1 : 0;

    readerProxy->UpdatePropertyInformation();
    int numberOfPointsRead =
      pqSMAdaptor::getElementProperty(readerProxy->GetProperty("RealNumberOfOutputPoints")).toInt();
    QList<QVariant> values =
      pqSMAdaptor::getMultipleElementProperty(readerProxy->GetProperty("DataBounds"));
    double bounds[6] = { values[0].toDouble(), values[1].toDouble(), values[2].toDouble(),
      values[3].toDouble(), values[4].toDouble(), values[5].toDouble() };

    bbox.AddBounds(bounds);

    pqCMBLIDARPieceObject* dataObj = pqCMBLIDARPieceObject::createObject(
      pdSource[0], bounds, this->Core->getActiveServer(), this->Core->activeRenderView(), visible);

    dataObj->setPieceIndex(pieceId);
    dataObj->setDisplayOnRatio(onRatio);
    dataObj->setReadOnRatio(onRatio);
    dataObj->setNumberOfPoints(pieceInfo[pieceId]);
    dataObj->setNumberOfReadPoints(numberOfPointsRead);
    dataObj->setNumberOfDisplayPointsEstimate(numberOfPointsRead);
    dataObj->setNumberOfSavePointsEstimate(pieceInfo[pieceId]);
    dataObj->setFileName(filename);
    table->AddLIDARPiece(dataObj, visible);

    FilePieceIdMap[filename].insert(pieceId, dataObj);
    arcManager->addProxy(filename, pieceId, bbox, dataObj->getDiggerSource());
    // The following effect is like OnPreviewSelected();
    if (visible) // update the render window
    {
      this->Core->activeRenderView()->resetCamera();
      this->Core->activeRenderView()->render();
    }
    dataObj->useElevationFilter(showElevation);
  }

  bbox.GetBounds(this->CurrentReaderBounds);
  return 1;
}

//-----------------------------------------------------------------------------
int pqCMBLIDARReaderManager::importLASData(const char* filename, pqCMBLIDARPieceTable* table,
  pqCMBModifierArcManager* arcManager,
  QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >& FilePieceIdMap, bool showElevation)
{
  vtkSMSourceProxy* readerProxy = this->getReaderSourceProxy(filename);
  if (!readerProxy)
  {
    return 0;
  }
  if (this->CurrentLASPieces.size() == 0)
  {
    return 0;
  }

  if (ChangeOrigin)
  {
    vtkSMPropertyHelper(readerProxy, "Origin").Set(Origin, 3);
  }

  // 1, -1 used to indicate CurrentReaderBounds not yet set
  this->CurrentReaderBounds[0] = 1;
  this->CurrentReaderBounds[1] = -1;

  vtkIdType totalNumberOfPoints = 0;
  std::map<unsigned char, LASPieceInfo>::const_iterator piece;
  for (piece = this->CurrentLASPieces.begin(); piece != this->CurrentLASPieces.end(); piece++)
  {
    totalNumberOfPoints += piece->second.NumberOfPointsInClassification;
  }
  int onRatio[32], mainOnRatio = this->Core->calculateMainOnRatio(totalNumberOfPoints);

  QList<QVariant> pieceOnRatioList;
  for (piece = this->CurrentLASPieces.begin(); piece != this->CurrentLASPieces.end(); piece++)
  {
    onRatio[piece->first] = this->Core->calculateOnRatioForPiece(
      mainOnRatio, piece->second.NumberOfPointsInClassification);
    pieceOnRatioList << piece->first << onRatio[piece->first];
  }
  pqSMAdaptor::setElementProperty(readerProxy->GetProperty("ScanMode"), 0);

  QList<pqPipelineSource*> pdSources;
  this->readPieces(this->ReaderSourceMap[filename], pieceOnRatioList, pdSources);

  int aborted = pqSMAdaptor::getElementProperty(readerProxy->GetProperty("AbortExecute")).toInt();
  if (aborted)
  {
    return 0;
  }

  if (!pdSources.count())
  {
    QMessageBox::warning(
      this->Core->parentWidget(), "LAS File Reading", tr("Failed to load any data from LAS file!"));
    return 0;
  }

  vtkBoundingBox bbox;
  for (int i = 0; i < pdSources.count(); i++)
  {
    vtkNew<vtkPVLASOutputBlockInformation> info;
    pdSources[i]->getProxy()->GatherInformation(info.GetPointer());

    const char* classificationName = info->GetClassificationName();
    unsigned char classification = info->GetClassification();
    vtkIdType numberOfPointsInClassification = info->GetNumberOfPointsInClassification();
    vtkIdType numberOfPoints = info->GetNumberOfPoints();
    double* bounds = info->GetBounds();
    bbox.AddBounds(bounds);

    int visible = numberOfPoints > this->Core->getMinimumNumberOfPointsPerPiece() ? 1 : 0;

    pqCMBLIDARPieceObject* dataObj = pqCMBLIDARPieceObject::createObject(
      pdSources[i], bounds, this->Core->getActiveServer(), this->Core->activeRenderView(), visible);

    dataObj->setPieceIndex(classification);
    dataObj->setPieceName(classificationName);
    dataObj->setDisplayOnRatio(onRatio[classification]);
    dataObj->setReadOnRatio(onRatio[classification]);
    dataObj->setNumberOfPoints(numberOfPointsInClassification);
    dataObj->setNumberOfReadPoints(numberOfPoints);
    dataObj->setNumberOfDisplayPointsEstimate(numberOfPoints);
    dataObj->setNumberOfSavePointsEstimate(numberOfPointsInClassification);
    dataObj->setFileName(filename);
    table->AddLIDARPiece(dataObj, visible);

    FilePieceIdMap[filename].insert(i, dataObj);
    arcManager->addProxy(filename, i, bbox, dataObj->getDiggerSource());
    // The following effect is like OnPreviewSelected();
    if (visible) // update the render window
    {
      this->Core->activeRenderView()->resetCamera();
      this->Core->activeRenderView()->render();
    }
    dataObj->useElevationFilter(showElevation);
  }

  bbox.GetBounds(this->CurrentReaderBounds);

  return 1;
}
//-----------------------------------------------------------------------------
int pqCMBLIDARReaderManager::importDEMData(const char* filename, pqCMBLIDARPieceTable* table,
  pqCMBModifierArcManager* arcManager,
  QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >& FilePieceIdMap, bool showElevation)
{
  vtkSMSourceProxy* readerProxy = this->getReaderSourceProxy(filename);
  if (!readerProxy)
  {
    return 0;
  }

  if (ChangeOrigin)
  {
    vtkSMPropertyHelper(readerProxy, "Origin").Set(Origin, 3);
  }

  // 1, -1 used to indicate CurrentReaderBounds not yet set
  this->CurrentReaderBounds[0] = 1;
  this->CurrentReaderBounds[1] = -1;

  // For DEM reader, the output has to be polydata
  vtkSMPropertyHelper(readerProxy, "OutputImageData").Set(0);
  QList<QVariant> zorigin;
  zorigin << this->DEMTransformForZOrigin[0] << this->DEMTransformForZOrigin[1];
  pqSMAdaptor::setMultipleElementProperty(readerProxy->GetProperty("TransformForZOrigin"), zorigin);
  if (this->DEMZRotationAngle != 0)
  {
    pqSMAdaptor::setElementProperty(
      readerProxy->GetProperty("ZRotationAngle"), this->DEMZRotationAngle);
  }
  readerProxy->UpdateVTKObjects();
  readerProxy->UpdatePropertyInformation();
  int totalNumberOfPoints =
    pqSMAdaptor::getElementProperty(readerProxy->GetProperty("TotalNumberOfPoints")).toInt();

  int mainOnRatio = this->Core->calculateMainOnRatio(totalNumberOfPoints);

  this->Core->enableAbort(true); //make sure progress bar is enabled
  vtkBoundingBox bbox;
  QList<QVariant> pieceOnRatioList;
  pieceOnRatioList << 0 << mainOnRatio;

  QList<pqPipelineSource*> pdSource;
  this->readPieces(this->ReaderSourceMap[filename], pieceOnRatioList, pdSource);

  if (!pdSource.count() || !pdSource[0])
  {
    QMessageBox::critical(this->Core->parentWidget(), "DEM File Reading",
      tr("Failed to load DEM File: ").append(filename));
    return 0;
  }
  int visible = totalNumberOfPoints > this->Core->getMinimumNumberOfPointsPerPiece() ? 1 : 0;

  readerProxy->UpdatePropertyInformation();
  int numberOfPointsRead =
    pqSMAdaptor::getElementProperty(readerProxy->GetProperty("RealNumberOfOutputPoints")).toInt();
  QList<QVariant> values =
    pqSMAdaptor::getMultipleElementProperty(readerProxy->GetProperty("DataBounds"));
  double bounds[6] = { values[0].toDouble(), values[1].toDouble(), values[2].toDouble(),
    values[3].toDouble(), values[4].toDouble(), values[5].toDouble() };

  bbox.AddBounds(bounds);
  if (this->DEMZRotationAngle == 0)
  {
    this->DEMZRotationAngle =
      pqSMAdaptor::getElementProperty(readerProxy->GetProperty("GetZRotationAngle")).toDouble();
  }

  pqCMBLIDARPieceObject* dataObj = pqCMBLIDARPieceObject::createObject(
    pdSource[0], bounds, this->Core->getActiveServer(), this->Core->activeRenderView(), visible);

  dataObj->setPieceIndex(0);
  dataObj->setDisplayOnRatio(mainOnRatio);
  dataObj->setReadOnRatio(mainOnRatio);
  dataObj->setNumberOfPoints(totalNumberOfPoints);
  dataObj->setNumberOfReadPoints(numberOfPointsRead);
  dataObj->setNumberOfDisplayPointsEstimate(numberOfPointsRead);
  dataObj->setNumberOfSavePointsEstimate(totalNumberOfPoints);
  dataObj->setFileName(filename);
  table->AddLIDARPiece(dataObj, visible);

  FilePieceIdMap[filename].insert(0, dataObj);
  arcManager->addProxy(filename, 0, bbox, dataObj->getDiggerSource());
  // The following effect is like OnPreviewSelected();
  if (visible) // update the render window
  {
    this->Core->activeRenderView()->resetCamera();
    this->Core->activeRenderView()->render();
  }

  dataObj->useElevationFilter(showElevation);
  bbox.GetBounds(this->CurrentReaderBounds);
  return 1;
}

int pqCMBLIDARReaderManager::importGDALData(const char* filename, pqCMBLIDARPieceTable* table,
  pqCMBModifierArcManager* arcManager,
  QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >& FilePieceIdMap, bool showElevation)
{
  vtkSMSourceProxy* readerProxy = this->getReaderSourceProxy(filename);
  if (!readerProxy)
  {
    return 0;
  }

  if (ChangeOrigin)
  {
    vtkSMPropertyHelper(readerProxy, "Origin").Set(Origin, 3);
  }

  // 1, -1 used to indicate CurrentReaderBounds not yet set
  this->CurrentReaderBounds[0] = 1;
  this->CurrentReaderBounds[1] = -1;

  readerProxy->UpdateVTKObjects();
  readerProxy->UpdatePropertyInformation();
  int totalNumberOfPoints =
    pqSMAdaptor::getElementProperty(readerProxy->GetProperty("TotalNumberOfPoints")).toInt();

  int mainOnRatio = this->Core->calculateMainOnRatio(totalNumberOfPoints);

  this->Core->enableAbort(true); //make sure progress bar is enabled
  vtkBoundingBox bbox;
  QList<QVariant> pieceOnRatioList;
  pieceOnRatioList << 0 << mainOnRatio;

  QList<pqPipelineSource*> pdSource;
  this->readPieces(this->ReaderSourceMap[filename], pieceOnRatioList, pdSource);

  if (!pdSource.count() || !pdSource[0])
  {
    QMessageBox::critical(this->Core->parentWidget(), "GDAL DEM File Reading",
      tr("Failed to load DEM File: ").append(filename));
    return 0;
  }
  int visible = totalNumberOfPoints > this->Core->getMinimumNumberOfPointsPerPiece() ? 1 : 0;

  readerProxy->UpdatePropertyInformation();
  int numberOfPointsRead =
    pqSMAdaptor::getElementProperty(readerProxy->GetProperty("RealNumberOfOutputPoints")).toInt();
  QList<QVariant> values =
    pqSMAdaptor::getMultipleElementProperty(readerProxy->GetProperty("DataBounds"));
  double bounds[6] = { values[0].toDouble(), values[1].toDouble(), values[2].toDouble(),
    values[3].toDouble(), values[4].toDouble(), values[5].toDouble() };

  bbox.AddBounds(bounds);
  pqCMBLIDARPieceObject* dataObj = pqCMBLIDARPieceObject::createObject(
    pdSource[0], bounds, this->Core->getActiveServer(), this->Core->activeRenderView(), visible);

  dataObj->setPieceIndex(0);
  dataObj->setDisplayOnRatio(mainOnRatio);
  dataObj->setReadOnRatio(mainOnRatio);
  dataObj->setNumberOfPoints(totalNumberOfPoints);
  dataObj->setNumberOfReadPoints(numberOfPointsRead);
  dataObj->setNumberOfDisplayPointsEstimate(numberOfPointsRead);
  dataObj->setNumberOfSavePointsEstimate(totalNumberOfPoints);
  dataObj->setFileName(filename);
  table->AddLIDARPiece(dataObj, visible);

  FilePieceIdMap[filename].insert(0, dataObj);
  arcManager->addProxy(filename, 0, bbox, dataObj->getDiggerSource());
  // The following effect is like OnPreviewSelected();
  if (visible) // update the render window
  {
    this->Core->activeRenderView()->resetCamera();
    this->Core->activeRenderView()->render();
  }

  dataObj->useElevationFilter(showElevation);
  bbox.GetBounds(this->CurrentReaderBounds);
  return 1;
}

int pqCMBLIDARReaderManager::importVTPData(const char* filename, pqCMBLIDARPieceTable* table,
  pqCMBModifierArcManager* arcManager,
  QMap<QString, QMap<int, pqCMBLIDARPieceObject*> >& FilePieceIdMap, bool showElevation)
{
  vtkSMSourceProxy* readerProxy = this->getReaderSourceProxy(filename);
  if (!readerProxy)
  {
    return 0;
  }

  QMessageBox::warning(this->Core->parentWidget(), tr("Load VTP File Warning"),
    tr("Currently all points in vtp dataset will be loaded in, "
       "and they will NOT be filtered by number of target points or ROI!"),
    QMessageBox::Ok);

  // 1, -1 used to indicate CurrentReaderBounds not yet set
  this->CurrentReaderBounds[0] = 1;
  this->CurrentReaderBounds[1] = -1;

  readerProxy->UpdateVTKObjects();
  readerProxy->UpdatePropertyInformation();
  vtkPVDataInformation* dataInfo =
    this->ReaderSourceMap[filename]->getOutputPort(0)->getDataInformation();
  int totalNumberOfPoints = dataInfo->GetNumberOfPoints();
  int mainOnRatio = 1;

  this->Core->enableAbort(true); //make sure progress bar is enabled
  vtkBoundingBox bbox;
  QList<QVariant> pieceOnRatioList;
  pieceOnRatioList << 0 << mainOnRatio;

  QList<pqPipelineSource*> pdSource;
  this->readPieces(this->ReaderSourceMap[filename], pieceOnRatioList, pdSource);

  if (!pdSource.count() || !pdSource[0])
  {
    QMessageBox::critical(this->Core->parentWidget(), "VTP File Reading",
      tr("Failed to load VTP File: ").append(filename));
    return 0;
  }
  int visible = totalNumberOfPoints > this->Core->getMinimumNumberOfPointsPerPiece() ? 1 : 0;

  readerProxy->UpdatePropertyInformation();
  dataInfo = pdSource[0]->getOutputPort(0)->getDataInformation();
  int numberOfPointsRead = dataInfo->GetNumberOfPoints();

  // numberOfPointsRead should be the same as totalNumberOfPoints;
  double bounds[6];
  dataInfo->GetBounds(bounds);

  bbox.AddBounds(bounds);
  pqCMBLIDARPieceObject* dataObj = pqCMBLIDARPieceObject::createObject(
    pdSource[0], bounds, this->Core->getActiveServer(), this->Core->activeRenderView(), visible);

  dataObj->setPieceIndex(0);
  dataObj->setDisplayOnRatio(mainOnRatio);
  dataObj->setReadOnRatio(mainOnRatio);
  dataObj->setNumberOfPoints(totalNumberOfPoints);
  dataObj->setNumberOfReadPoints(numberOfPointsRead);
  dataObj->setNumberOfDisplayPointsEstimate(numberOfPointsRead);
  dataObj->setNumberOfSavePointsEstimate(totalNumberOfPoints);
  dataObj->setFileName(filename);
  table->AddLIDARPiece(dataObj, visible);

  FilePieceIdMap[filename].insert(0, dataObj);
  arcManager->addProxy(filename, 0, bbox, dataObj->getDiggerSource());
  // The following effect is like OnPreviewSelected();
  if (visible) // update the render window
  {
    this->Core->activeRenderView()->resetCamera();
    this->Core->activeRenderView()->render();
  }

  bbox.GetBounds(this->CurrentReaderBounds);
  dataObj->useElevationFilter(showElevation);
  return 1;
}

//-----------------------------------------------------------------------------
vtkIdType pqCMBLIDARReaderManager::getPieceNumPointsInfo(
  const char* filename, QList<vtkIdType>& pieceInfo)
{
  vtkSMSourceProxy* readerProxy = this->getReaderSourceProxy(filename);
  if (!readerProxy)
  {
    return 0;
  }

  readerProxy->UpdatePropertyInformation();

  int numberOfPieces =
    pqSMAdaptor::getElementProperty(readerProxy->GetProperty("KnownNumberOfPieces")).toInt();

  vtkIdType numberOfPointsInPiece, totalNumberOfPoints = 0;
  for (int i = 0; i < numberOfPieces; i++)
  {
    pqSMAdaptor::setElementProperty(readerProxy->GetProperty("PieceIndex"), i);
    readerProxy->UpdateVTKObjects();
    readerProxy->UpdatePropertyInformation();

    numberOfPointsInPiece =
      pqSMAdaptor::getElementProperty(readerProxy->GetProperty("NumberOfPointsInPiece")).toInt();
    totalNumberOfPoints += numberOfPointsInPiece;
    pieceInfo.push_back(numberOfPointsInPiece);
  }

  return totalNumberOfPoints;
}

//-----------------------------------------------------------------------------
void pqCMBLIDARReaderManager::readPieces(
  pqPipelineSource* reader, QList<QVariant>& pieces, QList<pqPipelineSource*>& pdSources)
{
  vtkSMSourceProxy* readerProxy = vtkSMSourceProxy::SafeDownCast(reader->getProxy());
  if (!readerProxy || pieces.count() == 0)
  {
    return;
  }

  this->readData(readerProxy, pieces);
  int aborted = readerProxy->GetProperty("AbortExecute")
    ? pqSMAdaptor::getElementProperty(readerProxy->GetProperty("AbortExecute")).toInt()
    : 0;
  if (aborted)
  {
    return;
  }

  QString readerName(readerProxy->GetXMLName());
  if (readerName.compare(SM_LIDAR_READER_NAME) == 0 ||
    readerName.compare(SM_DEM_READER_NAME) == 0 || readerName.compare(SM_GDAL_READER_NAME) == 0 ||
    readerName.compare(SM_VTP_READER_NAME) == 0 ||
    pqPluginIOBehavior::isPluginReader(readerProxy->GetHints()))
  {
    pqPipelineSource* pdSource =
      this->Builder->createSource("sources", "HydroModelPolySource", this->Core->getActiveServer());
    vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(readerProxy);
    vtkSMSourceProxy::SafeDownCast(pdSource->getProxy())->UpdatePipeline();
    pdSources.push_back(pdSource);
  }
  else if (readerName.compare(SM_LAS_READER_NAME) == 0)
  {
    // create a source for each block
    vtkSMOutputPort* outputPort = readerProxy->GetOutputPort(static_cast<unsigned int>(0));
    vtkPVCompositeDataInformation* compositeInformation =
      outputPort->GetDataInformation()->GetCompositeDataInformation();
    int numBlocks = compositeInformation->GetNumberOfChildren();

    QString classifcationName;
    for (int i = 0; i < numBlocks; i++)
    {
      pqPipelineSource* extract =
        this->Builder->createFilter("filters", "ExtractLeafBlock", reader);

      pqSMAdaptor::setElementProperty(extract->getProxy()->GetProperty("BlockIndex"), i);
      extract->getProxy()->UpdateVTKObjects();
      vtkSMSourceProxy::SafeDownCast(extract->getProxy())->UpdatePipeline();

      pqPipelineSource* pdSource = this->Builder->createSource(
        "sources", "HydroModelPolySource", this->Core->getActiveServer());
      vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())
        ->CopyData(vtkSMSourceProxy::SafeDownCast(extract->getProxy()));
      pdSource->updatePipeline();
      this->Builder->destroy(extract);

      pdSources.push_back(pdSource);
    }
  }
}

//-----------------------------------------------------------------------------
int pqCMBLIDARReaderManager::readData(vtkSMSourceProxy* readerProxy, QList<QVariant>& pieces)
{
  if (!readerProxy)
  {
    return -1;
  }
  this->ActiveReader = readerProxy;
  int numberOfPointsRead = 0;
  if (readerProxy->GetProperty("AbortExecute"))
  {
    pqSMAdaptor::setElementProperty(readerProxy->GetProperty("AbortExecute"), 0);
  }
  this->Core->enableAbort(true); //make sure progressbar is enabled
  QString readerName(readerProxy->GetXMLName());
  if (readerName.compare(SM_LIDAR_READER_NAME) == 0)
  {
    pqSMAdaptor::setMultipleElementProperty(
      readerProxy->GetProperty("RequestedPiecesForRead"), pieces);
    readerProxy->UpdateVTKObjects();
    readerProxy->UpdatePipeline();

    readerProxy->UpdatePropertyInformation();
    numberOfPointsRead =
      pqSMAdaptor::getElementProperty(readerProxy->GetProperty("RealNumberOfOutputPoints")).toInt();
  }
  else if (readerName.compare(SM_LAS_READER_NAME) == 0)
  {
    pqSMAdaptor::setMultipleElementProperty(
      readerProxy->GetProperty("RequestedClassificationsForRead"), pieces);
    readerProxy->UpdateVTKObjects();
    readerProxy->UpdatePipeline();
  }
  else if (readerName.compare(SM_DEM_READER_NAME) == 0)
  {
    vtkSMPropertyHelper(readerProxy, "OnRatio").Set(pieces[1].toInt());
    readerProxy->UpdateVTKObjects();
    readerProxy->UpdatePipeline();
    readerProxy->UpdatePropertyInformation();
    numberOfPointsRead =
      pqSMAdaptor::getElementProperty(readerProxy->GetProperty("RealNumberOfOutputPoints")).toInt();
  }
  else if (readerName.compare(SM_GDAL_READER_NAME) == 0)
  {
    vtkSMPropertyHelper(readerProxy, "OnRatio").Set(pieces[1].toInt());
    readerProxy->UpdateVTKObjects();
    readerProxy->UpdatePipeline();
    readerProxy->UpdatePropertyInformation();
    numberOfPointsRead =
      pqSMAdaptor::getElementProperty(readerProxy->GetProperty("RealNumberOfOutputPoints")).toInt();
  }
  else if (readerName.compare(SM_VTP_READER_NAME) == 0)
  {
    readerProxy->UpdateVTKObjects();
    readerProxy->UpdatePipeline();
    readerProxy->UpdatePropertyInformation();
    std::string filename = vtksys::SystemTools::CollapseFullPath(
      pqSMAdaptor::getElementProperty(readerProxy->GetProperty("FileName"))
        .toString()
        .toStdString());

    vtkPVDataInformation* dataInfo =
      this->ReaderSourceMap[filename.c_str()]->getOutputPort(0)->getDataInformation();
    numberOfPointsRead = dataInfo->GetNumberOfPoints();
  }
  else if (pqPluginIOBehavior::isPluginReader(
             readerProxy->GetHints()) && // assume plugin readers have these
    readerProxy->GetProperty("RequestedPiecesForRead") &&
    readerProxy->GetProperty("RealNumberOfOutputPoints"))
  {
    pqSMAdaptor::setMultipleElementProperty(
      readerProxy->GetProperty("RequestedPiecesForRead"), pieces);
    readerProxy->UpdateVTKObjects();
    readerProxy->UpdatePipeline();

    readerProxy->UpdatePropertyInformation();
    numberOfPointsRead =
      pqSMAdaptor::getElementProperty(readerProxy->GetProperty("RealNumberOfOutputPoints")).toInt();
  }
  this->ActiveReader = NULL;
  return numberOfPointsRead; // not valid value for LASReader
}

//-----------------------------------------------------------------------------
void pqCMBLIDARReaderManager::setReaderTransform(
  vtkSMSourceProxy* readerProxy, pqCMBLIDARPieceObject* dataObj)
{
  if (!readerProxy)
  {
    return;
  }

  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  QString readerName(readerProxy->GetXMLName());
  if (readerName.compare(SM_LIDAR_READER_NAME) == 0 ||
    readerName.compare(SM_DEM_READER_NAME) == 0 || readerName.compare(SM_GDAL_READER_NAME) == 0 ||
    (pqPluginIOBehavior::isPluginReader(
       readerProxy->GetHints()) && // assume plugin readers have these
        readerProxy->GetProperty("Transform")))
  {
    dataObj->getTransform(transform);
    vtkSMPropertyHelper(readerProxy, "Transform").Set(transform->GetMatrix()->Element[0], 16);
    readerProxy->UpdateProperty("Transform");
  }
  else if (readerName.compare(SM_LAS_READER_NAME) == 0)
  {
    dataObj->getTransform(transform);
    double matrix[17];
    memcpy(matrix + 1, transform->GetMatrix()->Element[0], 16 * sizeof(double));
    matrix[0] = dataObj->getPieceIndex();
    vtkSMPropertyHelper(readerProxy, "Transform").Set(matrix, 17);
    readerProxy->UpdateProperty("Transform");
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARReaderManager::setReaderTransformData(
  vtkSMSourceProxy* readerProxy, bool transformData)
{
  if (!readerProxy)
  {
    return;
  }

  QString readerName(readerProxy->GetXMLName());
  if (readerName.compare(SM_LIDAR_READER_NAME) == 0 ||
    readerName.compare(SM_LAS_READER_NAME) == 0 || readerName.compare(SM_DEM_READER_NAME) == 0 ||
    readerName.compare(SM_GDAL_READER_NAME) == 0 || readerProxy->GetProperty("Transform"))
  {
    vtkSMPropertyHelper(readerProxy, "TransformOutputData").Set(transformData);
    readerProxy->UpdateProperty("TransformOutputData");
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARReaderManager::clearTransform(vtkSMSourceProxy* readerProxy)
{
  if (!readerProxy)
  {
    return;
  }
  QString readerName = readerProxy->GetXMLName();
  if (readerName.compare(SM_LIDAR_READER_NAME) == 0 ||
    readerName.compare(SM_DEM_READER_NAME) == 0 || readerName.compare(SM_GDAL_READER_NAME) == 0 ||
    (pqPluginIOBehavior::isPluginReader(
       readerProxy->GetHints()) && // assume plugin readers have these
        readerProxy->GetProperty("ClearTransform")))
  {
    readerProxy->InvokeCommand("ClearTransform");
  }
  else if (readerName.compare(SM_LAS_READER_NAME) == 0)
  {
    readerProxy->InvokeCommand("ClearTransforms");
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARReaderManager::updatePieces(const char* filename,
  QList<pqCMBLIDARPieceObject*>& pieces, bool forceRead, pqCMBLIDARPieceTable* table, bool clipData,
  double* clipBounds)
{
  vtkSMSourceProxy* readerProxy = this->getReaderSourceProxy(filename);
  if (!readerProxy)
  {
    return;
  }
  QString readerName = readerProxy->GetXMLName();
  if (readerName.compare(SM_LIDAR_READER_NAME) == 0 ||
    readerName.compare(SM_DEM_READER_NAME) == 0 || readerName.compare(SM_GDAL_READER_NAME) == 0 ||
    pqPluginIOBehavior::isPluginReader(readerProxy->GetHints()))
  {
    this->updateLIDARPieces(filename, pieces, forceRead, table, clipData, clipBounds);
  }
  else if (readerName.compare(SM_LAS_READER_NAME) == 0)
  {
    this->updateLASPieces(filename, pieces, forceRead, table, clipData, clipBounds);
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARReaderManager::updateLIDARPieces(const char* filename,
  QList<pqCMBLIDARPieceObject*>& pieces, bool forceRead, pqCMBLIDARPieceTable* table, bool clipData,
  double* clipBounds)
{
  vtkSMSourceProxy* readerProxy = this->getReaderSourceProxy(filename);
  if (!readerProxy)
  {
    return;
  }

  for (int i = 0; i < pieces.count(); i++)
  {
    pqCMBLIDARPieceObject* dataObj = pieces[i];
    if (forceRead || !this->Core->isObjectUpToDate(dataObj))
    {
      QList<QVariant> pieceOnRatioList;
      pieceOnRatioList << dataObj->getPieceIndex() << dataObj->getDisplayOnRatio();
      // only need to do transformation in the reader if we're clipping the data
      if (dataObj->isPieceTransformed() && clipData)
      {
        this->setReaderTransform(readerProxy, dataObj);
        // don't want to transform the data for output from the reader, but do
        // want to transform the to see if in the ReadBounds
        this->setReaderTransformData(readerProxy, false);
      }
      else
      {
        this->clearTransform(readerProxy);
      }

      int numberOfPointsRead = this->readData(readerProxy, pieceOnRatioList);
      int aborted = readerProxy->GetProperty("AbortExecute")
        ? pqSMAdaptor::getElementProperty(readerProxy->GetProperty("AbortExecute")).toInt()
        : 0;
      if (aborted)
      {
        return;
      }
      dataObj->setReadOnRatio(dataObj->getDisplayOnRatio());
      if (clipData)
      {
        dataObj->setClipBounds(clipBounds);
        dataObj->setClipState(true);
        dataObj->saveClipPosition();
        dataObj->saveClipOrientation();
        dataObj->saveClipScale();
        dataObj->saveClipOrigin();
      }
      else
      {
        dataObj->setClipState(false);
      }

      dataObj->setNumberOfReadPoints(numberOfPointsRead);

      vtkSMDataSourceProxy* pdSource =
        vtkSMDataSourceProxy::SafeDownCast(dataObj->getSource()->getProxy());
      pdSource->CopyData(readerProxy);
      pdSource->UpdatePipeline();

      dataObj->updateRepresentation();

      table->computeDisplayNumberOfPointsEstimate(dataObj);
      table->computeSaveNumberOfPointsEstimate(dataObj);
      table->updateWithPieceInfo(dataObj);
    }
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARReaderManager::updateLASPieces(const char* filename,
  QList<pqCMBLIDARPieceObject*>& pieces, bool forceRead, pqCMBLIDARPieceTable* table, bool clipData,
  double* clipBounds)
{
  vtkSMSourceProxy* readerProxy = this->getReaderSourceProxy(filename);
  if (!readerProxy)
  {
    return;
  }

  // 1st clear any transforms, and setup so that we are NOT transforming
  // the output data of the reader
  this->clearTransform(readerProxy);
  // don't want to transform the data for output from the reader, but do
  // want to transform the to see if in the ReadBounds
  this->setReaderTransformData(readerProxy, false);

  // now cycle through pieces, setting up read ratios and transforms
  QList<QVariant> pieceOnRatioList;
  for (int i = 0; i < pieces.count(); i++)
  {
    pqCMBLIDARPieceObject* dataObj = pieces[i];
    if (forceRead || !this->Core->isObjectUpToDate(dataObj))
    {
      pieceOnRatioList << dataObj->getPieceIndex() << dataObj->getDisplayOnRatio();
      // only need to do transformation in the reader if we're clipping the data
      if (dataObj->isPieceTransformed() && clipData)
      {
        this->setReaderTransform(readerProxy, dataObj);
      }
    }
  }

  this->readData(readerProxy, pieceOnRatioList);
  int aborted = pqSMAdaptor::getElementProperty(readerProxy->GetProperty("AbortExecute")).toInt();
  if (aborted)
  {
    return;
  }

  // create a source for each block
  vtkSMOutputPort* outputPort = readerProxy->GetOutputPort(static_cast<unsigned int>(0));
  vtkPVCompositeDataInformation* compositeInformation =
    outputPort->GetDataInformation()->GetCompositeDataInformation();
  int numBlocks = compositeInformation->GetNumberOfChildren();

  for (int i = 0; i < numBlocks; i++)
  {
    pqPipelineSource* extract =
      this->Builder->createFilter("filters", "ExtractLeafBlock", this->ReaderSourceMap[filename]);

    pqSMAdaptor::setElementProperty(extract->getProxy()->GetProperty("BlockIndex"), i);
    extract->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy::SafeDownCast(extract->getProxy())->UpdatePipeline();

    vtkNew<vtkPVLASOutputBlockInformation> info;
    extract->getProxy()->GatherInformation(info.GetPointer());

    // which dataObj does this block belong to
    // classification == PieceIndex
    unsigned char classification = info->GetClassification();
    vtkIdType numberOfPointsRead = info->GetNumberOfPoints();
    pqCMBLIDARPieceObject* dataObj = NULL;
    for (int j = 0; j < pieces.count(); j++)
    {
      if (pieces[j]->getPieceIndex() == classification)
      {
        dataObj = pieces[j];
        break;
      }
    }
    if (dataObj == NULL)
    {
      QMessageBox::warning(this->Core->parentWidget(), "No Classification equaled",
        tr("We did not find classification"));
      return;
    }

    dataObj->setReadOnRatio(dataObj->getDisplayOnRatio());
    if (clipData)
    {
      dataObj->setClipBounds(clipBounds);
      dataObj->setClipState(true);
      dataObj->saveClipPosition();
      dataObj->saveClipOrientation();
      dataObj->saveClipScale();
      dataObj->saveClipOrigin();
    }
    else
    {
      dataObj->setClipState(false);
    }

    dataObj->setNumberOfReadPoints(numberOfPointsRead);

    vtkSMDataSourceProxy* pdSource =
      vtkSMDataSourceProxy::SafeDownCast(dataObj->getSource()->getProxy());
    pdSource->CopyData(vtkSMSourceProxy::SafeDownCast(extract->getProxy()));
    pdSource->UpdateVTKObjects();
    pdSource->UpdatePipeline();
    this->Builder->destroy(extract);

    dataObj->updateRepresentation();

    table->computeDisplayNumberOfPointsEstimate(dataObj);
    table->computeSaveNumberOfPointsEstimate(dataObj);
    table->updateWithPieceInfo(dataObj);
  }
}
//-----------------------------------------------------------------------------
QList<pqCMBLIDARPieceObject*> pqCMBLIDARReaderManager::getFilePieceObjects(
  const char* filename, QList<pqCMBLIDARPieceObject*>& sourcePieces)
{
  QList<pqCMBLIDARPieceObject*> selObjects;
  QString selFile(filename);
  foreach (pqCMBLIDARPieceObject* dataObj, sourcePieces)
  {
    if (dataObj && selFile.compare(dataObj->getFileName().c_str()) == 0)
    {
      selObjects.push_back(dataObj);
    }
  }
  return selObjects;
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARReaderManager::getSourcesForOutput(bool atDisplayRatio,
  QList<pqCMBLIDARPieceObject*>& pieces, QList<pqPipelineSource*>& outputSources,
  bool forceUpdate /*= false*/)
{
  if (!this->ReaderSourceMap.count())
  {
    return false;
  }

  bool result = false;
  vtkSMSourceProxy* readerProxy;
  foreach (QString filename, this->ReaderSourceMap.uniqueKeys())
  {
    QList<pqCMBLIDARPieceObject*> selObjects =
      this->getFilePieceObjects(filename.toLatin1().constData(), pieces);

    readerProxy = vtkSMSourceProxy::SafeDownCast(this->ReaderSourceMap[filename]->getProxy());
    QString readerName = readerProxy->GetXMLName();
    if (readerName.compare(SM_LIDAR_READER_NAME) == 0 ||
      readerName.compare(SM_DEM_READER_NAME) == 0 || readerName.compare(SM_GDAL_READER_NAME) == 0 ||
      readerName.compare(SM_VTP_READER_NAME) == 0 ||
      pqPluginIOBehavior::isPluginReader(readerProxy->GetHints()))
    {
      result = this->getSourcesForOutputLIDAR(
        this->ReaderSourceMap[filename], atDisplayRatio, selObjects, outputSources, forceUpdate);
    }
    else if (readerName.compare(SM_LAS_READER_NAME) == 0)
    {
      result = this->getSourcesForOutputLAS(
        this->ReaderSourceMap[filename], atDisplayRatio, selObjects, outputSources, forceUpdate);
    }
  }

  if (!result)
  {
    this->destroyTemporarySources();
  }
  return result;
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARReaderManager::getSourcesForOutputLIDAR(pqPipelineSource* reader,
  bool atDisplayRatio, QList<pqCMBLIDARPieceObject*>& pieces,
  QList<pqPipelineSource*>& outputSources, bool forceUpdate)
{
  vtkSMSourceProxy* readerProxy = vtkSMSourceProxy::SafeDownCast(reader->getProxy());
  if (!readerProxy)
  {
    return false;
  }
  for (int i = 0; i < pieces.count(); i++)
  {
    pqCMBLIDARPieceObject* dataObj = pieces[i];

    bool pieceTransformed = false;
    if (dataObj->isPieceTransformed())
    {
      // want to transform the data for output from the reader
      this->setReaderTransform(readerProxy, dataObj);
      this->setReaderTransformData(readerProxy, true);
      pieceTransformed = true;
    }

    int onRatio = atDisplayRatio ? dataObj->getDisplayOnRatio() : dataObj->getSaveOnRatio();

    if (pieceTransformed || dataObj->getReadOnRatio() != onRatio)
    {
      // read the piece again with new onRatio
      QList<QVariant> pieceOnRatioList;
      pieceOnRatioList << dataObj->getPieceIndex() << onRatio;
      QList<pqPipelineSource*> tempSource;
      this->readPieces(reader, pieceOnRatioList, tempSource);
      if (!tempSource.count() || !tempSource[0])
      {
        return false; // read must have been cancelled
      }

      this->TemporaryPDSources.push_back(tempSource[0]);

      pqPipelineSource* contourSource =
        this->Builder->createFilter("filters", "ClipPolygons", tempSource[0]);
      this->TemporaryContourFilters.push_back(contourSource);
      this->Core->updateContourSource(vtkSMSourceProxy::SafeDownCast(contourSource->getProxy()),
        vtkSMSourceProxy::SafeDownCast(dataObj->getContourSource()->getProxy()), forceUpdate);

      pqPipelineSource* thresholdSource =
        this->Builder->createFilter("filters", "PointThresholdFilter", contourSource);
      this->TemporaryThresholdFilters.push_back(thresholdSource);

      this->Core->updateThresholdSource(vtkSMSourceProxy::SafeDownCast(thresholdSource->getProxy()),
        vtkSMSourceProxy::SafeDownCast(dataObj->getThresholdSource()->getProxy()), forceUpdate);
      outputSources.push_back(thresholdSource);
    }
    else
    {
      outputSources.push_back(dataObj->getThresholdSource());
    }
  }

  return true;
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARReaderManager::getSourcesForOutputLAS(pqPipelineSource* reader, bool atDisplayRatio,
  QList<pqCMBLIDARPieceObject*>& pieces, QList<pqPipelineSource*>& outputSources, bool forceUpdate)
{
  vtkSMSourceProxy* readerProxy = vtkSMSourceProxy::SafeDownCast(reader->getProxy());
  if (!readerProxy)
  {
    return false;
  }
  // clear any transforms
  this->clearTransform(readerProxy);
  // set though that any transformed pieces will be transformed on reader output
  this->setReaderTransformData(readerProxy, true);

  // now cycle through pieces, setting up read ratios and transforms
  QList<QVariant> pieceOnRatioList;
  for (int i = 0; i < pieces.count(); i++)
  {
    pqCMBLIDARPieceObject* dataObj = pieces[i];
    bool pieceTransformed = false;
    if (dataObj->isPieceTransformed())
    {
      // want to transform the data for output from the reader
      this->setReaderTransform(readerProxy, dataObj);
      pieceTransformed = true;
    }

    int onRatio = atDisplayRatio ? dataObj->getDisplayOnRatio() : dataObj->getSaveOnRatio();

    if (pieceTransformed || dataObj->getReadOnRatio() != onRatio)
    {
      pieceOnRatioList << dataObj->getPieceIndex() << onRatio;
    }
    else
    {
      outputSources.push_back(dataObj->getThresholdSource());
    }
  }

  if (pieceOnRatioList.count() > 0)
  {
    QList<pqPipelineSource*> pdSources;
    this->readPieces(reader, pieceOnRatioList, pdSources);

    int aborted = pqSMAdaptor::getElementProperty(readerProxy->GetProperty("AbortExecute")).toInt();
    if (aborted)
    {
      return false;
    }

    if (!pdSources.count())
    {
      QMessageBox::warning(this->Core->parentWidget(), "LAS File Reading",
        tr("Failed to load any data from LAS file!"));
      return false;
    }

    QList<pqPipelineSource*>::iterator pdSource;
    for (pdSource = pdSources.begin(); pdSource != pdSources.end(); pdSource++)
    {
      this->TemporaryPDSources.push_back(*pdSource);

      vtkNew<vtkPVLASOutputBlockInformation> info;
      (*pdSource)->getProxy()->GatherInformation(info.GetPointer());

      // which dataObj does this block belong to
      // classification == PieceIndex
      unsigned char classification = info->GetClassification();
      pqCMBLIDARPieceObject* dataObj = 0;
      for (int i = 0; i < pieces.count(); i++)
      {
        if (pieces[i]->getPieceIndex() == classification)
        {
          dataObj = pieces[i];
          break;
        }
      }

      if (!dataObj)
      {
        QMessageBox::warning(
          this->Core->parentWidget(), "LAS File Reading Error", tr("Returned unexpected data!"));
        return false;
      }

      pqPipelineSource* contourSource =
        this->Builder->createFilter("filters", "ClipPolygons", *pdSource);
      this->TemporaryContourFilters.push_back(contourSource);
      this->Core->updateContourSource(vtkSMSourceProxy::SafeDownCast(contourSource->getProxy()),
        vtkSMSourceProxy::SafeDownCast(dataObj->getContourSource()->getProxy()), forceUpdate);

      pqPipelineSource* thresholdSource =
        this->Builder->createFilter("filters", "PointThresholdFilter", contourSource);
      this->TemporaryThresholdFilters.push_back(thresholdSource);

      this->Core->updateThresholdSource(vtkSMSourceProxy::SafeDownCast(thresholdSource->getProxy()),
        vtkSMSourceProxy::SafeDownCast(dataObj->getThresholdSource()->getProxy()), forceUpdate);
      outputSources.push_back(thresholdSource);
    }
  }
  return true;
}
//-----------------------------------------------------------------------------
void pqCMBLIDARReaderManager::destroyAllReaders()
{
  if (this->ReaderSourceMap.count() > 0)
  {
    foreach (QString filename, this->ReaderSourceMap.uniqueKeys())
    {
      this->Builder->destroy(this->ReaderSourceMap[filename]);
    }
  }
  this->ChangeOrigin = false;
  this->ReaderSourceMap.clear();
  this->ActiveReader = NULL;
  this->DEMTransformForZOrigin[0] = this->DEMTransformForZOrigin[1] = 90.0;
  this->DEMZRotationAngle = 0.0;
}
//-----------------------------------------------------------------------------
void pqCMBLIDARReaderManager::destroyTemporarySources()
{
  // destroy threshold filters 1st, as it is consumer of Contour filter
  for (int i = 0; i < this->TemporaryThresholdFilters.count(); i++)
  {
    this->Builder->destroy(this->TemporaryThresholdFilters[i]);
  }
  this->TemporaryThresholdFilters.clear();

  // then contour filters, as it is consumer of PDSources
  for (int i = 0; i < this->TemporaryContourFilters.count(); i++)
  {
    this->Builder->destroy(this->TemporaryContourFilters[i]);
  }
  this->TemporaryContourFilters.clear();

  // destroy the sources last
  for (int i = 0; i < this->TemporaryPDSources.count(); i++)
  {
    this->Builder->destroy(this->TemporaryPDSources[i]);
  }
  this->TemporaryPDSources.clear();
}

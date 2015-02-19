// .NAME pqCMBFileExtensions -
// .SECTION Description - File extensions for cmb applications
//

#ifndef __pqCMBFileExtensions_h
#define __pqCMBFileExtensions_h

#include "cmbSystemConfig.h"

#include "vtkSMProxyManager.h"
#include "vtkSMProxy.h"
#include "vtkSMSession.h"
#include "vtkSMSessionProxyManager.h"

#include <QString>
#include <QMap>
#include <QPair>

//BTX
namespace pqCMBFileExtensions
{

  typedef QMap<QString, QPair<QString, QString> > cmb_FileExtMap;
  // ModelBuilder file types
  static QString ModelBuilder_FileTypes()
  {
    QString filters;
    vtkSMProxyManager* proxyManager = vtkSMProxyManager::GetProxyManager();
    // when we change the server we may not have a session yet. that's ok
    // since we'll come back here after the proxy definitions are loaded
    // from that session.
    if(vtkSMSession* session = proxyManager->GetActiveSession())
      {
      vtkSMSessionProxyManager* pxm = session->GetSessionProxyManager();

      bool haveMoab = (pxm && pxm->HasDefinition("sources","CmbMoabSolidReader"));
/*
      filters = "Supported files (*.cmb *.crf *.vtk *.vtu *.vtp *.2dm *.3dm *.sol *.stl *.tin *.obj *.sbt *.sbi *.sbs *.map *.poly *.smesh *.shp *.h5m *.sat *.brep *.stp *.cub);;";
  //    filters += "CMB files (*.cmb);;";
      filters += "CMB Resource files (*.crf);;";
  
      filters += "VTK data files (*.vtk *.vtu *.vtp);;";
      filters += "Solids (*.2dm *.3dm *.sol *.stl *.tin *.obj);;";
      filters += "SimBuilder files (*.crf *.sbt *.sbi *.sbs);;";
      filters += "Map files (*.map);;";
      filters += "Poly files (*.poly *.smesh);;";
      filters += "Shape files (*.shp);;";
  */
      if(haveMoab)
        {
        vtkSMProxy* moabProxy = pxm->GetProxy("sources", "CmbMoabSolidReader");
        filters += "Moab files (*.h5m *.sat *.brep *.stp *.cub *.exo);;";
        }
      filters += "All files (*)";
      }
    return filters;
  }

  // SceneBuilder file types
  static QString SceneBuilder_FileTypes()
  {
    return "SceneGen (*.sg);;OSDLSceneGen (*.osd.txt);;Map (*.map)";
  }

  // PointsBuilder file types
  static QString PointsBuilder_FileTypes()
  {
    return "LIDAR (*.pts *.bin *.bin.pts);;LAS (*.las);;DEM (*.hdr *.FLT *.ftw);;All files (*)";
  }

  // MeshViewer file types
  static QString MeshViewer_FileTypes()
  {
    return "CMB 2D Mesh Files (*.2dm);;CMB 3D Mesh Files (*.3dm);;GAMBIT Mesh Files (*.neu);;VTK legacy data (*.vtk)";
  }

  // GeologyBuilder file types
  static QString GeologyBuilder_FileTypes()
  {
    return "Boreholes (*.bor);;SceneGen (*.sg);;OSDLSceneGen (*.osd.txt);;Map (*.map)";
  }

  // We need this because in CMB_Plugin, the CMBGeometryReader is combining
  // extensions of other readers, "2dm 3dm bin bin.pts fac obj poly smesh pts sol stl tin",
  // so pqLoadDataReaction::loadData() will popup
  // the select readers dialog since it detects all available readers, which we are trying to avoid
  // This map only need to contain those that are repeated in the SM xml configure file
  // <file_extension, <reader_group, reader_name> >
  static cmb_FileExtMap ModelBuilder_ReadersMap()
  {
    cmb_FileExtMap readerMap;
    readerMap.insert("crf", QPair<QString, QString>("sources", "StringReader"));
    readerMap.insert("sbt", QPair<QString, QString>("sources", "StringReader"));
    readerMap.insert("sbs", QPair<QString, QString>("sources", "StringReader"));
    readerMap.insert("sbi", QPair<QString, QString>("sources", "StringReader"));
    readerMap.insert("simb.xml", QPair<QString, QString>("sources", "StringReader"));

    readerMap.insert("2dm", QPair<QString, QString>("sources", "CMBGeometryReader"));
    readerMap.insert("3dm", QPair<QString, QString>("sources", "CMBGeometryReader"));
    readerMap.insert("sol", QPair<QString, QString>("sources", "CMBGeometryReader"));
    readerMap.insert("vtk", QPair<QString, QString>("sources", "CMBGeometryReader"));
    readerMap.insert("vtp", QPair<QString, QString>("sources", "CMBGeometryReader"));
    readerMap.insert("poly", QPair<QString, QString>("sources", "CMBGeometryReader"));
    readerMap.insert("smesh", QPair<QString, QString>("sources", "CMBGeometryReader"));
    readerMap.insert("obj", QPair<QString, QString>("sources", "CMBGeometryReader"));
    readerMap.insert("tin", QPair<QString, QString>("sources", "CMBGeometryReader"));
    readerMap.insert("stl", QPair<QString, QString>("sources", "CMBGeometryReader"));

    // These should eventually come from Moab reader plugin
    readerMap.insert("h5m", QPair<QString, QString>("sources", "CmbMoabSolidReader"));
    readerMap.insert("sat", QPair<QString, QString>("sources", "CmbMoabSolidReader"));
    readerMap.insert("brep", QPair<QString, QString>("sources", "CmbMoabSolidReader"));
    readerMap.insert("exo", QPair<QString, QString>("sources", "CmbMoabSolidReader"));
    readerMap.insert("cub", QPair<QString, QString>("sources", "CmbMoabSolidReader"));
    return readerMap;
  }

  static cmb_FileExtMap SceneBuilder_ReadersMap()
  {
    cmb_FileExtMap readerMap = ModelBuilder_ReadersMap();
    readerMap.insert("osd.txt", QPair<QString, QString>("sources", "OSDLReader"));
    readerMap.insert("bin.pts", QPair<QString, QString>("sources", "CMBGeometryReader"));
    readerMap.insert("bin", QPair<QString, QString>("sources", "CMBGeometryReader"));
    readerMap.insert("pts", QPair<QString, QString>("sources", "CMBGeometryReader"));
    readerMap.insert("fac", QPair<QString, QString>("sources", "CMBGeometryReader"));
    return readerMap;
  }

  static cmb_FileExtMap PointsBuilder_ReadersMap()
  {
    cmb_FileExtMap readerMap;
    readerMap.insert("bin.pts", QPair<QString, QString>("sources", "LIDARReader"));
    readerMap.insert("bin", QPair<QString, QString>("sources", "LIDARReader"));
    readerMap.insert("pts", QPair<QString, QString>("sources", "LIDARReader"));
    return readerMap;
  }

  static cmb_FileExtMap MeshViewer_ReadersMap()
  {
    cmb_FileExtMap readerMap;
    readerMap.insert("2dm", QPair<QString, QString>("sources", "CMBMeshReader"));
    readerMap.insert("3dm", QPair<QString, QString>("sources", "CMBMeshReader"));
    readerMap.insert("vtk", QPair<QString, QString>("sources", "LegacyVTKFileReader"));
    return readerMap;
  }


}
//ETX
#endif
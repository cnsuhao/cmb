vtk_module(vtkCMBIO
  DEPENDS
    vtkCMBGeneral
    vtkIOXML
  PRIVATE_DEPENDS
    vtkCMBMeshing
    vtkFiltersGeometry
    vtkGeovisCore
    vtkIOExport
    vtkIOGDAL
    vtkIOParallelExodus
    vtksys
  EXCLUDE_FROM_WRAP_HIERARCHY
)

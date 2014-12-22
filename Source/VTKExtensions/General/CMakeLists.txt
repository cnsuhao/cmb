
set(CMB_General_Unwrapped_srcs
  vtkCMBArc.cxx
  vtkCMBArcEndNode.cxx
  vtkCMBArcManager.cxx
  #vtkDateFormatter.cxx
  ${UI_BUILT_SRCS}
)

set(CMB_General_SRC
  vtkCMBArcProvider.cxx
  vtkCMBConeSource.cxx
  vtkCMBProgramManager.cxx
  vtkCMBProjectManager.cxx
  vtkCMBScalarLineSource.cxx
  vtkCMBSphericalPointSource.cxx
  vtkContourPointCollection.cxx
  #vtkDateFormatter.cxx
  #vtkDateTime.cxx
  vtkDiscreteLookupTable.cxx
  vtkGMSMeshSource.cxx
  vtkHydroModelMultiBlockSource.cxx
  vtkHydroModelPolySource.cxx
  vtkHydroModelSelectionSource.cxx
  vtkModelLineSource.cxx
  vtkMultiBlockWrapper.cxx
  vtkSceneContourSource.cxx
  ${CMB_General_Unwrapped_srcs}
)

 set_source_files_properties(
   ${CMB_General_Unwrapped_srcs}
   WRAP_EXCLUDE
 )

set(vtkCMBGeneral_NO_HeaderTest 1)

set (old_VTK_WRAP_HINTS ${VTK_WRAP_HINTS})
set(VTK_WRAP_HINTS ${CMAKE_CURRENT_SOURCE_DIR}/hints)

vtk_module_library(vtkCMBGeneral ${CMB_General_SRC})

set (VTK_WRAP_HINTS ${old_VTK_WRAP_HINTS})

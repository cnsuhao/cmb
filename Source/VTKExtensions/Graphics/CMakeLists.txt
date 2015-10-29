
#smtk is required for triangle meshing
find_package(SMTK REQUIRED)

set(CMB_Graphics_SRC
  vtkCleanPolylines.cxx
  vtkCMBArcWidgetRepresentation.cxx
  vtkCMBBandedPolyDataContourFilter.cxx
  vtkCMBExtractCellFromDataSet.cxx
  vtkCMBExtractMapContour.cxx
  vtkCMBImplicitPlaneRepresentation.cxx
  vtkCMBLargeTextureSurfaceRepresentation.cxx
  # vtkCMBPolylineActor.cxx
  # vtkCMBPolylineRepresentation.cxx
  vtkCMBInitialValueProblemSolver.cxx
  vtkCMBStreamTracer.cxx
  vtkExtractLeafBlock.cxx
  vtkExtractLine.cxx
  vtkExtractMultiBlockBlock.cxx
  vtkImageTextureCrop.cxx
  vtkOrientedGlyphContourRepresentation2.cxx
  vtkPVTrackballDolly.cxx
  vtkSplineFunctionItem.cxx
)

if("${VTK_RENDERING_BACKEND}" STREQUAL "OpenGL2")
  add_definitions(-DVTKGL2)
endif()

set(vtkCMBGraphics_NO_HeaderTest 1)
vtk_module_library(vtkCMBGraphics ${CMB_Graphics_SRC})

#we link against smtkCore so that we get the proper include directories
#for smtk, as those aren't exported with vtkSMTKDiscreteExt
target_link_libraries(vtkCMBGraphics
                      LINK_PRIVATE
                      smtkCore
                      vtkSMTKDiscreteExt)


set(CMB_Graphics_SRC
  vtkCleanPolylines.cxx
  vtkCMBArcWidgetRepresentation.cxx
  vtkCMBExtractCellFromDataSet.cxx
  vtkCMBExtractMapContour.cxx
  vtkCMBImplicitPlaneRepresentation.cxx
  vtkCMBLargeTextureSurfaceRepresentation.cxx
  vtkCMBPolylineActor.cxx
  vtkCMBPolylineRepresentation.cxx
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

set(vtkCMBGraphics_NO_HeaderTest 1)
vtk_module_library(vtkCMBGraphics ${CMB_Graphics_SRC})

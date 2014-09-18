set (__dependencies)

if (PARAVIEW_ENABLE_PYTHON)
  list(APPEND __dependencies
      vtkPythonInterpreter)
endif (PARAVIEW_ENABLE_PYTHON)

vtk_module(vtkCMBModelBridge
  DEPENDS
#   smtkDiscreteModel
   vtkPVServerManagerCore
   vtkPVClientServerCoreRendering
   vtkPVVTKExtensionsRendering
   vtksys
   ${__dependencies}
 EXCLUDE_FROM_WRAP_HIERARCHY
)

project (cmbAppCommon)

# Any build configured headers should be added to this
# variable so that they also get installed.
set (dynamicHeaders)

find_package(Remus REQUIRED)
find_package(SMTK REQUIRED)


add_definitions("-DCMB_ENABLE_PYTHON")

# Add Qt UI files here
set(UI_FORMS
    UI/qtAboutDialog.ui
    UI/qtArcFunctionControl.ui
    UI/qtCMBArcModifierInputDialog.ui
    UI/qtCMBApplicationOptions.ui
    UI/qtCMBBathymetryDialog.ui
    UI/qtCMBConicalSourceDialog.ui
    UI/qtCMBCreateSimpleGeometry.ui
    UI/qtCMBOptionsDialog.ui
    UI/qtCMBArcEditWidget.ui
    UI/qtCMBFunctionEditor.ui
    UI/qtCMBFunctionWedgeEditor.ui
    UI/qtCMBSceneArcPolygonMeshingDialog.ui
    UI/qtCMBSceneArcSnappingDialog.ui
    UI/qtCMBGenerateContoursDialog.ui
    UI/qtCMBSceneMesherDialog.ui
    UI/qtCMBSceneObjectDuplicate.ui
    UI/qtCMBSceneObjectImportNoUnits.ui
    UI/qtCMBSceneSurfaceMesherDialog.ui
    UI/qtCMBManualFunctionWidget.ui
    UI/qtCMBTINStitcherDialog.ui
    UI/qtDefineGroundPlane.ui
    UI/qtDefineVOI.ui
    UI/qtEnumPropertyWidget.ui
    UI/qtCMBMainWindow.ui
    UI/qtHelpDialog.ui
    UI/qtLIDARNumberOfPointsDialog.ui
    UI/qtModifierArcDialog.ui
    UI/qtNewSceneUnits.ui
    UI/qtObjectPlacementConstraint.ui
    UI/qtObjectTypeDialog.ui
    UI/qtPreviewDialog.ui
    UI/qtCMBSceneObjectFilterDialog.ui
    UI/qtCMBStackedTINDialog.ui
    UI/qtArcEditWidget.ui
    UI/pqCMBFileDialog.ui
    )

pv_qt_wrap_ui(UI_BUILT_SOURCES
  ${UI_FORMS}
  )

# Add Qt resource files here
set(UI_RESOURCES
  ../Resources/Resources.qrc
  )

pv_qt_add_resources(RCS_SOURCES
  ${UI_RESOURCES}
  )

source_group("Resources" FILES
  ${UI_RESOURCES}
  ${UI_FORMS}
  )

source_group("Generated" FILES
  # ${MOC_BUILT_SOURCES}
  ${RCS_SOURCES}
  ${UI_BUILT_SOURCES}
  )

# Add cumulus source files
set(CUMULUS_SOURCES)
set(CUMULUS_LIBRARY)
if (ENABLE_Cumulus)
  set(CUMULUS_SOURCES qtJobsPanel.cxx qtJobsPanel.h)
  set(CUMULUS_LIBRARY smtkCumulusExt)
endif ()


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pqCMBAppCommonConfig.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/pqCMBAppCommonConfig.h
  ESCAPE_QUOTES)
set(dynamicHeaders
  "${dynamicHeaders};${CMAKE_CURRENT_BINARY_DIR}/pqCMBAppCommonConfig.h")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmbAppCommonExport.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmbAppCommonExport.h
  ESCAPE_QUOTES)
set(dynamicHeaders
  "${dynamicHeaders};${CMAKE_CURRENT_BINARY_DIR}/cmbAppCommonExport.h")

add_library(cmbAppCommon
  pqCMBLoadDataReaction.cxx
  pqCMBLoadDataReaction.h
  pqPluginIOBehavior.cxx
  pqPluginIOBehavior.h
  pqCMBColorMapWidget.cxx
  pqCMBColorMapWidget.h
  qtCMBPanelsManager.cxx
  qtCMBPanelsManager.h
  cmbManualProfileFunction.cxx
  cmbManualProfileFunction.h
  cmbProfileFunction.cxx
  cmbProfileFunction.h
  cmbProfileWedgeFunction.cxx
  cmbProfileWedgeFunction.h
  qtCMBAboutDialog.cxx
  qtCMBAboutDialog.h
  pqCMBAppCommonInit.cxx
  pqCMBAppCommonInit.h
  pqCMBColorMapDialog.cxx
  pqCMBColorMapDialog.h
  pqCMBDisplayProxyEditor.cxx
  pqCMBDisplayProxyEditor.h
  pqCMBEnumPropertyWidget.cxx
  pqCMBEnumPropertyWidget.h
  pqScalarBarWidget.cxx
  pqScalarBarWidget.h
  qtCMBHelpDialog.cxx
  qtCMBHelpDialog.h
  vtkCMBInteractorStyleRubberBand.cxx
  vtkCMBInteractorStyleRubberBand.h
  pqCMBLIDARPieceObject.cxx
  pqCMBLIDARPieceObject.h
  pqCMBLIDARPieceTable.cxx
  pqCMBLIDARPieceTable.h
  pqCMBCommonMainWindow.cxx
  pqCMBCommonMainWindow.h
  pqCMBCommonMainWindowCore.cxx
  pqCMBCommonMainWindowCore.h
  pqCMBPreviewDialog.cxx
  pqCMBPreviewDialog.h
  pqCMBProcessWidget.cxx
  pqCMBProcessWidget.h
  pqCMBRubberBandHelper.cxx
  pqCMBRubberBandHelper.h
  pqCMBRulerDialog.cxx
  pqCMBRulerDialog.h
  qtCMBProgressWidget.cxx
  qtCMBProgressWidget.h
  qtCMBTableWidget.cxx
  qtCMBTableWidget.h
  pqCMBFileDialog.cxx
  pqCMBFileDialog.h
  pqCMBFileDialogEventPlayer.h
  pqCMBFileDialogEventPlayer.cxx
  pqCMBFileDialogEventTranslator.h
  pqCMBFileDialogEventTranslator.cxx
  pqCMBTreeWidgetEventTranslator.cxx
  pqCMBTreeWidgetEventTranslator.h
  vtkQtCMBConnectComboBox.cxx
  vtkQtCMBConnectComboBox.h
  pqCMBModifierArcManager.h
  pqCMBModifierArcManager.cxx
  pqCMBModifierArc.h
  pqCMBModifierArc.cxx
  pqCMBRecentlyUsedResourceLoaderImplementatation.cxx
  pqCMBRecentlyUsedResourceLoaderImplementatation.h
  qtCMBApplicationOptions.cxx
  qtCMBApplicationOptions.h
  qtCMBApplicationOptionsDialog.cxx
  qtCMBApplicationOptionsDialog.h
  qtCMBBathymetryDialog.cxx
  qtCMBBathymetryDialog.h
  qtCMBColorDialog.cxx
  qtCMBColorDialog.h
  qtCMBConeDialog.cxx
  qtCMBConeDialog.h
  qtCMBContextMenuBehavior.cxx
  qtCMBContextMenuBehavior.h
  qtCMBCreateSimpleGeometry.cxx
  qtCMBCreateSimpleGeometry.h
  qtCMBMeshingMonitor.cxx
  qtCMBManualFunctionWidget.cxx
  qtCMBManualFunctionWidget.h
  qtCMBOptionsContainer.cxx
  qtCMBOptionsContainer.h
  qtCMBOptionsDialog.cxx
  qtCMBOptionsDialog.h
  qtCMBOptionsPage.cxx
  qtCMBOptionsPage.h
  qtCMBProfileWedgeFunctionWidget.cxx
  qtCMBProfileWedgeFunctionWidget.h
  qtCMBProjectServerManager.cxx
  qtCMBProjectServerManager.h
  pqCMBRenderLog.h
  qtCMBTreeWidget.cxx
  qtCMBTreeWidget.h
  Scene/Core/cmbSceneNodeReplaceEvent.cxx
  Scene/Core/pqCMBSceneNode.cxx
  Scene/Core/pqCMBSceneNodeIterator.cxx
  Scene/Core/pqCMBSceneTree.cxx
  Scene/CMB/pqOmicronModelWriter.cxx
  Scene/Geology/pqCMBBoreHole.cxx
  Scene/Geology/pqCMBCrossSection.cxx
  Scene/IO/pqCMBSceneReader.cxx
  Scene/IO/pqCMBSceneV1Reader.cxx
  Scene/IO/pqCMBSceneV2Reader.cxx
  Scene/IO/pqCMBSceneV2Writer.cxx
  Scene/Objects/cmbSceneUnits.cxx
  Scene/Objects/pqCMBArc.cxx
  Scene/Objects/pqCMBConicalRegion.cxx
  Scene/Objects/pqCMBFacetedObject.cxx
  Scene/Objects/pqCMBGlyphObject.cxx
  Scene/Objects/pqCMBLine.cxx
  Scene/Objects/pqCMBSceneObjectBase.cxx
  Scene/Objects/pqCMBPlane.cxx
  Scene/Objects/pqCMBPoints.cxx
  Scene/Objects/pqCMBPolygon.cxx
  Scene/Objects/pqCMBSolidMesh.cxx
  Scene/Objects/pqCMBTexturedObject.cxx
  Scene/Objects/pqCMBUniformGrid.cxx
  Scene/Objects/pqCMBVOI.cxx
  Scene/Widgets/qtCMBArcWidgetManager.cxx
  Scene/Widgets/qtCMBArcModifierInputDialog.cxx
  Scene/Widgets/qtCMBGroundPlaneDialog.cxx
  Scene/Widgets/qtCMBNewSceneUnitsDialog.cxx
  Scene/Widgets/qtCMBPlacementConstraintWidget.cxx
  Scene/Widgets/qtCMBArcEditWidget.cxx
  Scene/Widgets/qtCMBConeNodeDialog.cxx
  Scene/Widgets/qtCMBGenerateContoursDialog.cxx
  Scene/Widgets/qtCMBSceneMesherDialog.cxx
  Scene/Widgets/qtCMBSceneObjectDuplicateDialog.cxx
  Scene/Widgets/qtCMBSceneObjectFilterDialog.cxx
  Scene/Widgets/qtCMBSceneObjectImporter.cxx
  Scene/Widgets/qtCMBSceneSurfaceMesherDialog.cxx
  Scene/Widgets/qtCMBTINStitcherDialog.cxx
  Scene/Widgets/qtCMBStackedTINDialog.cxx
  Scene/Widgets/qtCMBUserTypeDialog.cxx
  Scene/Widgets/qtCMBVOIDialog.cxx

  ${CUMULUS_SOURCES}
  ${RCS_SOURCES}
  ${UI_BUILT_SOURCES}
  )

target_link_libraries(cmbAppCommon
  LINK_PUBLIC
    vtkCMBGeneral
    pqApplicationComponents
    smtkCore
    smtkQtExt # required by qtSelectionManager
#    vtkCMBMeshing # required by pqCMBSceneNode.cxx
#    vtkIOXMLParser
    CMB_Plugin
    smtkPQWidgetsExt
    ${qt_targets}
  LINK_PRIVATE
    QtTesting
    RemusClient
    smtkDiscreteSession
    ${CUMULUS_LIBRARY}
    vtkSMTKDiscreteModel
    vtksys
    vtkSMTKReaderExt
    vtkSMTKWidgetsExt
  )

set(python_path_sources)
if (USE_SYSTEM_SMTK)
  list(APPEND python_path_sources
    "USE_SYSTEM_SMTK")
endif ()

if (CMB_SUPERBUILD_DEVELOPER_ROOT)
  list(APPEND python_path_sources
    "CMB_SUPERBUILD_DEVELOPER_ROOT=\"${CMB_SUPERBUILD_DEVELOPER_ROOT}\"")
endif ()

set_source_files_properties(
  pqCMBCommonMainWindow.cxx
  PROPERTIES
    COMPILE_DEFINITIONS "${python_path_sources}")

if (PARAVIEW_ENABLE_PYTHON)
  target_link_libraries(cmbAppCommon
    LINK_PRIVATE vtkUtilitiesPythonInitializer)
endif()

target_include_directories(cmbAppCommon
  PUBLIC
    ${cmbAppCommon_SOURCE_DIR}
    ${cmbAppCommon_BINARY_DIR}
    ${cmbAppCommon_SOURCE_DIR}/Resource
    ${cmbAppCommon_SOURCE_DIR}/Scene
    ${cmbAppCommon_SOURCE_DIR}/Scene/Core
    ${cmbAppCommon_SOURCE_DIR}/Scene/CMB
    ${cmbAppCommon_SOURCE_DIR}/Scene/Geology
    ${cmbAppCommon_SOURCE_DIR}/Scene/IO
    ${cmbAppCommon_SOURCE_DIR}/Scene/Objects
    ${cmbAppCommon_SOURCE_DIR}/Scene/Widgets
    ${vtkCMBMeshing_INCLUDE_DIRS}
    ${REMUS_INCLUDE_DIRS}
    ${SMTK_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
  PRIVATE
    ${VXL_RPL_INCLUDE_DIR}

  )


#let cmake do what qt4_wrap_cpp used to do automatically
set_target_properties(cmbAppCommon PROPERTIES AUTOMOC TRUE)


# This ensure that the cmbsuite.qch file is generated.
add_dependencies(cmbAppCommon CMBSuiteHelp)

cmb_install_plugin(cmbAppCommon)

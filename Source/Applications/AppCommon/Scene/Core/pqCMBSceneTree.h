//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBSceneTree - represents a Scene Tree.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBSceneTree_h
#define __pqCMBSceneTree_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QList>
#include <QPoint>
#include <QMap>
#include <QColor>

#include <map>
#include <vector>
#include <deque>
#include "cmbSceneUnits.h"
#include <QStringList>
#include "vtkType.h"
#include "vtkBoundingBox.h"
#include "pqVariableType.h"
#include "cmbSystemConfig.h"

class pqCMBSceneNode;
class pqCMBSceneNodeCreationEvent;
class pqCMBSceneNodeDeletionEvent;
class cmbSceneNodeReplaceEvent;
class CmbPolylineSceneNode;
class QAction;
class QIcon;
class QPixmap;
class QTreeWidget;
class QTreeWidgetItem;
class QMenu;
class cmbEvent;
class pqCMBSceneObjectBase;
class pqCMBGlyphObject;
class pqCMBLine;
class pqCMBArc;
class pqCMBTexturedObject;
class pqServer;
class pqRenderView;
class pqOutputPort;
class pqPipelineSource;
class qtLineWidget;
class qtCMBArcWidgetManager;
class pqDataRepresentation;

class CMBAPPCOMMON_EXPORT pqCMBSceneTree : public  QObject {
  Q_OBJECT

  friend class pqCMBSceneNodeCreationEvent;
  friend class pqCMBSceneNodeDeletionEvent;
  friend class cmbSceneNodeReplaceEvent;

public:
  pqCMBSceneTree(QPixmap *visPixMap, QPixmap *ivisPixMap,
            QPixmap *snapPixMap, QPixmap *lockPixMap,
            QTreeWidget *widget,
            QTreeWidget *infoWidget = NULL);
  ~pqCMBSceneTree() override;

  void empty();
  bool isEmpty() const {return (this->Root == NULL);}
  pqCMBSceneNode *createNode(const char *name, pqCMBSceneNode *parent,
                           pqCMBSceneObjectBase *obj, // Was NULL by default
                           cmbSceneNodeReplaceEvent *event);

  pqCMBSceneNode *createLineTypeNode();

  pqCMBSceneNode *createRoot(const char *name);
  void addPointsToGlyph(pqCMBSceneNode *node,
                        int count,
                        double *scaleFactor,
                        std::deque<double> *glyphPoints = NULL,
                        bool repositionOriginal = false,
                        QMap<pqCMBSceneNode*, int> *constraints = NULL,
                        bool useTextureConstraint = false,
                        int glyphPlaybackOption = -1);
  bool getGlyphPoint(double p[3],
                     std::deque<double> *glyphPoints,
                     int glyphPlaybackOption,
                     double bounds[6]);

  std::vector<pqCMBSceneNode *> duplicateNode(pqCMBSceneNode *node,
                                            std::deque<double> *glyphPoints = NULL,
                                            int count = 1,
                                            bool randomPlacement = false,
                                            bool repositionOriginal = false,
                                            QMap<pqCMBSceneNode*, int> *constraints = NULL,
                                            bool useGlyphs = false,
                                            bool useTextureConstraint = false,
                                            int glyphPlaybackOption = -1);

  pqCMBSceneNode *duplicateNode(pqCMBSceneNode *node, double zOffset);
  pqCMBSceneNode *getRoot() const { return this->Root;}
  const std::vector<pqCMBSceneNode *> &getSelected() const
    { return this->Selected;}
  pqCMBSceneNode *getLineTypeNode(bool createIfDoesntExist = true);
  pqCMBSceneNode *getArcTypeNode(bool createIfDoesntExist = true);

  pqCMBSceneNode *findNode(const char *name) const;
  pqCMBSceneNode *findNode(pqCMBSceneObjectBase *obj) const;
  pqCMBSceneNode *findNode(pqPipelineSource *obj) const;
  QIcon *getIconVisible() const { return this->IconVisible;}
  QIcon *getIconInvisible() const { return this->IconInvisible;}
  QIcon *getIconNull() const { return this->IconNULL;}
  QIcon *getIconLocked() const { return this->IconLocked;}
  QTreeWidget *getWidget() const { return this->TreeWidget;}
  QTreeWidget *getInfoWidget() const { return this->InfoTreeWidget;}
  std::string createUniqueName(const char *) const;
  void enterEditMode() { this->EditMode = true;}
  void exitEditMode()
    {
      this->EditMode = false;
      emit requestSceneUpdate();
    }

  void changeName(pqCMBSceneNode *node, const char *newName);

  void setApplyBathymetryAction(QAction *changeAction);
  void setArcActions(QAction *edit, QAction *mergeEndNodes,
                     QAction *mergeArcs, QAction *growArcSelection,
                     QAction *autoConnectArcs);
  void setChangeNumberOfPointsLoadedAction(QAction *action);
  void setChangeUserDefineObjectTypeAction(QAction *);
  void setColorActions(QAction *set, QAction *unset);
  void setConicalNodeActions(QAction *create, QAction *edit);
  void setConvertToGlyphAction(QAction *);
  void setCreateArcNodeAction(QAction *);
  void setCreateLineNodeAction(QAction *);
  void setCreatePolygonAction(QAction *);
  void setCreateVOINodeAction(QAction *);
  void setDefineVOIAction(QAction *);
  void setDeleteNodeAction(QAction *);
  void setDuplicateNodeAction(QAction *nonrandomAction,
                              QAction *randomAction);
  void setElevationAction(QAction *);
  void setExportSolidsAction(QAction *);
  void setExportPolygonsAction(QAction *);
  void setGenerateArcsAction(QAction *);
  void setGenerateImageMeshAction(QAction *);
  void setGroundPlaneActions(QAction *create, QAction *edit);
  void setImportObjectAction(QAction *);
  void setInsertNodeAction(QAction *);
  void setSelectSnapNodeAction(QAction *setAction, QAction *unsetAction);
  void setSnapObjectAction(QAction *);
  void setTextureAction(QAction *change);
  void setTINStackAction(QAction *);
  void setTINStitchAction(QAction *);
  void setUndoRedoActions(QAction *undoAction, QAction *redoAction);
//  void setUseGlyphPlaybackAction(QAction *useGlyphPlaybackAction);

  void setCurrentView(pqRenderView *view);
  pqRenderView *getCurrentView() const
    {return this->CurrentView;}

  void setCurrentServer(pqServer *server);
  pqServer *getCurrentServer() const
    {return this->CurrentServer;}

  void setUnits(cmbSceneUnits::Enum unitType);
  cmbSceneUnits::Enum getUnits() const
    { return this->Units;}

  void getVOIs(std::vector<pqCMBSceneNode *> *vois) const;
  void getArcs(std::vector<pqCMBSceneNode *> *Arcs) const;

  const QStringList &getTextureFileNames()
    { return this->TextureFiles;}

  const QStringList &getUserDefinedObjectTypes()
    { return this->UserObjectTypes;}

  pqCMBSceneNode *FindLineNode(qtLineWidget *);
  void setLineWidgetCallbacks(pqCMBLine* obj);

  bool getRandomConstraintPoint(double p[3],
                                QMap<pqCMBSceneNode*, int> *constraints,
                                int glyphPlaybackOption = -1,
                                std::deque<double> *glyphPoints = NULL);
  bool IsTemporaryPtsFileForMesherNeeded(QStringList &surfaceNames);
  void  setSnapTarget(pqCMBSceneNode *node);
  // Add a typeName to the list of known types - the cleanList option will sort the
  // list and remove duplicates
  void addUserDefinedType(const char *typeName, bool cleanList=true);
  void cleanUpUserDefinedTypes();

  bool getDataObjectPickable(){return this->DataObjectPickable;}
  void setDataObjectPickable(bool val){this->DataObjectPickable = val;}

  qtCMBArcWidgetManager* getArcWidgetManager(){return ArcWidgetManager;}

  bool containsDataObjects() const;
  pqCMBSceneNode* getSceneObjectNode(pqCMBSceneObjectBase* obj);
  pqCMBSceneNode* getSceneNodeByName(const char* nodename);

  void setBoreHolesRadius(double radius);
  int createBorFileObjects(
    const QString& filename, pqPipelineSource* source);
  pqDataRepresentation* getABoreHoleRepresentation();
  pqDataRepresentation* getACrossSectionRepresentation();

  // This is used to insert Undo/Redo Events - USE WITH
  // CARE!
  void insertEvent(cmbEvent *event);

  // set/get the initial scene object color
  void setInitialSceneObjectColor(QColor& color)
    { this->InitialSceneObjectColor = color; }
  QColor& initialSceneObjectColor()
    { return this->InitialSceneObjectColor; }

public slots:
  void sceneObjectChanged();
  void convertNodesToGlyphs();
  void deleteSelected();
  void insertTypeNode();
  void duplicateSelected();
  void duplicateSelectedRandomly();
  void createCone();
  void editCone();
  void createVOI();
  void updateDefineVOIOption();
  void updateEditConeOption();
  void createGroundPlane();
  void editGroundPlane();
  void editArcObject();
  void setArcSnapping();
  void mergeArcs();
  void growArcSelection();
  void autoConnectArcs();
  void updateGroundPlaneOptions();
  void updateArcOptions();
  void createLineObject();
  void createArcObject();
  void createPolygonObject();
  pqCMBSceneNode* createBoreholeObject(pqPipelineSource*,
    pqCMBSceneNode *parent);
  pqCMBSceneNode* createCrossSectionObject(pqPipelineSource*,
    pqCMBSceneNode *parent);
  pqCMBSceneNode* createLineNode(double bounds[6]);
  void stitchTINs();
  void stackTINs();
  void generateArcsFromImage();
  void importObject();
  void setSnapTarget();
  void unsetSnapTarget();
  void snapSelected();
  void snapObject(pqCMBSceneNode *node);
  void setNodeColor();
  void unsetNodeColor();
  void updateSelectedColorMode();
  void updateSnapOptions();
  void updateTexturedObjectOptions();
  void updateBathymetryOptions();
  void editBathymetry();
  void updateTINStitchOptions();
  void updateTINStackOptions();
  void updateGenerateArcsOptions();
  void clearSelection();
  void clearSelectedGlyphPointsColor();
  void nodesSelected(bool clearSelection = false);
  void editTexture();
  void updateElevation();
  void changeNumberOfPointsLoadedAction();
  void selectLineNode();
  void updateLineNodeVisibility(bool);
  void collapseAllDataInfo();
  void recomputeInfo(QTreeWidgetItem*);
  // Description:
  // Exports Solids and Lines into a CMB file.
  void exportSelectedSolids();
  void exportSelectedPolygons();
  void defineSelectedVOI();
  void showContextMenu(const QPoint &pos);
  void changeUserDefineObjectTypeAction();
  void deleteNode(pqCMBSceneNode *node,
                  cmbSceneNodeReplaceEvent *event);
  void updateUseGlyphPlayback(bool checked);

  // These methods deal with the controlling of
  // events that can be Undone/Redone

  void turnOffEventRecording()
  { this->RecordEvents = false;}
  void turnOnEventRecording()
  { this->RecordEvents = true;}
  bool recordingEvents() const
  { return this->RecordEvents;}
  void emptyEventList();
  void deleteUndoEvents();
  void undo();
  void redo();
  void setTextureMap(const QString& filename,
    int numberOfRegistrationPoints, double *points);
  void unsetTextureMap();
  void addTextureFileName(const char *filename);
  void generateMeshFromImage();

signals:
  void requestSceneUpdate();
  void selectionUpdated(const QList<pqCMBSceneNode*> *,
                        const QList<pqCMBSceneNode*> *);
  void nodeUnselected(pqCMBSceneNode *);
  void nodeSelected(pqCMBSceneNode *);
  void nodeNameChanged(pqCMBSceneNode *);
  void noNodeSelected();
  void lockedNodesChanged();
  void requestTINStitch();
  void requestSolidExport();
  void requestPolygonsExport();

  void newCurrentView(pqRenderView*);
  void newCurrentServer(pqServer*);
  void enableMenuItems(bool);

  void focusOnSceneTab();
  void focusOnDisplayTab();
  void firstDataObjectAdded();

  void setCameraManipulationMode(int mode);
  void resetCameraManipulationMode();
  void resetViewDirection(double look_x, double look_y, double look_z,
                          double up_x, double up_y, double up_z);
  void enableToolbars(bool enable);

protected:

  void setNewObjectPosition(pqCMBSceneNode *node, bool randomPlacement,
                            bool translateBasedOnView, int repeatCount,
                            QMap<pqCMBSceneNode*, int> *constraints,
                            int glyphPlaybackOption,
                            QString glyphPlaybackFilename,
                            bool useTextureConstraint = false);

  void attachNode(pqCMBSceneNode *node);
  void detachNode(pqCMBSceneNode *node);
  void convertChildrenToGlyphs(pqCMBSceneNode *node,
                                const std::string &filename,
                                cmbSceneNodeReplaceEvent *event);
  //pass nothing to clear the color
  void setColorNodeIcon(double color[4]);
  void clearColorNodeIcon();
  bool boundingBoxContainsPoint(vtkBoundingBox *bb, double p[3]);
   QIcon *IconVisible;
  QIcon *IconInvisible;
  QIcon *IconSnap;
  QIcon *IconLocked;
  QIcon *IconNULL;
  pqCMBSceneNode *Root;
  pqCMBSceneNode *SnapTarget;
  std::map<std::string, pqCMBSceneNode *> NameMap;
  std::map<pqCMBSceneObjectBase *, pqCMBSceneNode *> ObjectMap;
  std::map<pqPipelineSource *, pqCMBSceneNode *> SourceMap;
  bool EditMode;
  QTreeWidget *TreeWidget;
  QTreeWidget *InfoTreeWidget;
  QAction *InsertAction;
  QAction *ImportAction;
  QAction *DeleteAction;
  QAction *DuplicateAction;
  QAction *DuplicateRandomlyAction;
  QAction *VOIAction;
  QAction *ConeCreateAction;
  QAction *ConeEditAction;
  QAction *ConvertToGlyphAction;
  QAction *ExportSolidsAction;
  QAction *ExportPolygonsAction;
  QAction *LineAction;
  QAction *ArcAction;
  QAction *EditArcAction;
  QAction *ArcSnappingAction;
  QAction *MergeArcsAction;
  QAction *GrowArcSelectionAction;
  QAction *AutoConnectArcsAction;
  QAction *CreatePolygonAction;
  QAction *SetSnapTargetAction;
  QAction *UnsetSnapTargetAction;
  QAction *SetSnapObjectAction;
  QAction *SetNodeColorAction;
  QAction *UnsetNodeColorAction;
  QAction *ChangeTextureAction;
  QAction *ApplyBathymetryAction;
  QAction *ChangeNumberOfPointsLoadedAction;
  QAction *TINStitchAction;
  QAction *TINStackAction;
  QAction *GenerateArcsAction;
  QAction *DefineVOIAction;
  QAction *CreateGroundPlaneAction;
  QAction *EditGroundPlaneAction;
  QAction *ChangeUserDefineObjectTypeAction;
  QAction *ElevationAction;
  QAction *RedoAction;
  QAction *UndoAction;
  QAction *GenerateImageMesh;
  pqRenderView *CurrentView;
  pqServer *CurrentServer;
  pqCMBTexturedObject* CurrentTextureObj;
  cmbSceneUnits::Enum Units;
  std::vector<pqCMBSceneNode *> Selected;
  QStringList TextureFiles;
  QStringList UserObjectTypes;
  QMenu *ContextMenu;
  QMenu *PropertiesMenu;
  QMenu *FileMenu;
  QMenu *EditMenu;
  QMenu *CreateMenu;
  QMenu *ToolsMenu;
  QList<cmbEvent*> UndoRedoList;
  int CurrentUndoIndex;
  const int MaxUndoRedoLimit;
  bool RecordEvents;
  QColor InitialSceneObjectColor;

  // if this is set to false, all the data object will not be pickable in the scene
  bool DataObjectPickable;

  //create the arc widget manager and setup the signal / slots
  void createArcWidgetManager();

  //controls the creation of arcs, and manages the singelton arc widget
  qtCMBArcWidgetManager* ArcWidgetManager;

  // If using glyph playback
  bool useGlyphPlayback;

protected slots:
  void nodeChanged(QTreeWidgetItem*, int);
  void nodeClicked(QTreeWidgetItem*, int);

  void arcWidgetReady();
  void arcWidgetBusy();
  void updateArcsAfterSplit(pqCMBSceneNode* arcNode,QList<vtkIdType> newArcIds);
  void updateArc(pqCMBSceneNode* arcNode);
  void refreshArcsAndPolygons();

  void enableSceneTree(const bool &lock);
};





#endif /* pqCMBSceneTree_h */

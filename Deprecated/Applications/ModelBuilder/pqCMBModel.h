//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBModel - a CMB model object.
// .SECTION Description
// This class is the main client side CMB model class that handles all the
// CMB model operations invoked from the client GUI. It is responsible for
// setting up the model operations properly and launching those operations
// on the server. It also handles getting results from the server side model
// operations and updating the client side model accordingly so that the
// server side model is consistent with the client side model.
// .SECTION Caveats

#ifndef __pqCMBModel_h
#define __pqCMBModel_h

#include <QObject>

#include <QString>
#include <QMap>
#include <QPointer>
#include <QList>
#include <QColor>
#include <QWidget>
#include "vtkSmartPointer.h"
#include "vtkWeakPointer.h"
#include "vtkNew.h"
#include "vtkPVCMBModelInformation.h"

#include "smtk/PublicPointerDefs.h"
#include "cmbSystemConfig.h"

class pqCMBModelFace;
class pqCMBModelVertex;
class pqCMBModelEdge;
class QTreeWidget;

class vtkDiscreteModel;
class vtkEventQtSlotConnect;
class vtkSMProxy;
class vtkSMSourceProxy;
class pqOutputPort;
class pqDataRepresentation;
class pqPipelineSource;
class pqRenderView;
class pqServer;
class vtkModelMaterial;
class vtkDiscreteModelEntityGroup;
class vtkDiscreteLookupTable;
class vtkModelEntity;
class vtkObject;
class vtkPVSelectionInformation;
class vtkIdTypeArray;
class vtkCMBModelStateOperatorClient;
class vtkModelVertex;
class pqCMBFloatingEdge;
class pqCMBModelEdge;
class pqCMBModelVertex;
class pqCMBModelEntity;
class vtkGeoTransformOperatorClient;
class vtkCMBMeshClient;
class vtkCollection;
class vtkSMOperatorProxy;
class vtkTransform;

class  pqCMBModel :  public QObject
{
  Q_OBJECT

  typedef QMap< vtkIdType, pqCMBModelEntity* > ModelEntityMap;
  typedef ModelEntityMap::iterator EntityMapIterator;
  typedef QList<vtkIdType>::iterator IdListIterator;

public:
  pqCMBModel(pqRenderView*, pqServer*);
  virtual ~pqCMBModel();

  // Description:
  // Get/clear/update the current model
  vtkDiscreteModel* getModel();
  void clearClientModel(bool emitSignal = true);
  void updateModel();
  pqPipelineSource* modelSource()
    {return this->ModelSource;}
  pqDataRepresentation* modelRepresentation()
    {return this->ModelRepresentation; }
  void getSelectedModelEntities(QList<vtkIdType>& selEntities);
  vtkSMProxy* getServerMeshProxy();
  void updateModelSource();

  // Description:
  // Convenient method to get current model dimension
  // NOTE: We should be careful when using this method to update UI
  // since now the mixed model is supported, and 3D models can have edges,
  // vertex and 2D operations.
  int getModelDimension();

  // Description:
  // Convenient method to check if the model contain 2D edges
  // NOTE: This only checks the edges that associated with model faces,
  // NOT the floating edges for 3D model.
  bool has2DEdges();

  // Description;
  // Get model bounds. Return true on success, false on failure.
  bool getModelBounds(double boulds[6]);

  // Description;
  // Check whether the model has geometry entity loaded
  bool hasGeometryEntity();

  // Description:
  // Get the model wrapper.  This is exposed for the
  // adh model exporter operator.
  vtkSMProxy* getModelWrapper();

  // Description:
  // Model I/O related methods
  // Return 1 on success, 0 on failure
  int loadModelFile(const QString& filename);
  int canLoadFile(const QString& filename);
  int loadReaderSource(const QString& filename,
    pqPipelineSource* source);

  // Description:
  // Create a model from a simple geometric description.
  bool createRectangleModel(double* boundingBox, int xResolution, int yResolution);
  bool createEllipseModel(double* values, int resolution);

  // Description:
  // Create a model from a sourceProxy that creates polydata.
  bool createSimpleModel(vtkSMProxy* sourceProxy);

  // Description:
  // In 2D Model, the Model vertex should be invisible
  // if all the associated edges are invisible.
  bool shouldVertexBeVisible(vtkModelVertex* vtx);

  // Description:
  // Save/reload a state of current model
  void saveModelState();
  void reloadSavedModelState();

  // Description:
  // Methods to modify model face representation displays
  void changeModelEntityVisibility(vtkIdType entityId,
    int visible, bool render=true);
  void changeModelEntityVisibility(pqCMBModelEntity* entity,
    int visible, bool render=true);
  void highlightModelEntities(QList<vtkIdType> selfaces);
  void clearAllEntityHighlights(bool rerender=true);

  // Description:
  // Methods to modify model edge representation displays
  void changeModelEdgeVisibility(vtkIdType edgeid, int visible);
  void highlightModelEdges(QList<vtkIdType> selEdges);
  void clearAllModelEdgesHighlights(bool rerender=true);
  void setModelEdgesResolution(QList<vtkIdType> edges, int res);
  void setModelEdgeResolution(vtkIdType id, int res, bool rerender=true);

  // Description:
  // Name and color operators to modify model entities' name and color
  void modifyUserSpecifiedColor(
    int entityType, vtkIdType entityId, const QColor& newColor, bool rerender=false);
  void modifyUserSpecifiedName(
    int entityType, vtkIdType entityId, const char* name);

  // Description:
  // split faces and update the representations
  void splitModelFaces(QList<vtkIdType>& selFaces, double angle);

  // Description:
  // Creates the model edges of a hybrid model
  void createModelEdges();

  // Description:
  // merge faces and update the representations
  void mergeModelFaces(QList<vtkIdType>& selFaces);

  // Description:
  // Methods to modify material entity of the model
  vtkModelMaterial* createMaterial();
  void removeMaterial(vtkModelMaterial*);
  void changeShellMaterials(
    vtkModelMaterial* matEntity, QList<vtkIdType> shellIds);

  // Description:
  // Methods to modify model-group-entity(BCS) of the model
  vtkDiscreteModelEntityGroup* createBCS(int entType = -1);
  void removeBCS(vtkIdType bcId);
  void addEntitiesToBCGroups(vtkIdType bcId, QList<vtkIdType>& faceIds);
  void removeModelFacesFromBCS(vtkIdType bcId, QList<vtkIdType>& faceIds);

  // Description:
  // Grow (split) visible model faces given compositeIndex, cell id and a feature angle
  void growModelFacesWithCellId(vtkIdType compositeIndex, vtkIdType cellId,
    double angle, int growMode=0);

  // Description:
  // Grow (split) visible model faces give the feature angle. The selected
  // cell will used as the seed to grow.
  void growModelFacesWithAngle(pqOutputPort* selPort, double angle,
    int growMode=0);
  vtkIdTypeArray* getGrowSelectionIds(
    vtkPVSelectionInformation* selInfo);

  // Description:
  // Called whenever a cell grow is made. converts the selection to a value
  // based selection so that we can control selections of individual cells.
  void modifyCellGrowSelections(QList<pqOutputPort*>&, int growMode=0 );
  void modifyCellGrowSelection(QList<vtkIdType>& , int growMode=0);

  // Description:
  // Save BCSs file.
  void saveBCSs(const QString& filename);
  // Description:
  // Save the model.
  void saveData(const QString& filename);
  // Description:
  // Write Omicron "mesh" input file (returns true on success)
  bool writeOmicronMeshInput(const QString& filename, const QString& bcsFilename,
                             const QString& tetgenCmds);

  // Description:
  // Closes the currently opened data.
  void closeData();

  // Description:
  // clear Grow selection
  void clearGrowResult();

  // Description:
  // Accept the grow result, which will eventually split the selected faces
  // according to the feature angle, then create a new BC group with all
  // selected faces, and return the new bcId if success
  vtkModelEntity* acceptGrowResult(QList<vtkIdType> &outBCSFaces);

  // Description:
  // create a new pqCMBModelFace given an face id;
  pqCMBModelFace* createNewFace(vtkIdType faceId);
  pqCMBModelEdge* createNewEdge(vtkIdType edgeId);
  pqCMBModelVertex* createNewVertex(vtkIdType VertexId);

  // Description:
  // Get model face ids given a list of BCs ids.
  QList<vtkIdType> getModelEntitiesFromBCSs(QList<vtkIdType> bcNodes);

  // Description:
  // Convert a node to/from a end node of an arc/edge for 2D model
  bool splitSelectedEdgeNodes(const QMap< vtkIdType, QList<vtkIdType> >& selArcs);
  bool convertSelectedEndNodes(
    const QList<vtkIdType>& selVTXs);
  void convertSelectedNodes();

  // Description:
  // Get/Set the edge color-by mode
  int getFaceColorMode();
  void setFaceColorMode(int mode);

  // Description:
  // Get/Set the edge color-by mode
  int getEdgeColorMode();
  void setEdgeColorMode(int mode);

  // Description:
  // Get/Set the edge domain (2d model face) color-by mode
  int getColorEdgeDomainMode();
  void setColorEdgeDomainMode(int mode);

  // Description:
  // update the model with the mesh
  void setupMesh(vtkCMBMeshClient* meshClient);
  void updateEdgeMesh();
  void updateFaceMesh();
  void applyMeshBathymetry(vtkSMProxy* bathymetrySource, double eleRaius,
    bool useHighLimit, double eleHigh, bool useLowLimit, double eleLow,
    bool applyOnlyToVisibleMesh=false);
  void updateMeshBathymetry();
  void updateAdjacentFacesMesh(QList<vtkModelEntity*> AdjacentFaces);
  void updateSelectedEdgeMesh(vtkCollection* selEdges);
  void updateSelectedFaceMesh(vtkCollection* selFaces);

  // Description
  // Apply/remove the bathymetry source to current model and mesh.
  void applyBathymetry(pqPipelineSource* bSource, double eleRadius,
    bool useHighLimit=false, double eleHigh=0.0,
    bool useLowLimit=false, double eleLow=0.0,
    bool applyOnlyToVisibleMesh=false);
  void removeBathymetry(bool update=true);

  // Description:
  // Some convenient methods.
  void setModelFacesPickable(bool pickable);
  QString getCurrentModelFile() const
  {return this->CurrentModelFileName;}
  QString getOutputModelFile() const
  {return this->OutputFileName;}
  void getVisibleModelEntityIDs(QList<vtkIdType> &visEntities);
  const QMap<vtkIdType, pqCMBModelEntity*> &GetFaceIDToFaceMap() const
   {return this->FaceIDToFaceMap;}
  const QMap<vtkIdType, pqCMBModelEntity*> &Get2DEdgeID2EdgeMap() const
  {return this->EdgeIDToEdgeMap;}
  // For 2D model, return the EdgeIDToEdgeMap; for 3D model, return FaceIDToFaceMap
  const QMap<vtkIdType, pqCMBModelEntity*> &GetCurrentModelEntityMap() ;

  const QMap<vtkIdType, pqCMBModelEntity*> &GetNGIDToNGRepMap() const
  {return this->NGIDToNGRepMap;}
  const QMap<vtkIdType, pqCMBModelEntity*> &Get3DFloatingEdgeId2EdgeMap() const
   {return this->EdgeID2RepMap;}
  const QMap<vtkIdType, pqCMBModelEntity*> &Get2DVertexID2VertexMap() const
  {return this->VertexIDToVertexMap;}
  const QList<vtkIdType> &GetLastMergeRemovedFaces() const
   {return this->MergeRemovedFaces;}
  void setEdgesColor(const QColor& ecolor);
  void setFacesColor(const QColor& fcolor);

  // Description:
  // Zoom on this object related methods
  void zoomOnObject(pqCMBModelEntity* modelEntity);
  void zoomOnSelection();

  pqCMBModelEntity* getFirstSelectedModelEntity();
  void updateModelEntityRepresentation(vtkIdType faceId);
  void removeModelEntities(QList<vtkIdType>& selFaces);
  bool isShellTranslationPointsLoaded();
  int getNumberOfModelEntitiesWithType(int itemType);
  pqPipelineSource* getMasterPolyProvider();
  void colorAttributeEntities (QList<vtkIdType>& entIds, QColor color,
    QList<vtkIdType>& coloredEntIds, bool byDomain);
  bool setEntityColor (vtkIdType entId, QColor color);

  void setCurrentFaceAttributeColorInfo(smtk::attribute::ManagerPtr AttManager,
                                        const QString& type);
  void setCurrentEdgeAttributeColorInfo(smtk::attribute::ManagerPtr AttManager,
                                        const QString& type);
  void setCurrentDomainAttributeColorInfo(smtk::attribute::ManagerPtr AttManager,
                                          const QString& type);

  // Description:
  // Texture registrations
  const double* getRegistrationPoints()
  { return this->RegistrationPoints;}
  bool hasTexture() const
    { return (this->NumberOfRegistrationPoints > 0);}
  const QString & getTextureFileName()
    { return this->TextureFileName;}
  int getNumberOfRegistrationPoints() const
    { return this->NumberOfRegistrationPoints;}
  void getRegistrationPointPair(int i, double xy[2], double st[2]) const;
  void setTextureMap(const char *filename, int numberOfRegistrationPoints,
      double *points);
  void unsetTextureMap();
  void updateModelTexture();
  bool setEntityShowTexture(vtkIdType faceId, int show);

public slots:
  void onSaveData(QWidget* parent=NULL);
  void onSaveBCSs(QWidget* parent=NULL);
  void onCloseData();
  void onLookupTableModified();
  //void onDragStarted(QTreeWidget*);
  void onModelModified();
  void onBCGroupNumberChanged();
  void onTransformProperyModified(
  vtkObject* senderObj, unsigned long ulEvent,
    void* evtPropertyName);

  void setFaceMeshVisibility(int visible);
  void setEdgeMeshVisibility(int visible);
  void setEdgePointsVisibility(int visible);
  void setEdgeMeshPointsVisibility(int visible);

  // Description:
  // Convert lat-long
  void convertLatLong(bool);
  void onEmptyEntitiesCreated()
  {  emit this->emptyEntitiesCreated();  }

signals:
  void currentModelLoaded();
  void currentModelCleared();
  void currentVTKConnectionChanged(pqDataRepresentation* connRep);
  void currentModelModified();
  void modelEntityNameChanged(vtkModelEntity*);
  void entitiesSplit(QMap< vtkIdType, QList<vtkIdType> >& splitMap, bool bEdgesFromFace=false);
  void entitiesMerged(vtkIdType toFaceId, QList<vtkIdType>& selFaceIds);
  void bcGroupNumberChanged();
  void emptyEntitiesCreated();

protected:

  // Description:
  // initialize methods
  void init();
  int initModel();
  void createLookupTable();

  // Description:
  // CMB model setup and update
  void setupClientModel();
  int showModel();
  void updateModelInternal();

  // Description:
  // apply/unapply mesh bathymetry
  void applyBathymetryToMeshEntities(vtkSMProxy* bathymetrySource,
    double eleRaius, QMap<vtkIdType, pqCMBModelEntity*> & entityMap,
    bool useHighLimit, double eleHigh, bool useLowLimit, double eleLow,
    bool applyOnlyToVisibleMesh=false);

  // Description:
  // get Model bathymetry filter
  pqPipelineSource* getModelBathymetryFilter();

  // Description:
  // CMB model VTK-connections
  void setupRepresentationVTKConnection(
    pqDataRepresentation*, bool updateRep = true);
  void connectTransformProperties(pqDataRepresentation*);
  int modify2DModelTransformProperties(
    vtkObject* senderObj, unsigned long, void* evtPropertyName);
  void getModelTransform(vtkTransform* modelTransform);

  // Description:
  // Methods to modify display of CMB model
  void resetRepresentationColors();
  void setAllRepresentationsVisibility(int);
  void updateEntityRepresentations(
    QMap<vtkIdType, pqCMBModelEntity*> &entityMap);
  void updateModelRepresentation();
  void removeModelEntities(QList<vtkIdType>& faces,
    QMap<vtkIdType, pqCMBModelEntity*> &entityMap);

  void clearEntityMap(ModelEntityMap& entitymap);
  void synchronizeModelRepTransformation(pqDataRepresentation* targetRep);
  void updateModelDataInfo();
  void updateHybridModelEdges(
    std::map<vtkIdType, std::vector<vtkIdType> >& SplitEdgeMap,
    std::map<vtkIdType, std::vector<vtkIdType> >& SplitVertMap,
    std::vector<vtkIdType>& NewEdges, std::vector<vtkIdType>& NewVerts,
    QMap< vtkIdType, QList<vtkIdType> > &SplitEdgeToEdgesMap);
  void createCMBArcs(QList<vtkIdType>& newVTKArcs,
    QList<vtkIdType>& newVTKVTXs, QList<vtkIdType>& newCMBArcs);

  // attributes
  void colorByAttributes(QList<vtkIdType>& assignedIds,
    const QString& strAttDef, bool byDomain=false);

  // Description:
  // Some ivars
  vtkDiscreteModel* Model;
  vtkSMProxy* ModelWrapper;
  QMap< vtkIdType, pqCMBModelEntity* > FaceIDToFaceMap;
  QMap< vtkIdType, pqCMBModelEntity* > EdgeIDToEdgeMap;
  QMap< vtkIdType, pqCMBModelEntity* > VertexIDToVertexMap;
  QMap< vtkIdType, pqCMBModelEntity* > NGIDToNGRepMap;
  QMap< vtkIdType, pqCMBModelEntity*> EdgeID2RepMap;
  QList<vtkIdType> MergeRemovedFaces;

  pqPipelineSource* ModelSource;
  pqDataRepresentation* ModelRepresentation;
  pqPipelineSource* ModelSelectionSource;

private:

  // Description:
  // Some private ivars
  QPointer<pqServer> ActiveServer;
  QPointer<pqRenderView> RenderView;
  QString CurrentModelFileName;
  QString OutputFileName;
  vtkSmartPointer<vtkDiscreteLookupTable> ColorLookupTable;
  vtkSmartPointer<vtkCMBModelStateOperatorClient> StateOperator;
  vtkSmartPointer<vtkGeoTransformOperatorClient> LatLongTransformOperator;

  // Description:
  // Some VTK-connection related private ivars
  vtkSmartPointer<vtkEventQtSlotConnect> VTKConnect;
  vtkSmartPointer<vtkEventQtSlotConnect> VTKColorModeConnect;
  vtkSmartPointer<vtkEventQtSlotConnect> VTKModelConnect;

  // Description:
  // Some Grow related private ivars
  vtkSMSourceProxy* GrowModelFaces;
  pqPipelineSource* CurrentGrowSource;
  pqPipelineSource* CurrentGrowSelectionSource;
  pqPipelineSource* MasterPolyProvider;
  pqDataRepresentation* CurrentGrowRep;

  int FaceColorMode;
  int EdgeColorMode;
  int EdgeDomainColorMode;
  vtkWeakPointer<vtkCMBMeshClient> MeshClient;

  QPointer<pqPipelineSource> ModelBathymetryFilter;
  QPointer<pqPipelineSource> BathymetrySource;
  double BathElevationRadious;
  double BathHighestZValue;
  bool BathUseHighestZValue;
  double BathLowestZValue;
  bool BathUseLowestZValue;
  bool BathOnlyOnVisibleMesh;
  int ShowEdgePoints;

  vtkSMOperatorProxy* BathyMetryOperatorProxy;

  QList<vtkIdType> CurrentSelectedEntities;
  vtkNew<vtkPVCMBModelInformation> ModelDataInfo;

  // Attribute related
  smtk::attribute::ManagerPtr AttManager;
  QString CurrentFaceAttDefType;
  QString CurrentEdgeAttDefType;
  QString CurrentDomainAttDefType;

  // Texture related
  QPointer<pqPipelineSource> RegisterTextureFilter;
  QPointer<pqPipelineSource> TextureImageSource;
  QString TextureFileName;
  int NumberOfRegistrationPoints;
  double RegistrationPoints[12];
  vtkSMOperatorProxy* TextureOperatorProxy;

  void prepTexturedObject(pqServer *server, pqRenderView *view);

};

#endif /* __pqCMBModel_h */

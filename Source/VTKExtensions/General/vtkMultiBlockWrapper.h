//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkMultiBlockWrapper - Wrapper class for CMB multiblocks
// .SECTION Description
// Wrapper class for using vtkMultiBlockDataSet for CMB.  The "system" id
// is created by this wrapper and the "user" id and name is just stored
// in this data structure for GUI reference.  All system ids will just
// be referred to as id and user ids and names will be referred to as
// UserId and UserName, respectively.

#ifndef __MultiBlockWrapper_h
#define __MultiBlockWrapper_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkObject.h"
#include "vtkSetGet.h"
#include "cmbSystemConfig.h"

class vtkMultiBlockDataSet;
class vtkPolyData;
class vtkIdList;
class vtkIntArray;
class vtkDataArray;
class vtkFieldData;
class vtkDataObject;
class vtkAbstractArray;
class vtkFloatArray;
class vtkInformationIntegerKey;

class VTKCMBGENERAL_EXPORT vtkMultiBlockWrapper : public vtkObject
{
public:
  static vtkMultiBlockWrapper *New();
  vtkTypeMacro(vtkMultiBlockWrapper,vtkObject);

  // Description:
  // Functions get string names used to store cell/field data.
  static const char* GetModelFaceTagName();
  static const char* GetShellTagName();
  static const char* GetMaterialTagName();
  static const char* GetReverseClassificationTagName();
  static const char* GetSplitFacesTagName();
  static const char* GetModelFaceDataName();
  static const char* GetModelFaceConvenientArrayName();
  static const char* GetMaterialUserNamesString();
  static const char* GetShellUserNamesString();
  static const char* GetShellTranslationPointString();
  static const char* GetModelFaceUserNamesString();
  static const char* GetBCSUserNamesString();
  static const char* GetShellColorsString();
  static const char* GetMaterialColorsString();
  static const char* GetBCSColorsString();
  static const char* GetModelFaceColorsString();
  static const char* GetModelFaceUse1String();

  // Description:
  // Given a vtkPolyData that is used to represent a model face,
  // return its integer (system) id.  It returns -1 if it is not
  // a model face.
  static int GetModelFaceId(vtkPolyData*);

  void SetMultiBlock(vtkMultiBlockDataSet*);
  vtkMultiBlockDataSet* GetMultiBlock();

  // Description:
  // Get the main, or master, vtkPolydata.
  vtkPolyData* GetMasterPolyData();

  // Description:
  // Gets the number of model faces that comprise the geometric model.
  int GetNumberOfModelFaces();

  // Description:
  // Get the root multi-block data that contains all nodal group polydatas.
  vtkMultiBlockDataSet* GetModelFaceRootBlock();

  // Description:
  // Gets the number of nodal groups that comprise the geometric model.
  int GetNumberOfNodalGroups();

  // Description:
  // Get the root multi-block data that contains all nodal group polydatas.
  vtkMultiBlockDataSet* GetNodalGroupRootBlock();

  // Description:
  // Gets the list of Ids of model faces that comprise the geometric model.
  void GetModelFaceIds(vtkIdList*);

  // Description:
  // Gets the list of Ids of BCSs.
  void GetBCSIds(vtkIdList*);

  // Description:
  // Gets the list of Ids of materials.
  void GetMaterialIds(vtkIdList*);

  // Description:
  // Gets the list of Ids of shells that comprise the geometric model.
  void GetShellIds(vtkIdList*);

  // Description:
  // Gets the material Id associated with the shell.
  int GetShellMaterial(int);

  // Description:
  // Sets/gets the translation point of ShellId.  If not translation point is
  // set, GetShellTranslationPoint returns NULL.  To remove a translation
  // point, pass in NULL for TranslationPoint.
  double* GetShellTranslationPoint(vtkIdType ShellId);
  void SetShellTranslationPoint(vtkIdType ShellId, double* TranslationPoint);

  // Description:
  // Creates a new model face from a set of cell ids (specified on the
  // master polydata). Returns the system id of created face.
  int CreateModelFace(int shellId, int materialId, float* rgba,
                      vtkIdList* cellIds, vtkIdList* BCSIds, int modelFaceUse1);

  // Description:
  // Creates a new model face from an existing model face.  All of the
  // cells in cellIds must belong to the same existing model face.
  // Returns the system id of the created face.
  int CreateModelFace(vtkIdList* cellIds);

  // Description:
  // Remove a model face given its id, and update related info.
  void DeleteModelFace(int faceId);

  // Description:
  // Merge the given model faces into the target model face.  All of the
  // model faces have to belong to same material/shell/BCs.
  // Returns 1 on success, 0 otherwise.
  int MergeModelFaces(vtkIdType targetFaceId, vtkIdList* faceIds);

  // Description:
  // Creates new model faces from existing model faces.  The
  // cells in cellIds are not required to belong to the same
  // existing model face. Returns the new model face ids in modelFaceIds.
  void CreateModelFaces(vtkIntArray* markedCells, vtkIntArray* modelFaceIds);

  // Description:
  // Will call vtkPolyData::RemoveDeletedCells() for each model face.
  // This function will only do the operation if needToRemoveDeletedCells
  // is true.  needToRemoveDeletedCells gets set to true when a cell is
  // removed from a model face polydata.
  void RemoveDeletedCellsFromModelFaces();

  // Description:
  // Returns the vtkPolyData representation of a model face given
  // its integer id or index.
  // Valid values are 0 <= id and 0 =< index < number of model faces.
  vtkPolyData* GetModelFaceWithId(int id);
  vtkPolyData* GetModelFaceWithIndex(int index);

  // Description:
  // Returns the cell id on the master poly data for a given
  // cell id on a given model face.
  vtkIdType GetCellIdOnMasterPolyData(vtkIdType cellId, int modelFaceId);

  // Description:
  // Given an object's system Id, return its user id and user name.
  void GetMaterialUserData(int materialId, int& materialUserId,
                           const char*& materialUserName);
  void GetBCSUserData(int BCSId, int& BCSUserId,
                      const char*& BCSUserName);
  // Description:
  // Given an model face's system Id, return its user name. Currently
  // a model face does not have a user Id.
  const char*  GetModelFaceUserName(int modelFaceId);

  // Description:
  // Given an shell's system Id, return its user name. Currently
  // a shell does not have a user Id.
  const char*  GetShellUserName(int shellId);

  // Description:
  // Given a model face id, it resets the id list and fills it in with
  // the BCSs that use/are applied over the model face.
  void GetModelFaceBCSIds(int modelFaceId, vtkIdList* idlist);

  // Description:
  // Given a model face id, returns the Material Id/Shell Id/Block index.
  int GetModelFaceMaterialId(int modelFaceId);
  int GetModelFaceShellId(int modelFaceId);
  unsigned int GetModelFaceBlockIndex(int modelFaceId);

  // Description:
  // Change the material id of a shell.  This function assumes that
  // we have already created a "valid" set of model faces.
  void ChangeMaterialIdOfShell(int shellId, int newMaterialId);

  // Description:
  // Change the user name of a material.
  void ChangeUserNameOfMaterial(int materialId, const char* userName);

  // Description:
  // Change the user id of a material.
  void ChangeUserIdOfMaterial(int materialId, int materialUserId);

  // Description:
  // Change the user color of a material.
  void ChangeUserColorOfMaterial(int materialId, float* userRGBA);

  // Description:
  // Change the user name of a shell.
  void ChangeUserNameOfShell(int shellId, const char* userName);

  // Description:
  // Change the user id of a shell.
  void ChangeUserIdOfShell(int shellId, int shellUserId);

  // Description:
  // Change the user color of a shell.
  void ChangeUserColorOfShell(int shellId, float* userRGBA);



  // Description:
  // Change the user name of a BCS.
  void ChangeUserNameOfBCS(int BCSId, const char* userName);

  // Description:
  // Change the user id of a BCS.
  void ChangeUserIdOfBCS(int BCSId, int BCSUserId);

  // Description:
  // Change the user color of a BCS.
  void ChangeUserColorOfBCS(int BCSId, float* userRGBA);

  // Description:
  // Change the user color of a model face.
  void ChangeUserColorOfModelFace(int modelFaceId, float* userRGBA);

  // Description:
  // Change the user name of a model face.
  void ChangeUserNameOfModelFace(int modelFaceId, const char* userName);

  // Description:
  // Functions to add a new BCS given a list of model face ids
  // to apply the BCS over. The function returns the system
  // specified BCS id (-1 for failure).
  int AddBCS(int BCSUserId, const char* BCSUserName,
             float* colors, vtkIdList* modelFaceIds);
  // Description:
  // Functions to delete a BCS given its system name/ID.
  void DeleteBCS(const char* name);
  void DeleteBCS(int id);

  // Description:
  // Functions to add a new material with given user data.  The
  // return value is the system id of the created material (-1
  // is returned to indicate failure).
  int AddMaterial(int MaterialUserId, const char* MaterialUserName,
                  float* colors);
  // Description:
  // Functions to delete a material given its system name/ID.
  // Note that no shell can be consist of the deleted material.
  void DeleteMaterial(int id);

  // Description:
  // Creates a shell with necessary information.
  int AddShell(const char* UserName, float* RGBA, int materialId,
    double* TranslationPoint);

  // Description:
  // Function to add/remove a BCS from a model face.  The model face id
  // is the first argument and the BCS id/name is the second argument.
  void AddBCSToModelFace(int modelFaceId, const char* BCSName);
  void AddBCSToModelFace(int modelFaceId, int BCSId);
  void RemoveBCSFromModelFace(int modelFaceId,
                              const char* BCSName);
  void RemoveBCSFromModelFace(int modelFaceId, int BCSSsytemId);

  // Description:
  // Key used to put block visibility in the meta-data associated with a block.
  static vtkInformationIntegerKey* BlockVisibilityMetaKey();

  // Description:
  // Function to Set/Get the model face visibility,
  // which is save with the meta data of the corresponding block.
  // NOTE: Currenly these functions have to used as a pair, because
  // the metadata structure is NOT set by default.
  // void SetFaceVisibility(int face, int visible);
  // int GetFaceVisibility(int face);

  int GetNumberOfBCSs();
  void GetBCSIdList(vtkIdList* IdList);
  vtkIntArray* GetBCSModelFaceIdArray(const char* BCSName);
  vtkIntArray* GetBCSModelFaceIdArray(int BCSId);

  // Description::
  // Process a polydata for the CMB writer.
  void ProcessForWriting(vtkPolyData*);

  // Description::
  // Load in the data (grid, BCSs, model faces, materials, regions/shells, ...)
  // in the given FileName for CMB.  Returns 0 if success.
  // int Load(const char* FileName);

  // Description:
  // Returns the number of shells/regions in the vtkMultiBlockDataSet.
  int GetNumberOfShells();

  // Description:
  // Functions to set split faces
  void SetSplitModelFaces(int faceid, vtkIntArray* faces);

  // Description:
  // Returns the BCS id/name given the name/id.
  int GetBCSIdFromName(const char*);
  const char* GetBCSNameFromId(int);

  // Description:
  // Create node group with given point Ids and model face Ids
  // Return 1 on success; 0 on failure.
  int CreateNodalGroup(vtkIntArray* ptIds, vtkIntArray* modelFaceIds,
    float* rgba, int isLoadingFile);

  // Description:
  // Remove a nodal group given its id, and update related info.
  void DeleteNodalGroup(int ngId);

  // Description:
  // Creates new nodal groups from existing nodal groups.  The
  // points in markedPoints are not required to belong to the same
  // existing nodal group. Returns the new nodal group ids in NGIds.
  void CreateNodalGroups(vtkIntArray* markedPoints, vtkIntArray* NGIds);

  // Description:
  // Will call vtkPolyData::RemoveDeletedCells() for each nodal group.
  // This function will only do the operation if needToRemoveDeletedCells
  // is true.  needToRemoveDeletedCells gets set to true when a cell is
  // removed from a nodal group polydata.
  void RemoveDeletedCellsFromNodalGroups();

  // Description:
  // Functions to add a new BCS given a list of nodal group ids
  // to apply the BCS over. The function returns the system
  // specified BCS id (-1 for failure).
  int AddNodalBCS(int BCSUserId, const char* BCSUserName,
             float* colors, vtkIdList* ngIds);

  // Description:
  // Function to add/remove a BCS from a nodal group.  The nodal group id
  // is the first argument and the BCS id/name is the second argument.
  void AddBCSToNodalGroup(int NodalGroupId, const char* BCSName);
  void AddBCSToNodalGroup(int NodalGroupId, int BCSId);
  void RemoveBCSFromNodalGroup(int NodalGroupId,
                              const char* BCSName);
  void RemoveBCSFromNodalGroup(int NodalGroupId, int BCSSsytemId);

  // Description:
  // Change the user name/UserId/Color of a Nodal BCS.
  void ChangeUserNameOfNodalBCS(int BCSId, const char* userName);
  void ChangeUserIdOfNodalBCS(int BCSId, int BCSUserId);
  void ChangeUserColorOfNodalBCS(int BCSId, float* userRGBA);

  // Description:
  // Change the user color/name of a nodal group.
  void ChangeUserColorOfNodalGroup(int ngId, float* userRGBA);
  void ChangeUserNameOfNodalGroup(int ngId, const char* userName);

  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Get the model face system id of the given cell id.  Note that
  // the cell id is for the cell on block 0 (i.e. the main grid).
  int GetModelFaceId(int cellId);

  // Description:
  // Set/get the model face use 1 for a model face.
  void SetModelFaceUse1(int modelFaceId, int shellId);
  int GetModelFaceUse1(int modelFaceId);

protected:
  vtkMultiBlockWrapper();
  ~vtkMultiBlockWrapper() override;

  // Description:
  // Returns the next system id to use for the color array of given name.
  int GetNextId(const char* colorsArrayName);

  // Description:
  // Marks that an object with given id has been deleted.
  void MarkObjectAsRemoved(const char* colorsArrayName, int id);
  void MarkObjectAsRemoved(vtkFloatArray* colors, int id);

  // Description:
  // Returns a name string and for a given BCS Id. This function
  // does not check to see if the name is already being used.
  // The calling function must free the passed in character array.
  void CreateBCSNameFromId(int BCSId, char** name);

  void AddBCSToModelFace(int modelFaceId, int BCSId,
                         const char* BCSName);
  void RemoveBCSFromModelFace(int modelFaceId, int BCSId,
                              const char* BCSName);
  void RemoveModelFaceFromBCSs(int modelFaceId);
  void RemoveModelFaceDataInfo(int modelFaceId);

  // Description:
  // Creates a new model face from an existing model face.  All of the
  // cells in cellIds, must belong to the same existing model face.
  // The vtkArray contains all of the model face BCSTag field array
  // data. Returns the system id of the created face.
  int CreateModelFace(float* rgba, vtkIdList* cellIds, int modelFaceUseId1,
                      vtkIntArray* array, int isLoadingFile=0);

  // Description:
  // Set the model face system id of the given cell id.  Note that
  // the cell id is for the cell on block 0 (i.e. the main grid).
  void SetModelFaceId(int cellId, int modelFaceId);

  // Description:
  // Get/Set the model face cell id of the given cell id in attached
  // vtkCellData.  Note that the cell id is for the cell on block 0
  // (i.e. the main grid).
  int GetModelFaceCellId(int cellId);
  void SetModelFaceCellId(int cellId, int modelFaceCellId);

  // Description:
  // Functions to perform deep copy when needed and replace shallow copied
  // data with deep copied data.
  vtkAbstractArray* PerformNeededDeepCopy(vtkAbstractArray*, vtkFieldData*);
  // this shouldn't be needed
  // vtkFieldData* PerformNeededDeepCopy(vtkFieldData*, vtkDataObject*);

  // Description:
  // Functions to perform shallow/deep copy when needed for model face or
  // nodal group vtkPolyDatas in vtkMultiBlock and
  // replace shallow copied data with deep copied data.
  vtkDataObject* PerformNeededShallowCopy(vtkDataObject*,
    unsigned int blockIndex, vtkMultiBlockDataSet* mbDataSet);
  vtkDataObject* PerformNeededDeepCopy(vtkDataObject*,
    unsigned int blockIndex, vtkMultiBlockDataSet* mbDataSet);

  vtkMultiBlockDataSet* mb;
  bool needToRemoveDeletedCells;

private:
  vtkMultiBlockWrapper(const vtkMultiBlockWrapper&);  // Not implemented.
  void operator=(const vtkMultiBlockWrapper&);  // Not implemented.
};

#endif

/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME vtkModelEntity - Abstract generic model entity class.
// .SECTION Description
// Abstract generic model entity class for use with vtkModel.  The user
// can set an entity's color or unique persistent Id through this.  Also,
// the user can set their own attributes for model specific information
// (e.g. UserName) but has to make sure to add the InformationKey
// in the Serialize Reader.

#ifndef __vtkModelEntity_h
#define __vtkModelEntity_h

#include "vtkDiscreteModelModule.h" // For export macro
#include "vtkModelItem.h"
#include "cmbSystemConfig.h"

class vtkInformationDoubleVectorKey;
class vtkInformationIdTypeKey;
class vtkInformationIntegerKey;
class vtkInformationKey;
class vtkInformationObjectBaseKey;
class vtkInformationStringKey;
class vtkModel;
class vtkProperty;

class VTKDISCRETEMODEL_EXPORT vtkModelEntity : public vtkModelItem
{
public:
  vtkTypeMacro(vtkModelEntity,vtkModelItem);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get color of the model entity
  void SetColor(double r, double g, double b, double a);
  double* GetColor();
  void GetColor(double RGBA[4]);

  // Description:
  // Set/get visible (non-zero is visible).
  void SetVisibility(int visible);
  int GetVisibility();

  vtkIdType GetUniquePersistentId();

  // Description:
  // Static functions for declaring properties of a
  // generic vtkModelEntity.
  static vtkInformationDoubleVectorKey* COLOR();
  static vtkInformationIdTypeKey* UNIQUEPERSISTENTID();
  static vtkInformationIntegerKey* VISIBILITY();
  static vtkInformationObjectBaseKey* DISPLAY_PROPERTY();
  static vtkInformationIntegerKey* PICKABLE();
  static vtkInformationStringKey* USERDATA();
  static vtkInformationIntegerKey* SHOWTEXTURE();

  // Description:
  // Methods to add and retrieve generic information.
  vtkGetObjectMacro(Attributes, vtkInformation);

  // Description:
  // Function for getting a model entity from a unique persistent id.
  vtkModelEntity* GetModelEntity(vtkIdType uniquePersistentId);

  // Description:
  // Function for getting a model entity from a unique persistent id and type.
  // This should be faster as it will only look for associated types
  // of Type.
  vtkModelEntity* GetModelEntity(int type, vtkIdType uniquePersistentId);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

  virtual void Initialize(vtkIdType uniquePersistentId);

  // Description:
  // Return whether or not this object has been initialized.
  vtkGetMacro(Initialized, int);

protected:
  vtkModelEntity();
  virtual ~vtkModelEntity();

  // Description:
  // Set the unique persistent Id of a model entity.  This function does not check
  // that the Id is not already being used as well as does not increment the highest
  // current unique persistent Id that is stored in vtkDiscreteModel.
  void SetUniquePersistentId(vtkIdType id); // only to be called from this and readers
  friend class vtkCMBParserBase;
  friend class vtkXMLModelWriter;
  friend class vtkXMLModelReader;

  // Description:
  // Remove/destroy this object and anything it aggregates from the its aggregator as well
  // as getting rid of topological associations.  Returns true for success and
  // false for failure. For "Use" objects, removes the association to other "Use"
  // objects only.  For vtkModelGeometricEntity objects, calls destroy for use
  // objects it owns and gets rid of associations to use objects.
  virtual bool Destroy() = 0;

private:
  vtkModelEntity(const vtkModelEntity&);  // Not implemented.
  void operator=(const vtkModelEntity&);  // Not implemented.

  // Description:
  // vtkInformation for specifying user defined attributes for
  // vtkModelEntities.
  vtkInformation* Attributes;

  int Initialized;
};

#endif


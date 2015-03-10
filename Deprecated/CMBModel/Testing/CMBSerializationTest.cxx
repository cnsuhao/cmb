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
#include "vtkModelMaterial.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkXMLModelReader.h"
#include "vtkXMLModelWriter.h"
#include "vtkInstantiator.h"
#include "vtkModelFaceUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelShellUse.h"

#include <vector>
#include <vtksys/ios/sstream>
#include <fstream>

// This tests the serialization of the CMB model but doesn't
// do it with client server.  Instead it does it by just
// comparing the stream of the first serialized model to the
// stream of the second serialized model where the second serialized
// model was created from unserializing the first stream.

int TestSerialization(vtkDiscreteModel* model)
{
  int errors = 0;
  // Write it to an archive (a string stream)
  vtkSmartPointer<vtkXMLModelWriter> writer =
    vtkSmartPointer<vtkXMLModelWriter>::New();
  vtksys_ios::ostringstream ostr;
  // Set to version to 1 (default is 0)
  writer->SetArchiveVersion(1);
  // The archiver expects a vector of objects
  std::vector<vtkSmartPointer<vtkObject> > objs;
  objs.push_back(model);
  // The root node with be called ConceptualModel. This is for
  // reference only.
  writer->Serialize(ostr, "ConceptualModel", objs);

  // Create an input stream to read the XML back
  vtksys_ios::istringstream istr(ostr.str());

  // Read using a vtkXMLModelReader
  vtkSmartPointer<vtkXMLModelReader> reader =
    vtkSmartPointer<vtkXMLModelReader>::New();
  vtkSmartPointer<vtkDiscreteModel> serializedModel =
    vtkSmartPointer<vtkDiscreteModel>::New();
  reader->SetModel(serializedModel);
  reader->Serialize(istr, "ConceptualModel");

  // Write out what we just read back to XML and print it to make sure
  // we got it right.
  objs.clear();
  objs.push_back(reader->GetModel());
  vtksys_ios::ostringstream ostr2;
  writer->Serialize(ostr2, "ConceptualModel", objs);

  if(strcmp(ostr2.str().c_str(), ostr.str().c_str()))
    {
    errors++;
    cerr << "Serialization information does not match.\n";

    cout << "==================ORIGINAL==========================\n";
    cout << ostr.str().c_str() << endl;
    cout << "==================SERIALIZED==========================\n";
    cout << ostr2.str().c_str() << endl;
    }

  // We still need to reset the model that was created during
  // serialization.
  serializedModel->Reset();

  return errors;
}

// returns the number of errors
int Test3DModelSerialization()
{
  int errors = 0;

  vtkSmartPointer<vtkDiscreteModel> model = vtkSmartPointer<vtkDiscreteModel>::New();
  vtkModelMaterial* Material = model->BuildMaterial();
  vtkDiscreteModelFace* Face0 = vtkDiscreteModelFace::SafeDownCast(
    model->BuildModelFace(0,0,0));
  vtkDiscreteModelFace* Face1 = vtkDiscreteModelFace::SafeDownCast(
    model->BuildModelFace(0,0,0));
  int FaceSides[2] = {1, 0};
  vtkDiscreteModelEntity* vtkCMBFaces[2] = {Face0, Face1};
  vtkModelFace* vtkFaces[2] = {Face0, Face1};
  vtkModelRegion* Region =
    model->BuildModelRegion(2, vtkFaces, FaceSides, Material);
  vtkDiscreteModelEntityGroup* EntityGroup =
    model->BuildModelEntityGroup(Face0->GetType(), 2, vtkCMBFaces);

  errors += TestSerialization(model);

  if(!model->DestroyModelGeometricEntity(vtkDiscreteModelRegion::SafeDownCast(Region)))
    {
    cerr << "Unable to destroy model region\n";
    errors++;
    }

  if(!model->DestroyModelGeometricEntity(Face0))
    {
    cerr << "Unable to destroy model face 0\n";
    errors++;
    }

  if(!model->DestroyModelGeometricEntity(Face1))
    {
    cerr << "Unable to destroy model face 1\n";
    errors++;
    }

  std::vector<vtkDiscreteModelEdge*> EdgeContainer;
  vtkModelItemIterator* Edges = model->NewIterator(vtkModelEdgeType);
  for(Edges->Begin();!Edges->IsAtEnd();Edges->Next())
    {
    EdgeContainer.push_back(vtkDiscreteModelEdge::SafeDownCast(Edges->GetCurrentItem()));
    }
  Edges->Delete();
  for(size_t i=0;i<EdgeContainer.size();i++)
    {
    if(!model->DestroyModelGeometricEntity(EdgeContainer[i]))
      {
      cerr << "Unable to destroy a model edge\n";
      errors++;
      }
    }

  if(!model->DestroyModelEntityGroup(EntityGroup))
    {
    cerr << "Unable to destroy model entity group\n";
    errors++;
    }

  if(!model->DestroyMaterial(Material))
    {
    cerr << "Unable to destroy material\n";
    errors++;
    }

  //model->Reset();

  // Force the model to be deleted.
  model = 0;

  return errors;
}

// returns the number of errors
int Test2DModelSerialization()
{
  vtkDiscreteModel* Model = vtkDiscreteModel::New();
  vtkModelMaterial* Material = Model->BuildMaterial();
  double x[3] = {0, 1, 2};
  // the point id (-2 value) is not set to anything realistic
  // because we currently do not have a grid to set it with respect to
  vtkModelVertex* Vertex0 = Model->BuildModelVertex(-2);
  x[0] = 5;
  vtkModelVertex* Vertex1 = Model->BuildModelVertex(-3);
  vtkModelEdge* Edges[2];
  Edges[0] = Model->BuildModelEdge(Vertex0, Vertex1);
  Edges[1] = Model->BuildModelEdge(Vertex0, Vertex1);
  int EdgeDirections[2] = {0, 1};
  vtkModelFace* Face = Model->BuildModelFace(2, Edges, EdgeDirections, Material);

  vtkDiscreteModelEntity* EntityGroupEntities = vtkDiscreteModelEdge::SafeDownCast(Edges[0]);
  /*vtkDiscreteModelEntityGroup* EntityGroup =*/
    Model->BuildModelEntityGroup(Edges[0]->GetType(), 1, &EntityGroupEntities);


  int errors = TestSerialization(Model);

  if(!Model->DestroyModelGeometricEntity(vtkDiscreteModelFace::SafeDownCast(Face)))
    {
    cerr << "Unable to destroy 2D model face\n";
    errors++;
    }
  for(int i=0;i<2;i++)
    {
    if(!Model->DestroyModelGeometricEntity(vtkDiscreteModelEdge::SafeDownCast(Edges[i])))
      {
      cerr << "Unable to destroy 2D model edge\n";
      errors++;
      }
    }
  Model->Reset();

  Model->Delete();
  return errors;
}

int main()
{
  int ModelErrors3D = Test3DModelSerialization();
  if(ModelErrors3D)
    {
    cout << "3D model serialization test failed with " << ModelErrors3D << " errors\n";
    }
  else
    {
    cout << "3D model serialization test passed!\n";
    }

  int ModelErrors2D = Test2DModelSerialization();
  if(ModelErrors2D)
    {
    cout << "2D model serialization test failed with " << ModelErrors2D << " errors\n";
    }
  else
    {
    cout << "2D model serialization test passed!\n";
    }


  return ModelErrors3D+ModelErrors2D;
}

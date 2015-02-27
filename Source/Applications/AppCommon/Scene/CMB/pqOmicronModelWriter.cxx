#include "pqOmicronModelWriter.h"

#include "pqCMBFacetedObject.h"
#include "pqCMBSolidMesh.h"
#include <pqSMAdaptor.h>
#include <pqDataRepresentation.h>
#include <vtkSMRepresentationProxy.h>
#include <vtkTransform.h>
#include <pqOutputPort.h>
#include <vtkPVDataInformation.h>
#include <vtksys/SystemTools.hxx>

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkDataObject.h"

pqOmicronModelWriter::pqOmicronModelWriter()
{
  this->AreaConstraint = 0.003;
  this->VolumeConstraint = 0.001;
  this->DiscConstraint = 0.03;
  this->VOI = 0;
  this->Surface = 0;
}

pqOmicronModelWriter::~pqOmicronModelWriter()
{
}

void pqOmicronModelWriter::SetVOI(pqCMBSceneObjectBase *voi)
{
  this->VOI = voi;
}

void pqOmicronModelWriter::AddSolid(pqCMBSceneObjectBase *solid)
{
  this->Solids.push_back(solid);
}

void pqOmicronModelWriter::Write()
{
  // assume setup correctly... for now anyway
  QList<QVariant> position, orientation, scale;
  QList<QVariant> voiPosition, voiOrientation, voiScale;
  double surfaceTranslation[3];

  ofstream fout(this->FileName.c_str());
  if(!fout)
    {
    cerr << "Could not open file " << this->FileName.c_str();
    return;
    }

  if (!this->HeaderComment.empty())
    {
    fout << "# " << this->HeaderComment.c_str() << endl;
    }

  // current omicron code expects that we excute from points directory
  fout << this->SurfaceFileName.c_str() << "\n";

  if (!this->OutputBaseName.empty())
    {
    fout << "Output_Filename: " << this->OutputBaseName.c_str() << endl;
    }

  surfaceTranslation[0] = this->SurfaceInitialTranslation[0] +  this->SurfacePosition[0];
  surfaceTranslation[1] = this->SurfaceInitialTranslation[1] +  this->SurfacePosition[1];
  surfaceTranslation[2] = this->SurfaceInitialTranslation[1] +  this->SurfacePosition[2];

  // the VOI: get points (via bounds) of source outline (since we only use
  // axis-aligned and then transform to get oriented) and transform them
  // to get 4 points of our VOI
  double bounds[6];
  this->VOI->getRepresentation()->getOutputPortFromInput()->
    getDataInformation()->GetBounds(bounds);

  vtkSmartPointer<vtkTransform> transform =
    vtkSmartPointer<vtkTransform>::New();
  this->VOI->getTransform(transform);

  fout << "4 Corners (X_i,Y_i) Coordinates: ";
  double pt[3], transformedPt[3];
  pt[2] = bounds[4];
  for (int i = 0; i < 2; i++)
    {
    pt[0] = bounds[i];
    for (int j = 0; j < 2; j++)
      {
      pt[1] = bounds[2+j];
      transform->TransformPoint(pt, transformedPt);

      fout << transformedPt[0] << " " << transformedPt[1] << " ";
      }
    }
  fout << "\n";

  // Note, rotation is written out as Z, then Y, then X ON PURPOSE
  fout << "(X_translation,Y_translation,Z_translation,rotz,roty,rotx,scale): " <<
    surfaceTranslation[0] << " " <<
    surfaceTranslation[1] << " " <<
    surfaceTranslation[2] << " " <<
    this->SurfaceOrientation[2] << " " <<
    this->SurfaceOrientation[0] << " " <<
    this->SurfaceOrientation[1] << " " <<
    this->SurfaceScale << "\n";

  fout << "(BOTTOM of domain, FACTOR spacing on frame): " << transformedPt[2] << " 1\n";

  fout << "area_constraint: " << this->AreaConstraint << "\n";
  fout << "volume_constraint: " << this->VolumeConstraint << "\n";
  fout << "disc_constraint: " << this->DiscConstraint << "\n";

  fout << "number_of_regions: " << this->Solids.size() << "\n";

  std::vector<pqCMBSceneObjectBase *>::const_iterator solidIter;
  for (solidIter = this->Solids.begin(); solidIter != this->Solids.end(); solidIter++)
    {
    position = pqSMAdaptor::getMultipleElementProperty(
        (*solidIter)->getRepresentation()->getProxy()->GetProperty("Position"));
    orientation = pqSMAdaptor::getMultipleElementProperty(
        (*solidIter)->getRepresentation()->getProxy()->GetProperty("Orientation"));
    scale = pqSMAdaptor::getMultipleElementProperty(
        (*solidIter)->getRepresentation()->getProxy()->GetProperty("Scale"));

    // Note, rotation is written out as Z, then Y, then X ON PURPOSE
    if(pqCMBFacetedObject* facetObj = dynamic_cast<pqCMBFacetedObject*>((*solidIter)))
      {
      fout << facetObj->getFileName() << "\n";
      }
    else if(pqCMBSolidMesh* solMesh = dynamic_cast<pqCMBSolidMesh*>((*solidIter)))
      {
      fout << solMesh->getFileName() << "\n";
      }
    else
      {
      // should we exit, or continue ?;
      continue;
      }
    fout << "(X_translation,Y_translation,Z_translation,rotz,roty,rotx,scale): " <<
      position[0].toDouble() << " " <<
      position[1].toDouble() << " " <<
      position[2].toDouble() << " " <<
      orientation[2].toDouble() << " " <<
      orientation[0].toDouble() << " " <<
      orientation[1].toDouble() << " " <<
      scale[0].toDouble() << "\n";
    }
}

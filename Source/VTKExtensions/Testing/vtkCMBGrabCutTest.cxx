//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkSmartPointer.h"
#include "vtkCMBGrabCutFilter.h"
#include "vtkCMBWatershedFilter.h"
#include "vtkTesting.h"
#include "vtkNew.h"
#include "vtkPNGReader.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkPNGWriter.h"
#include <vtkImageCanvasSource2D.h>
#include <vtkImageViewer2.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkImageViewer2.h>
#include <vtkPropPicker.h>
#include <vtkCornerAnnotation.h>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkInteractorStyle.h>
#include <vtkAssemblyPath.h>
#include <vtkImageActor.h>
#include <vtkMath.h>
#include <vtkVariant.h>
#include <vtkPropPicker.h>
#include <vtkCornerAnnotation.h>
#include <vtkTextProperty.h>
#include <vtkInteractorStyleImage.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkImageBlend.h>
#include <vtkImageMapper3D.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkGDALRasterReader.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkPolyDataMapper.h>

bool LeftButtonPressed = false;
bool ShiftButtonPressed = false;
bool ShiftButtonReleased = false;
int LastPt[2];

int Forground   = 255;
int Background  =   0;
int PotentialBG =  55;
int PotentialFG = 200;

enum DrawMode{ DrawFG, DrawBG } Mode = DrawFG;

class vtkDEMImageCanvasSource2D : public vtkImageCanvasSource2D
{
public:
  static vtkDEMImageCanvasSource2D *New()
  {
    return new vtkDEMImageCanvasSource2D;
  }

  void SetOrigin(double * o)
  {
    Origin[0] = o[0];
    Origin[1] = o[1];
    Origin[2] = o[2];
    this->Modified();
    this->ImageData->SetOrigin(o);
  }

  void SetSpacing(double * s)
  {
    Spacing[0] = s[0];
    Spacing[1] = s[1];
    Spacing[2] = s[2];
    this->Modified();
    this->ImageData->SetSpacing(s);
  }

protected:
  virtual int RequestInformation (vtkInformation *request,
                                  vtkInformationVector** inputVector,
                                  vtkInformationVector *outputVector)
  {
    vtkInformation* outInfo = outputVector->GetInformationObject(0);

    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
                 this->WholeExtent,6);

    outInfo->Set(vtkDataObject::SPACING(), Spacing[0], Spacing[1], Spacing[2] );
    outInfo->Set(vtkDataObject::ORIGIN(), Origin[0], Origin[1], Origin[2]);

    vtkDataObject::SetPointDataActiveScalarInfo(outInfo, this->ImageData->GetScalarType(),
                                                this->ImageData->GetNumberOfScalarComponents());
    return 1;
  }
  vtkDEMImageCanvasSource2D():vtkImageCanvasSource2D()
  {}
  ~vtkDEMImageCanvasSource2D()
  {
    //this->ImageData->Delete();
  }
  double Origin[3];
  double Spacing[3];

};


// Template for image value reading
template<typename T>
void vtkValueMessageTemplate(vtkImageData* image, int* position,
                             std::string& message)
{
  T* tuple = ((T*)image->GetScalarPointer(position));
  int components = image->GetNumberOfScalarComponents();
  if(tuple == NULL) return;
  for (int c = 0; c < components; ++c)
  {
    message += vtkVariant(tuple[c]).ToString();
    if (c != (components - 1))
    {
      message += ", ";
    }
  }
  message += " )";
}

class vtkCMBKeyStrokeRelease : public vtkCommand
{
public:
  static vtkCMBKeyStrokeRelease *New()
  {
    return new vtkCMBKeyStrokeRelease;
  }
  virtual void Execute(vtkObject *, unsigned long vtkNotUsed(event), void *)
  {
    vtkRenderWindowInteractor *interactor = this->Viewer->GetRenderWindow()->GetInteractor();
    vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());
    style->OnKeyRelease();
    ShiftButtonPressed = false;
    ShiftButtonReleased = true;
  }
  vtkImageViewer2*      Viewer;      // Pointer to the viewer
};

// The mouse motion callback, to pick the image and recover pixel values
class vtkCMBKeyStroke : public vtkCommand
{
public:
  static vtkCMBKeyStroke *New()
  {
    return new vtkCMBKeyStroke;
  }

  void SetPicker(vtkPropPicker *picker)
  {
    this->Picker = picker;
  }

  void SetViewer(vtkImageViewer2 *viewer)
  {
    this->Viewer = viewer;
  }

  void SetDrawer(vtkImageCanvasSource2D* d)
  {
    this->Drawer = d;
  }

  virtual void Execute(vtkObject *, unsigned long vtkNotUsed(event), void *)
  {
    vtkRenderWindowInteractor *interactor = this->Viewer->GetRenderWindow()->GetInteractor();
    vtkRenderer* renderer = this->Viewer->GetRenderer();
    vtkImageActor* actor = this->Viewer->GetImageActor();
    vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());

    std::string key = interactor->GetKeySym();
    // Output the key that was pressed
    std::cout << "Key: " << key << std::endl;

    // Handle an arrow key
    if(key == "F" || key == "f")
    {
      std::cout << "Set to forground" << std::endl;
      Drawer->SetDrawColor(Forground, Forground, Forground, 255.0);
    }
    else if(key == "B" || key == "b")
    {
      std::cout << "Set to background" << std::endl;
      Drawer->SetDrawColor(Background, Background, Background, 255.0);
    }
    else if(key == "D" || key == "d")
    {
      std::cout << "Set to erase" << std::endl;
      Drawer->SetDrawColor(PotentialBG, PotentialBG, PotentialBG, 0.0);
    }
    else if(key == "N" || key == "n")
    {
      //run grab cuts
      std::cout << "Run grab cuts" << std::endl;
      filter->DoGrabCut();
      filter->Update();
      writer->Write();
      writer_label->Write();
      writer_next->Write();
      //TODO UPDATE the drawer
      vtkImageData* updateMask = filter->GetOutput(1);
      vtkImageData* currentMask = Drawer->GetOutput();
      int* dims = updateMask->GetDimensions();
      double color[4];
      Drawer->GetDrawColor(color);
      for (int z = 0; z < dims[2]; z++)
      {
        for (int y = 0; y < dims[1]; y++)
        {
          for (int x = 0; x < dims[0]; x++)
          {
            double tmpC[] = {updateMask->GetScalarComponentAsFloat( x, y, z, 0),
                             updateMask->GetScalarComponentAsFloat( x, y, z, 0),
                             updateMask->GetScalarComponentAsFloat( x, y, z, 0),
                             currentMask->GetScalarComponentAsFloat( x, y, z, 3) };
            Drawer->SetDrawColor(tmpC);
            Drawer->DrawPoint(x,y);
          }
        }
      }
      Drawer->SetDrawColor(color);
    }
    else if (key.find("Shift") != std::string::npos)
    {
      ShiftButtonPressed = true;
      ShiftButtonReleased = false;
      style->OnKeyPress();
    }
    else
    {
      style->OnKeyPress();
    }

    // Forward events
  }

  vtkCMBGrabCutFilter * filter;

  vtkXMLPolyDataWriter * writer;
  vtkPNGWriter * writer_label;
  vtkPNGWriter * writer_next;

private:
  vtkImageViewer2*      Viewer;      // Pointer to the viewer
  vtkPropPicker*        Picker;      // Pointer to the picker
  vtkImageCanvasSource2D* Drawer;
};

// The mouse motion callback, to pick the image and recover pixel values
class vtkCMBGrabLeftMouseReleasedCallback : public vtkCommand
{
public:
  static vtkCMBGrabLeftMouseReleasedCallback *New()
  {
    return new vtkCMBGrabLeftMouseReleasedCallback;
  }

  void SetPicker(vtkPropPicker *picker)
  {
    this->Picker = picker;
  }

  void SetViewer(vtkImageViewer2 *viewer)
  {
    this->Viewer = viewer;
  }

  void SetDrawer(vtkImageCanvasSource2D* d)
  {
    this->Drawer = d;
  }

  virtual void Execute(vtkObject *, unsigned long vtkNotUsed(event), void *)
  {
    if(!LeftButtonPressed) return;
    vtkRenderWindowInteractor *interactor = this->Viewer->GetRenderWindow()->GetInteractor();
    vtkRenderer* renderer = this->Viewer->GetRenderer();
    vtkImageActor* actor = this->Viewer->GetImageActor();
    vtkImageData* image = this->Viewer->GetInput();
    vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());

    // Pick at the mouse location provided by the interactor
    this->Picker->Pick(interactor->GetEventPosition()[0],
                       interactor->GetEventPosition()[1],
                       0.0, renderer);

    // Get the world coordinates of the pick
    double pos[3];
    this->Picker->GetPickPosition(pos);

    double origin[3];
    double spacing[3];
    int dim[3];

    image->GetOrigin(origin);
    image->GetSpacing(spacing);
    image->GetDimensions(dim);

    int image_coordinate[] = {  (int)(0.5 + (pos[0] - origin[0]) / spacing[0]),
                                (int)(0.5 + (pos[1] - origin[1]) / spacing[1]),
                                0 };

    Drawer->FillTube(LastPt[0], LastPt[1], image_coordinate[0], image_coordinate[1], 3);
    style->OnLeftButtonUp();
    LeftButtonPressed = false;
  }

private:
  vtkImageViewer2*      Viewer;      // Pointer to the viewer
  vtkPropPicker*        Picker;      // Pointer to the picker
  vtkImageCanvasSource2D* Drawer;
};

// The mouse motion callback, to pick the image and recover pixel values
class vtkCMBGrabLeftMousePressCallback : public vtkCommand
{
public:
  static vtkCMBGrabLeftMousePressCallback *New()
  {
    return new vtkCMBGrabLeftMousePressCallback;
  }

  vtkCMBGrabLeftMousePressCallback()
  {
    this->Viewer     = NULL;
    this->Picker     = NULL;
  }

  ~vtkCMBGrabLeftMousePressCallback()
  {
    this->Viewer     = NULL;
    this->Picker     = NULL;
  }

  void SetPicker(vtkPropPicker *picker)
  {
    this->Picker = picker;
  }

  void SetViewer(vtkImageViewer2 *viewer)
  {
    this->Viewer = viewer;
  }

  void SetDrawer(vtkImageCanvasSource2D* d)
  {
    this->Drawer = d;
  }

  virtual void Execute(vtkObject *, unsigned long vtkNotUsed(event), void *)
  {
    vtkRenderWindowInteractor *interactor = this->Viewer->GetRenderWindow()->GetInteractor();
    vtkRenderer* renderer = this->Viewer->GetRenderer();
    vtkImageActor* actor = this->Viewer->GetImageActor();
    vtkImageData* image = this->Viewer->GetInput();
    vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());


    // Pick at the mouse location provided by the interactor
    this->Picker->Pick(interactor->GetEventPosition()[0],
                       interactor->GetEventPosition()[1],
                       0.0, renderer);

    // There could be other props assigned to this picker, so
    // make sure we picked the image actor
    vtkAssemblyPath* path = this->Picker->GetPath();
    bool validPick = false;

    if (path)
    {
      vtkCollectionSimpleIterator sit;
      path->InitTraversal(sit);
      vtkAssemblyNode *node;
      for (int i = 0; i < path->GetNumberOfItems() && !validPick; ++i)
      {
        node = path->GetNextNode(sit);
        if (actor == vtkImageActor::SafeDownCast(node->GetViewProp()))
        {
          validPick = true;
        }
      }
    }

    if (!validPick)
    {
      style->OnLeftButtonDown();
      return;
    }

    if (ShiftButtonPressed )
    {
      style->OnLeftButtonDown();
      return;
    }

    /*if(ShiftButtonReleased)
    {
      ShiftButtonReleased = false;
      style->OnLeftButtonDown();
      return;
    }*/

    // Get the world coordinates of the pick
    double pos[3];
    this->Picker->GetPickPosition(pos);

    double origin[3];
    double spacing[3];
    int dim[3];

    image->GetOrigin(origin);
    image->GetSpacing(spacing);
    image->GetDimensions(dim);

    int image_coordinate[] = {  (int)(0.5 + (pos[0] - origin[0]) / spacing[0]),
                                (int)(0.5 + (pos[1] - origin[1]) / spacing[1]),
                                0 };
    if (image_coordinate[0] < 0 || image_coordinate[1] < 0)
    {
      // Pass the event further on
      style->OnLeftButtonDown();
      return;
    }

    Drawer->FillBox(image_coordinate[0]-1, image_coordinate[0]+1,
                    image_coordinate[1]-1, image_coordinate[1]+1);
    LeftButtonPressed = true;
    LastPt[0] = image_coordinate[0];
    LastPt[1] = image_coordinate[1];

    interactor->Render();
    style->OnLeftButtonDown();
  }

private:
  vtkImageViewer2*      Viewer;      // Pointer to the viewer
  vtkPropPicker*        Picker;      // Pointer to the picker
  vtkImageCanvasSource2D* Drawer;
};

// The mouse motion callback, to pick the image and recover pixel values
class vtkCMBGrabMouseMoveCallback : public vtkCommand
{
public:
  static vtkCMBGrabMouseMoveCallback *New()
  {
    return new vtkCMBGrabMouseMoveCallback;
  }

  vtkCMBGrabMouseMoveCallback()
  {
    this->Viewer     = NULL;
    this->Picker     = NULL;
    this->Annotation = NULL;
  }

  ~vtkCMBGrabMouseMoveCallback()
  {
    this->Viewer     = NULL;
    this->Picker     = NULL;
    this->Annotation = NULL;
  }

  void SetPicker(vtkPropPicker *picker)
  {
    this->Picker = picker;
  }

  void SetAnnotation(vtkCornerAnnotation *annotation)
  {
    this->Annotation = annotation;
  }

  void SetViewer(vtkImageViewer2 *viewer)
  {
    this->Viewer = viewer;
  }

  void SetDrawer(vtkImageCanvasSource2D* d)
  {
    this->Drawer = d;
  }

  virtual void Execute(vtkObject *, unsigned long vtkNotUsed(event), void *)
  {
    vtkRenderWindowInteractor *interactor = this->Viewer->GetRenderWindow()->GetInteractor();
    vtkRenderer* renderer = this->Viewer->GetRenderer();
    vtkImageActor* actor = this->Viewer->GetImageActor();
    vtkImageData* image = this->Viewer->GetInput();
    vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());


    // Pick at the mouse location provided by the interactor
    this->Picker->Pick(interactor->GetEventPosition()[0],
                       interactor->GetEventPosition()[1],
                       0.0, renderer);

    // There could be other props assigned to this picker, so
    // make sure we picked the image actor
    vtkAssemblyPath* path = this->Picker->GetPath();
    bool validPick = false;

    if (path)
    {
      vtkCollectionSimpleIterator sit;
      path->InitTraversal(sit);
      vtkAssemblyNode *node;
      for (int i = 0; i < path->GetNumberOfItems() && !validPick; ++i)
      {
        node = path->GetNextNode(sit);
        if (actor == vtkImageActor::SafeDownCast(node->GetViewProp()))
        {
          validPick = true;
        }
      }
    }

    if (!validPick)
    {
      this->Annotation->SetText(0, "Off Image");
      interactor->Render();
      // Pass the event further on
      if(!LeftButtonPressed) style->OnMouseMove();
      return;
    }

    // Get the world coordinates of the pick
    double pos[3];
    this->Picker->GetPickPosition(pos);

    double origin[3];
    double spacing[3];
    int dim[3];

    image->GetOrigin(origin);
    image->GetSpacing(spacing);
    image->GetDimensions(dim);

    int image_coordinate[] = {  (int)(0.5 + (pos[0] - origin[0]) / spacing[0]),
                                (int)(0.5 + (pos[1] - origin[1]) / spacing[1]),
                                0 };
    if (image_coordinate[0] < 0 || image_coordinate[1] < 0)
    {
      this->Annotation->SetText(0, "Off Image");
      interactor->Render();
      // Pass the event further on
      style->OnMouseMove();
      return;
    }

    std::string message = "Location: ( ";
    message += vtkVariant(image_coordinate[0]).ToString();
    message += ", ";
    message += vtkVariant(image_coordinate[1]).ToString();
    message += ", ";
    message += vtkVariant(image_coordinate[2]).ToString();
    message += " )\nValue: ( ";

    switch (image->GetScalarType())
    {
      vtkTemplateMacro((vtkValueMessageTemplate<VTK_TT>(image,
                                                        image_coordinate,
                                                        message)));

      default:
        return;
    }

    this->Annotation->SetText( 0, message.c_str() );
    interactor->Render();
    if(LeftButtonPressed)
    {
      Drawer->FillTube(LastPt[0], LastPt[1], image_coordinate[0], image_coordinate[1], 3);
      LastPt[0] = image_coordinate[0];
      LastPt[1] = image_coordinate[1];
    }
    else
    {
      style->OnMouseMove();
    }
  }

private:
  vtkImageViewer2*      Viewer;      // Pointer to the viewer
  vtkPropPicker*        Picker;      // Pointer to the picker
  vtkCornerAnnotation*  Annotation;  // Pointer to the annotation
  vtkImageCanvasSource2D* Drawer;
};

int main(int argc, char** argv)
{

  if ( argc != 2 )
  {
    std::cout << "Usage: " << argv[0]
    << " InputFilename" << std::endl;
    return EXIT_FAILURE;
  }

  vtkSmartPointer<vtkGDALRasterReader> gdal_reader = vtkSmartPointer<vtkGDALRasterReader>::New();
  gdal_reader->SetFileName(argv[1]);
  gdal_reader->Update();
  vtkImageData* image = gdal_reader->GetOutput();

  vtkSmartPointer<vtkDEMImageCanvasSource2D> drawing = vtkSmartPointer<vtkDEMImageCanvasSource2D>::New();
  drawing->SetNumberOfScalarComponents(4);
  drawing->SetScalarTypeToUnsignedChar();
  drawing->SetExtent(image->GetExtent());
  drawing->SetOrigin(image->GetOrigin());
  drawing->SetSpacing(image->GetSpacing());
  drawing->SetDrawColor(PotentialBG, PotentialBG, PotentialBG, 0.0);
  drawing->FillBox(image->GetExtent()[0], image->GetExtent()[1],
                   image->GetExtent()[2], image->GetExtent()[3]);
  drawing->SetDrawColor(Forground, Forground, Forground, 255.0);

  vtkSmartPointer<vtkCMBGrabCutFilter> filter = vtkSmartPointer<vtkCMBGrabCutFilter>::New();
  //vtkSmartPointer<vtkCMBWatershedFilter> filter = vtkSmartPointer<vtkCMBWatershedFilter>::New();
  filter->SetInputConnection(0, gdal_reader->GetOutputPort());
  filter->SetInputConnection(1, drawing->GetOutputPort());
  filter->SetNumberOfIterations(24);
  filter->SetPotentialForegroundValue(PotentialFG);
  filter->SetPotentialBackgroundValue(PotentialBG);
  filter->SetForegroundValue(Forground);
  filter->SetBackgroundValue(Background);

  vtkSmartPointer<vtkXMLPolyDataWriter> writer =vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writer->SetFileName("/Users/jacobbecker/Downloads/testWorld2.vtp");
  writer->SetInputConnection(filter->GetOutputPort(2));
  vtkSmartPointer<vtkPNGWriter> writer_label = vtkSmartPointer<vtkPNGWriter>::New();
  writer_label->SetFileName("/Users/jacobbecker/Downloads/testWorldLabel2.png");
  writer_label->SetInputConnection(filter->GetOutputPort(0));
  vtkSmartPointer<vtkPNGWriter> writer_next = vtkSmartPointer<vtkPNGWriter>::New();
  writer_next->SetFileName("/Users/jacobbecker/Downloads/testWorldNext2.png");
  writer_next->SetInputConnection(filter->GetOutputPort(1));

  vtkSmartPointer<vtkImageActor> maskActor = vtkSmartPointer<vtkImageActor>::New();
  maskActor->GetMapper()->SetInputConnection(drawing->GetOutputPort());

  // Display the result
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();

  vtkSmartPointer<vtkImageViewer2> imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
  imageViewer->SetInputConnection(gdal_reader->GetOutputPort());
  imageViewer->SetupInteractor(renderWindowInteractor);
  imageViewer->GetRenderer()->ResetCamera();
  imageViewer->GetRenderer()->SetBackground(0,0,0);

  imageViewer->GetRenderer()->AddActor(maskActor);

  vtkSmartPointer<vtkPolyDataMapper> lineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  lineMapper->SetInputConnection(filter->GetOutputPort(2));
  vtkSmartPointer<vtkActor> lineActor = vtkSmartPointer<vtkActor>::New();
  lineActor->SetMapper(lineMapper);
  imageViewer->GetRenderer()->AddActor(lineActor);
  imageViewer->GetRenderer()->ResetCamera();

  // Picker to pick pixels
  vtkSmartPointer<vtkPropPicker> propPicker = vtkSmartPointer<vtkPropPicker>::New();
  propPicker->PickFromListOn();

  vtkImageActor* imageActor = imageViewer->GetImageActor();
  propPicker->AddPickList(imageActor);

  imageActor->InterpolateOff();

  // Annotate the image with window/level and mouse over pixel
  // information
  vtkSmartPointer<vtkCornerAnnotation> cornerAnnotation = vtkSmartPointer<vtkCornerAnnotation>::New();
  cornerAnnotation->SetLinearFontScaleFactor(2);
  cornerAnnotation->SetNonlinearFontScaleFactor(1);
  cornerAnnotation->SetMaximumFontSize(20);
  cornerAnnotation->SetText(0, "Off Image");
  cornerAnnotation->SetText(3, "<window>\n<level>");
  cornerAnnotation->GetTextProperty()->SetColor(1, 0, 0);

  imageViewer->GetRenderer()->AddViewProp(cornerAnnotation);

  // Callback listens to MouseMoveEvents invoked by the interactor's style
  vtkSmartPointer<vtkCMBGrabMouseMoveCallback> moveCallback = vtkSmartPointer<vtkCMBGrabMouseMoveCallback>::New();
  moveCallback->SetViewer(imageViewer);
  moveCallback->SetAnnotation(cornerAnnotation);
  moveCallback->SetPicker(propPicker);
  moveCallback->SetDrawer(drawing);

  vtkSmartPointer<vtkCMBGrabLeftMousePressCallback> pressCallback = vtkSmartPointer<vtkCMBGrabLeftMousePressCallback>::New();
  pressCallback->SetViewer(imageViewer);
  pressCallback->SetPicker(propPicker);
  pressCallback->SetDrawer(drawing);

  vtkSmartPointer<vtkCMBKeyStroke> keystroke = vtkSmartPointer<vtkCMBKeyStroke>::New();
  keystroke->SetViewer(imageViewer);
  keystroke->SetPicker(propPicker);
  keystroke->SetDrawer(drawing);

  keystroke->filter = filter;
  keystroke->writer = writer;
  keystroke->writer_label = writer_label;
  keystroke->writer_next  = writer_next;

  // InteractorStyleImage allows for the following controls:
  // 1) middle mouse + move = camera pan
  // 2) left mouse + move = window/level
  // 3) right mouse + move = camera zoom
  // 4) middle mouse wheel scroll = zoom
  // 5) 'r' = reset window/level
  // 6) shift + 'r' = reset camera
  vtkInteractorStyleImage* imageStyle = imageViewer->GetInteractorStyle();
  imageStyle->AddObserver(vtkCommand::MouseMoveEvent, moveCallback);
  imageStyle->AddObserver(vtkCommand::LeftButtonPressEvent, pressCallback);
  imageStyle->AddObserver(vtkCommand::KeyPressEvent, keystroke);
  vtkSmartPointer<vtkCMBKeyStrokeRelease> kr = vtkSmartPointer<vtkCMBKeyStrokeRelease>::New();
  kr->Viewer = imageViewer;
  imageStyle->AddObserver(vtkCommand::KeyReleaseEvent, kr);
  vtkSmartPointer<vtkCMBGrabLeftMouseReleasedCallback> release = vtkSmartPointer<vtkCMBGrabLeftMouseReleasedCallback>::New();
  release->SetViewer(imageViewer);
  release->SetPicker(propPicker);
  release->SetDrawer(drawing);
  imageStyle->AddObserver(vtkCommand::LeftButtonReleaseEvent, release);

  renderWindowInteractor->Initialize();
  renderWindowInteractor->Start();

  return 0;
}

#include "vtkCMBGrabCutUI.h"
#include "ui_grabCuts.h"

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
#include <vtkRenderer.h>
#include <vtkImageViewer2.h>
#include <vtkPropPicker.h>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkInteractorStyle.h>
#include <vtkAssemblyPath.h>
#include <vtkImageActor.h>
#include <vtkMath.h>
#include <vtkVariant.h>
#include <vtkPropPicker.h>
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
#include <vtkProperty.h>

#include <QFileDialog>

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
  {}
  double Origin[3];
  double Spacing[3];
};

class vtkCMBGrabCutUI::Internal
{
public:
  Internal()
  {
    PotAlpha = 0;
    Alpha = 255;
    Radius = 3;
    UpdatePotAlpha = false;
    Forground   = 255;
    Background  =   0;
    PotentialBG =  55;
    PotentialFG = 200;
    maskActor   = vtkSmartPointer<vtkImageActor>::New();
    imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
    drawing     = vtkSmartPointer<vtkDEMImageCanvasSource2D>::New();
    filter      = vtkSmartPointer<vtkCMBGrabCutFilter>::New();

    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();

    imageViewer->SetInputData(image);
    imageViewer->GetRenderer()->ResetCamera();
    imageViewer->GetRenderer()->SetBackground(0,0,0);

    imageViewer->GetRenderer()->AddActor(maskActor);

    filter->SetInputData(0, image);
    filter->SetInputConnection(1, drawing->GetOutputPort());
    filter->SetNumberOfIterations(24);
    filter->SetPotentialForegroundValue(PotentialFG);
    filter->SetPotentialBackgroundValue(PotentialBG);
    filter->SetForegroundValue(Forground);
    filter->SetBackgroundValue(Background);

    lineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    lineMapper->SetInputConnection(filter->GetOutputPort(2));
    lineActor = vtkSmartPointer<vtkActor>::New();
    lineActor->GetProperty()->SetColor(1.0, 1.0, 0.0);
    lineActor->SetMapper(lineMapper);

    imageViewer->GetRenderer()->AddActor(lineActor);

    maskActor->GetMapper()->SetInputConnection(drawing->GetOutputPort());

    propPicker = vtkSmartPointer<vtkPropPicker>::New();
    propPicker->PickFromListOn();
    vtkImageActor* imageActor = imageViewer->GetImageActor();
    propPicker->AddPickList(imageActor);
  }

  int Alpha;
  int PotAlpha;
  int Forground;
  int Background;
  int PotentialBG;
  int PotentialFG;
  int Radius;
  vtkSmartPointer<vtkImageActor> maskActor;
  vtkSmartPointer<vtkDEMImageCanvasSource2D> drawing;
  vtkSmartPointer<vtkCMBGrabCutFilter> filter;
  vtkSmartPointer<vtkPolyDataMapper> lineMapper;
  vtkSmartPointer<vtkActor> lineActor;
  vtkSmartPointer<vtkImageViewer2> imageViewer;
  vtkSmartPointer<vtkPropPicker> propPicker;

  bool leftMousePressed;
  double LastPt[2];

  bool UpdatePotAlpha;

  void updateAlphas()
  {
    double currentColor[4];
    drawing->GetDrawColor(currentColor);
    vtkImageData * image = drawing->GetOutput();
    int* dims = image->GetDimensions();
    for (int z = 0; z < dims[2]; z++)
    {
      for (int y = 0; y < dims[1]; y++)
      {
        for (int x = 0; x < dims[0]; x++)
        {
          double pclass = image->GetScalarComponentAsDouble(x, y, z, 0);
          if(pclass == this->Forground)
          {
            drawing->SetDrawColor(this->Forground, this->Forground,
                                  this->Forground, this->Alpha);
          }
          else if(pclass == this->Background)
          {
            drawing->SetDrawColor(this->Background, this->Background,
                                  this->Background, this->Alpha);
          }
          else if(pclass == this->PotentialFG)
          {
            drawing->SetDrawColor(this->PotentialFG, this->PotentialFG,
                                  this->PotentialFG, this->PotAlpha);
          }
          else if(pclass == this->PotentialBG)
          {
            drawing->SetDrawColor(this->PotentialBG, this->PotentialBG,
                                  this->PotentialBG, this->PotAlpha);
          }
          else
          {
            std::cout << "Unknown class " << pclass << std::endl;
            continue;
          }
          drawing->DrawPoint(x,y);
        }
      }
    }
    drawing->SetDrawColor(currentColor);
    vtkRenderWindowInteractor *interactor = this->imageViewer->GetRenderWindow()->GetInteractor();
    interactor->Render();
  }
};

// The mouse motion callback, to pick the image and recover pixel values
class vtkCMBGrabLeftMouseReleasedCallback : public vtkCommand
{
public:
  static vtkCMBGrabLeftMouseReleasedCallback *New()
  {
    return new vtkCMBGrabLeftMouseReleasedCallback;
  }

  void SetData(vtkCMBGrabCutUI::Internal *i)
  {
    this->internal = i;
  }

  virtual void Execute(vtkObject *, unsigned long vtkNotUsed(event), void *)
  {
    vtkRenderWindowInteractor *interactor = internal->imageViewer->GetRenderWindow()->GetInteractor();
    vtkRenderer* renderer = internal->imageViewer->GetRenderer();
    vtkImageActor* actor = internal->imageViewer->GetImageActor();
    vtkImageData* image = internal->imageViewer->GetInput();
    vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());

    if(!internal->leftMousePressed)
    {
      style->OnLeftButtonUp();
      return;
    }

    // Pick at the mouse location provided by the interactor
    internal->propPicker->Pick(interactor->GetEventPosition()[0],
                               interactor->GetEventPosition()[1],
                               0.0, renderer);

    // Get the world coordinates of the pick
    double pos[3];
    internal->propPicker->GetPickPosition(pos);

    double origin[3];
    double spacing[3];
    int dim[3];

    image->GetOrigin(origin);
    image->GetSpacing(spacing);
    image->GetDimensions(dim);

    int image_coordinate[] = { (int)(0.5 + (pos[0] - origin[0]) / spacing[0]),
                               (int)(0.5 + (pos[1] - origin[1]) / spacing[1]),
                               0 };

    internal->drawing->FillTube(internal->LastPt[0], internal->LastPt[1],
                                image_coordinate[0], image_coordinate[1], internal->Radius);
    style->OnLeftButtonUp();
    internal->leftMousePressed = false;
  }

private:
  vtkCMBGrabCutUI::Internal *internal;
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
    this->internal     = NULL;
  }

  ~vtkCMBGrabMouseMoveCallback()
  {
    this->internal     = NULL;
  }

  void SetData(vtkCMBGrabCutUI::Internal *i)
  {
    this->internal = i;
  }

  virtual void Execute(vtkObject *, unsigned long vtkNotUsed(event), void *)
  {
    vtkRenderWindowInteractor *interactor = internal->imageViewer->GetRenderWindow()->GetInteractor();
    vtkRenderer* renderer = internal->imageViewer->GetRenderer();
    vtkImageActor* actor = internal->imageViewer->GetImageActor();
    vtkImageData* image = internal->imageViewer->GetInput();
    vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());

    if(!internal->leftMousePressed)
    {
      style->OnMouseMove();
      return;
    }

    // Pick at the mouse location provided by the interactor
    internal->propPicker->Pick(interactor->GetEventPosition()[0],
                               interactor->GetEventPosition()[1],
                               0.0, renderer);

    // There could be other props assigned to this picker, so
    // make sure we picked the image actor
    vtkAssemblyPath* path = internal->propPicker->GetPath();
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
      return;
    }

    // Get the world coordinates of the pick
    double pos[3];
    internal->propPicker->GetPickPosition(pos);

    double origin[3];
    double spacing[3];
    int dim[3];

    image->GetOrigin(origin);
    image->GetSpacing(spacing);
    image->GetDimensions(dim);

    int image_coordinate[] = { (int)(0.5 + (pos[0] - origin[0]) / spacing[0]),
                               (int)(0.5 + (pos[1] - origin[1]) / spacing[1]),
                               0 };
    if (image_coordinate[0] < 0 || image_coordinate[1] < 0)
    {
      style->OnMouseMove();
      return;
    }

    interactor->Render();
    internal->drawing->FillTube(internal->LastPt[0], internal->LastPt[1],
                                image_coordinate[0], image_coordinate[1], internal->Radius);
    internal->LastPt[0] = image_coordinate[0];
    internal->LastPt[1] = image_coordinate[1];
  }
private:
  vtkCMBGrabCutUI::Internal *internal;
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
    this->internal     = NULL;
  }

  ~vtkCMBGrabLeftMousePressCallback()
  {
    this->internal     = NULL;
  }

  void SetData(vtkCMBGrabCutUI::Internal *i)
  {
    this->internal = i;
  }

  virtual void Execute(vtkObject *, unsigned long vtkNotUsed(event), void *)
  {
    //std::cout << "Mouse pressed" << std::endl;
    internal->leftMousePressed = true;
    vtkRenderWindowInteractor *interactor = internal->imageViewer->GetRenderWindow()->GetInteractor();
    vtkRenderer* renderer = internal->imageViewer->GetRenderer();
    vtkImageActor* actor = internal->imageViewer->GetImageActor();
    vtkImageData* image = internal->imageViewer->GetInput();
    vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());

    // Pick at the mouse location provided by the interactor
    this->internal->propPicker->Pick(interactor->GetEventPosition()[0],
                                     interactor->GetEventPosition()[1],
                                     0.0, renderer);

    // There could be other props assigned to this picker, so
    // make sure we picked the image actor
    vtkAssemblyPath* path = internal->propPicker->GetPath();
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

    //if (ShiftButtonPressed )
    //{
    //  style->OnLeftButtonDown();
    //  return;
    //}

    /*if(ShiftButtonReleased)
     {
     ShiftButtonReleased = false;
     style->OnLeftButtonDown();
     return;
     }*/

    // Get the world coordinates of the pick
    double pos[3];
    internal->propPicker->GetPickPosition(pos);

    double origin[3];
    double spacing[3];
    int dim[3];

    image->GetOrigin(origin);
    image->GetSpacing(spacing);
    image->GetDimensions(dim);

    int image_coordinate[] = { (int)(0.5 + (pos[0] - origin[0]) / spacing[0]),
                               (int)(0.5 + (pos[1] - origin[1]) / spacing[1]),
                               0 };
    if (image_coordinate[0] < 0 || image_coordinate[1] < 0)
    {
      // Pass the event further on
      style->OnLeftButtonDown();
      return;
    }

    this->internal->drawing->FillBox(image_coordinate[0]-internal->Radius,
                                     image_coordinate[0]+internal->Radius,
                                     image_coordinate[1]-internal->Radius,
                                     image_coordinate[1]+internal->Radius);
    this->internal->leftMousePressed = true;
    this->internal->LastPt[0] = image_coordinate[0];
    this->internal->LastPt[1] = image_coordinate[1];

    interactor->Render();
    style->OnLeftButtonDown();
  }
  
private:
  vtkCMBGrabCutUI::Internal *internal;
};


vtkCMBGrabCutUI::vtkCMBGrabCutUI()
:internal( new vtkCMBGrabCutUI::Internal() )
{
  this->ui = new Ui_grabCuts;
  this->ui->setupUi(this);
  connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
  connect(this->ui->Open, SIGNAL(clicked()), this, SLOT(open()));
  connect(this->ui->SaveVTP, SIGNAL(clicked()), this, SLOT(saveVTP()));
  connect(this->ui->SaveMask, SIGNAL(clicked()), this, SLOT(saveMask()));
  connect(this->ui->Run, SIGNAL(clicked()), this, SLOT(run()));

  connect(this->ui->NumberOfIter, SIGNAL(valueChanged(int)), this, SLOT(numberOfIterations(int)));
  connect(this->ui->DrawSize, SIGNAL(valueChanged(int)), this, SLOT(pointSize(int)));

  connect(this->ui->DrawMode, SIGNAL(currentIndexChanged(int)), this, SLOT(setDrawMode(int)));

  connect(this->ui->LabelTrans, SIGNAL(valueChanged(int)), this, SLOT(setTransparency(int)));
  connect(this->ui->DrawPossible, SIGNAL(clicked(bool)), this, SLOT(showPossibleLabel(bool)));

  this->ui->qvtkWidget->SetRenderWindow(this->internal->imageViewer->GetRenderWindow());
  this->internal->imageViewer->SetupInteractor(
                                          this->ui->qvtkWidget->GetRenderWindow()->GetInteractor());

  vtkSmartPointer<vtkCMBGrabLeftMousePressCallback> pressCallback =
                                          vtkSmartPointer<vtkCMBGrabLeftMousePressCallback>::New();
  pressCallback->SetData(internal);
  vtkSmartPointer<vtkCMBGrabMouseMoveCallback> moveCallback =
                                                vtkSmartPointer<vtkCMBGrabMouseMoveCallback>::New();
  moveCallback->SetData(internal);
  vtkSmartPointer<vtkCMBGrabLeftMouseReleasedCallback> release =
                                        vtkSmartPointer<vtkCMBGrabLeftMouseReleasedCallback>::New();
  release->SetData(internal);

  vtkInteractorStyleImage* imageStyle = this->internal->imageViewer->GetInteractorStyle();
  imageStyle->AddObserver(vtkCommand::MouseMoveEvent, moveCallback);
  imageStyle->AddObserver(vtkCommand::LeftButtonPressEvent, pressCallback);
  imageStyle->AddObserver(vtkCommand::LeftButtonReleaseEvent, release);
}

vtkCMBGrabCutUI::~vtkCMBGrabCutUI()
{
  delete internal;
  delete ui;
}

void vtkCMBGrabCutUI::slotExit()
{
  qApp->exit();
}

void vtkCMBGrabCutUI::open()
{
  QString fileName = QFileDialog::getOpenFileName(NULL, tr("Open Tiff File"),
                                                  "",
                                                  tr("Tiff file (*.tif)"));
  if(fileName.isEmpty())
  {
    return;
  }
  vtkSmartPointer<vtkGDALRasterReader> gdal_reader;
  gdal_reader = vtkSmartPointer<vtkGDALRasterReader>::New();
  gdal_reader->SetFileName(fileName.toStdString().c_str());
  gdal_reader->Update();
  vtkImageData * image = gdal_reader->GetOutput();
  this->internal->imageViewer->SetInputData(image);
  internal->filter->SetInputData(0, image);
  internal->drawing->SetNumberOfScalarComponents(4);
  internal->drawing->SetScalarTypeToUnsignedChar();
  internal->drawing->SetExtent(image->GetExtent());
  internal->drawing->SetOrigin(image->GetOrigin());
  internal->drawing->SetSpacing(image->GetSpacing());
  internal->drawing->SetDrawColor(internal->PotentialBG, internal->PotentialBG,
                                  internal->PotentialBG, 0.0);
  internal->drawing->FillBox(image->GetExtent()[0], image->GetExtent()[1],
                             image->GetExtent()[2], image->GetExtent()[3]);
  internal->drawing->SetDrawColor(internal->Forground, internal->Forground,
                                  internal->Forground, 255.0);
  internal->imageViewer->GetRenderer()->ResetCamera();

  vtkRenderWindowInteractor *interactor = internal->imageViewer->GetRenderWindow()->GetInteractor();
  interactor->Render();
}

void vtkCMBGrabCutUI::saveVTP()
{
  QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save Current Boundary File"),
                                                  "",
                                                  tr("VTP File (*.vtp)"));
  if(fileName.isEmpty())
  {
    return;
  }

  vtkSmartPointer<vtkXMLPolyDataWriter> writer =vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writer->SetFileName(fileName.toStdString().c_str());
  writer->SetInputConnection(internal->filter->GetOutputPort(2));
  writer->Write();
}

void vtkCMBGrabCutUI::saveMask()
{
  QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save Current binary mask"),
                                                  "",
                                                  tr("png file (*.png)"));
  if(fileName.isEmpty())
  {
    return;
  }

  vtkSmartPointer<vtkPNGWriter> writer_label = vtkSmartPointer<vtkPNGWriter>::New();
  writer_label->SetFileName(fileName.toStdString().c_str());
  vtkImageData * tmp = internal->filter->GetOutput(0);
  writer_label->SetInputData(tmp);
  writer_label->Write();
}

void vtkCMBGrabCutUI::run()
{
  internal->filter->DoGrabCut();
  internal->filter->Update();
  vtkImageData* updateMask = internal->filter->GetOutput(1);
  vtkImageData* currentMask = internal->drawing->GetOutput();

  int* dims = updateMask->GetDimensions();
  double color[4];
  internal->drawing->GetDrawColor(color);
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
        internal->drawing->SetDrawColor(tmpC);
        internal->drawing->DrawPoint(x,y);
      }
    }
  }
  internal->drawing->SetDrawColor(color);
}

void vtkCMBGrabCutUI::pointSize(int i)
{
  internal->Radius = i;
}

void vtkCMBGrabCutUI::numberOfIterations(int j)
{
  internal->filter->SetNumberOfIterations(j);
}

void vtkCMBGrabCutUI::showPossibleLabel(bool b)
{
  internal->UpdatePotAlpha = b;
  if(b)
  {
    internal->PotAlpha = internal->Alpha;
  }
  else
  {
    internal->PotAlpha = 0;
  }
  internal->updateAlphas();
}

void vtkCMBGrabCutUI::setTransparency(int t)
{
  if(t != internal->Alpha)
  {
    internal->Alpha = t;
    if(internal->UpdatePotAlpha)
    {
      internal->PotAlpha = internal->Alpha;
    }
    internal->updateAlphas();
  }
}

void vtkCMBGrabCutUI::setDrawMode(int m)
{
  switch( m )
  {
    case 1:
      internal->drawing->SetDrawColor(internal->Background, internal->Background,
                                      internal->Background, internal->Alpha);
      break;
    case 0:
      internal->drawing->SetDrawColor(internal->Forground, internal->Forground,
                                      internal->Forground, internal->Alpha);
      break;
    case 3:
      internal->drawing->SetDrawColor(internal->PotentialBG, internal->PotentialBG,
                                      internal->PotentialBG, internal->PotAlpha);
      break;
    case 2:
      internal->drawing->SetDrawColor(internal->PotentialFG, internal->PotentialFG,
                                      internal->PotentialFG, internal->PotAlpha);
  }
}

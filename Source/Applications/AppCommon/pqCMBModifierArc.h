#ifndef __pqCMBModifierArc_h
#define __pqCMBModifierArc_h

#include <QObject>
#include <QAbstractItemView>

class qtCMBArcWidget;
class qtCMBArcEditWidget;
class qtCMBArcWidgetManager;
class pqCMBArc;
class pqPipelineSource;
class vtkPiecewiseFunction;
class vtkSMSourceProxy;

class pqCMBModifierArc :  public QObject
{
  Q_OBJECT

public:
  enum RangeLable{ MIN = 0, MAX = 1};
  pqCMBModifierArc();
  ~pqCMBModifierArc();

  pqCMBArc * GetCmbArc()
  { return CmbArc; }

  void setId(int i)
  { Id = i; }

  int getId() const { return Id; }

  void setVisablity(bool vis);

  vtkPiecewiseFunction * getDisplacementProfile();
  vtkPiecewiseFunction * getWeightingFunction();

  double getDisplacementDepth(RangeLable r)
  {
    return DisplacementDepthRange[r];
  }

  double getDistanceRange(RangeLable r)
  {
    return DistanceRange[r];
  }

  bool getSymmetry() const
  {
    return Symmetric;
  }

  bool getRelative() const
  {
    return Relative;
  }

  bool getDisplacementFunctionUseSpline()
  {
    return DispUseSpline;
  }

  bool getWeightingFunctionUseSpline()
  {
    return WeightUseSpline;
  }

  void getDisplacementSplineControl(double&, double&, double&);
  void getWeightingSplineControl(double&, double&, double&);

public slots:
  void setLeftDistance(double dist);
  void setRightDistance(double dist);
  void setMinDisplacementDepth(double d);
  void setMaxDisplacementDepth(double d);
  void setDisplacementFunctionType(bool);
  void setWeightingFunctionType(bool);
  void setSymmetry(bool);
  void setRelative(bool);
  void sendChangeSignals();
  void updateArc(vtkSMSourceProxy* source);
  void switchToNotEditable();
  void switchToEditable();
  void removeFromServer(vtkSMSourceProxy* source);
  bool setCMBArc(pqCMBArc *);
  void setDisplacementSplineControl(double, double, double);
  void setWeightingSplineControl(double,double,double);

signals:
  void functionChanged(int);
  void updateDisplacementProfile(/*Displacent info*/);
  void updateWeightProfile(/*Weight info*/);
  void finishCreating();
  void requestRender();

protected:
  //Varable for the path
  pqCMBArc * CmbArc;
  bool IsExternalArc;
  double DistanceRange[2];
  double DisplacementDepthRange[2];
  double DispSplineControl[3];
  double WeightSplineControl[3];
  vtkPiecewiseFunction * DisplacementProfile;
  vtkPiecewiseFunction * WeightingFunction;
  qtCMBArcEditWidget* Modifier;
  int Id;
  bool Symmetric;
  bool Relative;
  bool IsVisible;
  bool DispUseSpline;
  bool WeightUseSpline;
  void sendRanges(vtkSMSourceProxy*);
};

#endif

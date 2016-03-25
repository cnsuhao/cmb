//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkArcDepressFilter.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkExecutive.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkImplicitFunction.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLine.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkTransform.h"
#include "vtkTriangle.h"
#include "vtkIncrementalPointLocator.h"
#include "vtkPiecewiseFunction.h"
#include <vtkPolyDataNormals.h>
#include "vtkSmartPointer.h"
#include "vtkKochanekSpline.h"

#include <algorithm>
#include <math.h>
#include <limits>
#include <cassert>

#include <boost/shared_ptr.hpp>

vtkStandardNewMacro(vtkArcDepressFilter);

class ArcDepressFunction
{
public:
  ArcDepressFunction() {}
  virtual ~ArcDepressFunction(){}
  virtual double evaluate(double d, double minF, double maxF) = 0;
  virtual double evaluate(double d) = 0;
  virtual void addPoint(double w, double v, double s, double m) = 0;
  virtual void clearPoints() = 0;
};

class ArcDepressPiecewiseFun: public ArcDepressFunction
{
public:
  ArcDepressPiecewiseFun()
    {
      fun = vtkPiecewiseFunction::New();
      fun->SetAllowDuplicateScalars(1);
    }
  virtual ~ArcDepressPiecewiseFun()
  {
    fun->Delete();
  }
  virtual double evaluate(double d, double minF, double maxF)
  {
    double wd = this->evaluate(d);
    return (1-wd)*minF+(wd)*maxF;
  }
  virtual double evaluate(double d)
  {
    return fun->GetValue(d);
  }
  void addPoint(double w, double v, double s, double m)
  {
    fun->AddPoint(w, v, s, m);
  }
  void clearPoints()
  {
    fun->RemoveAllPoints();
  }

  vtkPiecewiseFunction * fun;
};

class ArcDepressSplineFun: public ArcDepressFunction
{
public:
  ArcDepressSplineFun()
  {
    fun = vtkKochanekSpline::New();
    computed= false;
  }
  virtual ~ArcDepressSplineFun()
  {
    fun->Delete();
  }
  virtual double evaluate(double d, double minF, double maxF)
  {
    if(!computed)
    {
      fun->Compute();
      computed = true;
    }

    double wd = fun->Evaluate(d);
    double dist = maxF-minF;
    return minF+(wd)*dist;
  }

  virtual double evaluate(double d)
  {
    if(!computed)
    {
      fun->Compute();
      computed = true;
    }
    double tmp = fun->Evaluate(d);
    if(tmp < 0) tmp = 0;
    else if( tmp > 1) tmp = 1;
    return tmp;
  }

  void addPoint(double w, double v, double /*s*/, double /*m*/ )
  {
    if(fun->GetNumberOfPoints()!=0 && prev == w)
    {
      w += 0.000001;
    }
    fun->AddPoint(w,v);
    prev = w;
  }

  void clearPoints()
  {
    fun->RemoveAllPoints();
    computed = false;
  }
  double prev;
  bool computed;
  vtkKochanekSpline * fun;
};

class DepArcProfileFunction
{
public:
  enum FunctionType{Piecewise = 1, Spline = 0};
  virtual ~DepArcProfileFunction(){}
  virtual bool evalFun( double d, double & weight, double & desp ) const = 0;
  virtual double apply(double d, double pt) const = 0;
  virtual void apply(double d, double *pt, double *n) const = 0;
  virtual void addWeightPoint(double w, double v, double s, double m) = 0;
  virtual bool inside(double d) const = 0;
};

class DepArcManualProfileFunction : public DepArcProfileFunction
{
public:
  enum FunctionEnum{WeightFun = 0, DispFun = 1};
  enum range{MIN = 0,MAX = 1};

  DepArcManualProfileFunction(DepArcProfileFunction::FunctionType despType,
                              DepArcProfileFunction::FunctionType weightFun,
                              bool symmetric, bool relative )
  : IsSymmetric(symmetric), IsRelative(relative)
  {
    SelectedFunction[DispFun] = (despType == DepArcProfileFunction::Piecewise) ?
                                  dynamic_cast<ArcDepressFunction*>(new ArcDepressPiecewiseFun()) :
                                  dynamic_cast<ArcDepressFunction*>(new ArcDepressSplineFun());
    SelectedFunction[WeightFun] = (weightFun == DepArcProfileFunction::Piecewise) ?
                                  dynamic_cast<ArcDepressFunction*>(new ArcDepressPiecewiseFun()) :
                                  dynamic_cast<ArcDepressFunction*>(new ArcDepressSplineFun());

    this->setMinMaxDistance(0, 0);
    this->setMinMaxDesplacementDepth(0, 0);
  }

  ~DepArcManualProfileFunction()
  {
    delete SelectedFunction[0];
    delete SelectedFunction[1];
  }

  virtual void addWeightPoint(double w, double v, double s, double m)
  {
    addPoint(WeightFun, w, v, s, m);
  }

  void addPoint(FunctionEnum e, double w, double v, double s, double m)
  {
    SelectedFunction[e]->addPoint(w,v,s,m);
  }

  bool isSymmetric() const
  {
    return IsSymmetric;
  }

  virtual bool evalFun( double d, double & weight, double & desp ) const
  {
    if(d < getMinDistance() || d > getMaxDistance())
    {
      return false;
    }
    if(d < 0)
    {
      d = d/-getMinDistance();
    }
    else if( d >= 0)
    {
      d = d/getMaxDistance();
    }
    weight = SelectedFunction[WeightFun]->evaluate(d);
    desp = SelectedFunction[DispFun]->evaluate(d, getMinDepth(), getMaxDepth());
    return true;
  }

  virtual double apply(double d, double pt) const
  {
    double w, tmp;
    if(!evalFun(d, w, tmp)) return pt;
    if(IsRelative)
    {
      return pt + w*tmp;
    }
    else
    {
      return w*tmp +(1-w)*pt;
    }
  }

  virtual void apply(double d, double *pt, double *n) const
  {
    double w, tmp;
    if(!evalFun(d, w, tmp)) return;
    if(IsRelative)
    {
      pt[0] = pt[0] + w*tmp*n[0];
      pt[1] = pt[1] + w*tmp*n[1];
      pt[2] = pt[2] + w*tmp*n[2];
    }
    else
    {
      pt[0] = w*tmp*n[0] + (1-w)*pt[0];
      pt[1] = w*tmp*n[1] + (1-w)*pt[1];
      pt[2] = w*tmp*n[2] + (1-w)*pt[2];
    }
  }

  double getMaxDistance() const
  {
    return MinMaxDist[MAX];
  }

  double getMinDistance() const
  {
    return MinMaxDist[MIN];
  }

  double getMaxDepth() const
  {
    return MinMaxDespDepth[MAX];
  }

  double getMinDepth() const
  {
    return MinMaxDespDepth[MIN];
  }

  void setMinMaxDesplacementDepth(double min, double max)
  {
    MinMaxDespDepth[MIN] = min;
    MinMaxDespDepth[MAX] = max;
  }

  void setMinMaxDistance(double left, double right)
  {
    MinMaxDist[MIN] = left;
    MinMaxDist[MAX] = right;
  }

  bool inside(double d) const
  {
    assert(getMaxDistance() != 0);
    if(isSymmetric())
    {
      return std::abs(d) <= getMaxDistance();
    }
    return getMinDistance() <= d && d <= getMaxDistance();
  }

protected:

  bool IsSymmetric;
  bool IsRelative;

  ArcDepressFunction * SelectedFunction[2];
  double MinMaxDespDepth[2];
  double MinMaxDist[2];

};

class DepArcMixProfileFunction: public DepArcProfileFunction
{
public:
  DepArcMixProfileFunction(boost::shared_ptr<DepArcProfileFunction> fun0,
                           boost::shared_ptr<DepArcProfileFunction> fun1, double m)
  :mix(m)
  {
    assert(fun0 != NULL);
    assert(fun1 != NULL);
    fun[0] = fun0;
    fun[1] = fun1;
    assert(mix >0 && mix < 1.0);
  }

  virtual double apply(double d, double pt) const
  {
    return fun[0]->apply(d,pt)*(1-mix) + fun[1]->apply(d, pt) * mix;
  }

  virtual void apply(double d, double *pt, double *n) const
  {
    double pta[3], ptb[3];
    fun[0]->apply(d, pta, n);
    fun[1]->apply(d, ptb, n);
    double t = mix;
    double tm1 = 1-t;
    pt[0] = tm1 * pta[0] + t*ptb[0];
    pt[1] = tm1 * pta[1] + t*ptb[1];
    pt[2] = tm1 * pta[2] + t*ptb[2];
  }

  virtual bool evalFun( double d, double & weight, double & desp ) const
  {
    assert(false && " do not use for the mixing function");
  }

  bool inside(double d) const
  {
    return fun[0]->inside(d) || fun[1]->inside(d);
  }

  virtual void addWeightPoint(double w, double v, double s, double m)
  {
    assert(false && " do not use for the mixing function");
  }

  boost::shared_ptr<DepArcProfileFunction> fun[2];
  double mix;
};

namespace
{

  void cross(double const i1[2], double const i2[2], double r[3])
  {
    r[0] = i1[1] - i2[1];
    r[1] = i2[0] - i1[0];
    r[2] = i1[0]*i2[1] - i1[1]*i2[0];
  }

  double dot(double const i1[2], double const i2[2])
  {
    return i1[0]*i2[0]+i1[1]*i2[1];
  }
  
}

class DepArcData
{
  friend class vtkArcDepressFilter;
private:

  DepArcData()
  :IsEnabled(true)
  {
  }

  ~DepArcData()
  {
    this->clear();
  }

  struct point
  {
    double pt[2];

    void clearPointFunction()
    {
      pointFunction = boost::shared_ptr<DepArcProfileFunction>();
    }

    void setFunction(boost::shared_ptr<DepArcProfileFunction> fun)
    {
      pointFunction = fun;
    }

    boost::shared_ptr<DepArcProfileFunction> getFunction()
    {
      return pointFunction;
    }

    point(point const& pin)
    {
      pt[0] = pin.pt[0];
      pt[1] = pin.pt[1];
      pointFunction = pin.pointFunction;
    }
    explicit point(double x = 0, double y = 0)
    {
      pt[0] = x;
      pt[1] = y;
    }
    ~point()
    {
      clearPointFunction();
    }
    void operator=(point const& pin)
    {
      clearPointFunction();
      pt[0] = pin.pt[0];
      pt[1] = pin.pt[1];
      pointFunction = pin.pointFunction;
    }
    point operator-(point const& pin) const
    {
      point result(pt[0] - pin.pt[0], pt[1] - pin.pt[1]);

      return result;
    }
    point operator+(point const& pin) const
    {
      point result(pt[0] + pin.pt[0], pt[1] + pin.pt[1]);
      return result;
    }
    point operator*(double t) const
    {
      point result(t*pt[0], t*pt[1]);
      return result;
    }
    double operator[](size_t i) const
    {
      return pt[i];
    }
    double normSquared() const
    {
      return this->dot(*this);
    }
    double distSquared(point const& other) const
    {
      point tmp = *this - other;
      return tmp.dot(tmp);
    }
    double dot(point const& other) const
    {
      return this->pt[0]*other.pt[0]+this->pt[1]*other.pt[1];
    }
    bool inside(double d) const
    {
      return pointFunction->inside(d);
    }

    double apply(double d, double pt) const
    {
      assert(pointFunction.get() != NULL);
      return pointFunction->apply(d,pt);
    }

    void apply(double d, double *pt, double *n) const
    {
      assert(pointFunction.get() !=NULL);
      pointFunction->apply(d, pt, n);
    }
  private:
    boost::shared_ptr<DepArcProfileFunction> pointFunction;
  };

  struct line_seg
  {
  public:
    line_seg()
    :pt1(NULL), pt2(NULL)
    {
    }
    line_seg(point *p1, point *p2)
    :pt1(p1), pt2(p2)
    {
      assert(p1 != NULL);
      assert(p2 != NULL);
      dr = *p2 - *p1;
    }

    point getPoint(double t) const
    {
      assert(t<=1.0);
      if(t == 1) return *pt2;
      else if(t == 0) return *pt1;
      point result = *(pt1) + dr * t;

      if(pt2->getFunction() == NULL)
      {
        result.setFunction( pt1->getFunction() );
      }
      else if(pt1->getFunction() == NULL)
      {
        result.setFunction( pt2->getFunction() );
      }
      else
      {
        boost::shared_ptr<DepArcProfileFunction>
          mixxer(new DepArcMixProfileFunction(pt1->getFunction(), pt2->getFunction(), t));
        result.setFunction( mixxer );
      }
      return result;
    }

    point findClosestPoint(point & pt) const
    {
      double l2 = dr.normSquared();
      if(l2 == 0)
      {
        return *pt1;
      }
      point tpt1 = pt - *pt1;
      point tpt2 = this->dr;
      double t = tpt1.dot(tpt2)/l2;
      if(t <= 0.0)
      {
        return *pt1;
      }
      else if(t >= 1.0)
      {
        return *pt2;
      }
      else
      {
        return getPoint(t);
      }
    }

    bool side(point & pt, point & closePt) const
    {
      point cpDir = pt - closePt;
      double sign = dr[1]*cpDir[0] - dr[0]*cpDir[1];
      return sign < 0;
    }

    point const& operator[](size_t i) const
    {
      if(i == 0) return *pt1;
      return *pt2;
    }
    double length()
    {
      return std::sqrt(dr.normSquared());
    }
  private:
    point * pt1;
    point * pt2;
    point dr;
  };


  bool IsEnabled;

  void addPoint(double x, double y)
  {
    size_t at = points.size();
    points.push_back(new point(x,y));
    if(at>=1)
    {
      size_t prev = at-1;
      DepArcData::line_seg l(points[prev],points[at]);
      lines.push_back(l);
    }
  }

  void clear()
  {
    lines.clear();
    for(unsigned int i = 0; i < points.size(); ++i)
    {
      delete points[i];
    }
    points.clear();
    functions.clear();
    assert(lines.empty() && points.empty() && functions.empty());
  }

  bool getDistance(double pin[2], double & result, size_t & lsId, point & resultPt) const
  {
    if(points.empty())
    {
      return false;
    }
    point pt(pin[0], pin[1]);
    point closePt = *(points[0]);
    result = pt.distSquared(closePt);
    line_seg const* cls = NULL;
    lsId = lines.size();
    for (unsigned int i = 0; i<lines.size(); ++i)
    {
      line_seg const& ls = lines[i];
      point tmppt = ls.findClosestPoint(pt);
      double tmp = pt.distSquared(tmppt);
      if(tmp < result)
      {
        result = tmp;
        closePt = tmppt;
        cls = &ls;
        lsId = i;
      }
    }
    result = sqrt(result);
    resultPt = closePt;
    if(closePt.getFunction() == NULL)
    {
      //TODO: FUNCTION
      return false;
    }
    return closePt.inside(result);
  }

  void closeArc()
  {
    if(lines.empty()) return;

    size_t prev = points.size()-1;
    DepArcData::line_seg l(points[prev],points[0]);
    lines.push_back(l);
  }

  void setFunction(size_t b, size_t e, boost::shared_ptr<DepArcProfileFunction> fun)
  {
    for(;b<e; ++b)
    {
      points[b]->setFunction(fun);
    }
  }

  void setInterpolateFunction(size_t b, size_t e)
  {
    assert(points[b]->getFunction() != NULL);
    assert(points[e]->getFunction() != NULL);
    double total_distance = 0;
    if(points[b]->getFunction() == points[e]->getFunction())
    {
      //optimize
      setFunction(b+1,e,points[b]->getFunction());
      return;
    }
    boost::shared_ptr<DepArcProfileFunction> fun0 = points[b]->getFunction();
    boost::shared_ptr<DepArcProfileFunction> fun1 = points[e]->getFunction();

    for (size_t i = b; i<e; ++i)
    {
      total_distance += lines[i].length();
    }
    double currentLength = lines[b].length();
    for (size_t i = b+1; i<e; ++i)
    {
      double t = currentLength/total_distance;
      boost::shared_ptr<DepArcMixProfileFunction> mix(new DepArcMixProfileFunction(fun0, fun1, t));
      points[i]->setFunction(mix);
      currentLength += lines[i].length();
    }
  }

  void setUpFunctions()
  {
    size_t b = 0;
    while ( b < points.size() && points[b]->getFunction() == NULL)
    {
      b++;
    }
    if(b == points.size()) return;
    if(b != 0)
    {
      setFunction(0,b,points[b]->getFunction());
    }
    while (b != points.size())
    {
      assert(points[b]->getFunction() != NULL);
      size_t e = b+1;
      while( e < points.size() && points[e]->getFunction() == NULL )
      {
        e++;
      }
      if( e == points.size())
      {
        setFunction(b+1,e,points[b]->getFunction());
      }
      else if( e != b+1)
      {
        setInterpolateFunction(b,e);
      }
      b = e;
    }
  }

  void setFunctionToPoint(int ptId, int fId)
  {
    if(static_cast<size_t>(ptId) < points.size() && points[ptId])
    {
      assert(fId < functions.size());
      points[ptId]->setFunction(functions[fId]);
    }
  }

  std::vector<line_seg> lines;
  std::vector<point *> points;
  std::vector< boost::shared_ptr<DepArcProfileFunction> > functions;
};

void vtkArcDepressFilter::PrintSelf(ostream& /*os*/, vtkIndent /*indent*/)
{
}

void vtkArcDepressFilter::ClearActiveArcPoints(int arc_ind)
{
  if(arc_ind < 0 || static_cast<size_t>(arc_ind) >= Arcs.size() || Arcs[arc_ind] == NULL)
    return;
  Arcs[arc_ind]->clear();
  this->Modified();
}

void vtkArcDepressFilter::SetArcAsClosed(int arc_ind)
{
  if(arc_ind < 0 || static_cast<size_t>(arc_ind) >= Arcs.size() || Arcs[arc_ind] == NULL)
    return;
  Arcs[arc_ind]->closeArc();
}

void vtkArcDepressFilter::SetAxis(int axis)
{
  Axis = axis;
}

void vtkArcDepressFilter::AddPointToArc(double in_ind, double v1, double v2)
{
  int ind = static_cast<int>(in_ind);
  if(ind < 0) return;
  if(static_cast<size_t>(ind) >= Arcs.size() || Arcs[ind] == NULL) return;
  Arcs[ind]->addPoint(v1,v2);
  this->Modified();
}

void vtkArcDepressFilter
::SetManualControlRanges( double arc_ind, double funId,
                          double minDispDepth, double maxDispDepth,
                          double minDist, double maxDist)
{
  int ind = static_cast<int>(arc_ind);
  int fId = static_cast<int>(funId);
  if(ind < 0) return;
  if(static_cast<size_t>(ind) >= Arcs.size() || Arcs[ind] == NULL) return;
  DepArcData * td = Arcs[ind];
  assert(fId < td->functions.size());
  boost::shared_ptr<DepArcProfileFunction> fun = td->functions[fId];
  DepArcManualProfileFunction * mfun = dynamic_cast<DepArcManualProfileFunction *>(fun.get());
  assert(mfun != NULL);
  mfun->setMinMaxDesplacementDepth(  minDispDepth, maxDispDepth );
  mfun->setMinMaxDistance( minDist, maxDist );
  this->Modified();
}

void vtkArcDepressFilter::AddArc(int arc_ind)
{
  if(arc_ind < 0) return;
  size_t st_arc_ind = static_cast<size_t>(arc_ind);
  if(st_arc_ind >= this->Arcs.size())
  {
    this->Arcs.resize(st_arc_ind+1, NULL);
  }
  if(st_arc_ind < this->Arcs.size() && this->Arcs[st_arc_ind] == NULL)
  {
    this->Arcs[st_arc_ind] = new DepArcData();
    this->Modified();
  }
}

void vtkArcDepressFilter::RemoveArc(int arc_ind)
{
  if(static_cast<size_t>(arc_ind) >= Arcs.size() || Arcs[arc_ind] == NULL ) return;
  delete this->Arcs[arc_ind];
  this->Arcs[arc_ind] = NULL;
  this->Modified();
}

void vtkArcDepressFilter::SetArcEnable(int arc_ind, int isEnabled)
{
  if(static_cast<size_t>(arc_ind) >= Arcs.size() || arc_ind < 0 || Arcs[arc_ind] == NULL) return;
  Arcs[arc_ind]->IsEnabled = isEnabled != 0;
  this->Modified();
}

void vtkArcDepressFilter::SetFunctionToPoint(int arc_ind, int ptId, int funId)
{
  if(arc_ind < 0 || static_cast<size_t>(arc_ind) >= Arcs.size() || Arcs[arc_ind] == NULL)
    return;
  Arcs[arc_ind]->setFunctionToPoint(ptId, funId);
}

void vtkArcDepressFilter::AddWeightingFunPoint( double arc_ind, double funId,
                                               double x, double y, double m, double s)
{
  int ind = static_cast<int>(arc_ind);
  int fid = static_cast<int>(funId);
  if(ind < 0) return;
  if(static_cast<size_t>(ind) >= Arcs.size() || Arcs[ind] == NULL) return;
  DepArcData * td = Arcs[ind];
  assert(fid < td->functions.size());
  td->functions[fid]->addWeightPoint(x,y,m,s);
}

void vtkArcDepressFilter::AddManualDispFunPoint( double arc_ind, double funId,
                                                double x, double y, double m, double s)
{
  int ind = static_cast<int>(arc_ind);
  int fid = static_cast<int>(funId);
  if(ind < 0) return;
  if(static_cast<size_t>(ind) >= Arcs.size() || Arcs[ind] == NULL) return;
  DepArcData * td = Arcs[ind];
  assert(fid < td->functions.size());

  boost::shared_ptr<DepArcProfileFunction> fun = td->functions[fid];
  DepArcManualProfileFunction * mfun = dynamic_cast<DepArcManualProfileFunction *>(fun.get());
  assert(mfun != NULL);

  mfun->addPoint(DepArcManualProfileFunction::DispFun, x, y, m, s);
}

vtkArcDepressFilter::vtkArcDepressFilter()
:Axis(2)
{
  this->UseNormalDirection = false;
}

vtkArcDepressFilter::~vtkArcDepressFilter()
{
}

void vtkArcDepressFilter::ResizeOrder(int size)
{
  if(size < 0) return;
  ApplyOrder.resize(size, -1);
  this->Modified();
}

void vtkArcDepressFilter::SetOrderValue(int loc, int arc_ind)
{
  if(static_cast<size_t>(loc) >= ApplyOrder.size() || loc < 0 ||
     (arc_ind != -1 && static_cast<size_t>(arc_ind) >= Arcs.size()))
    return;
  ApplyOrder[loc] = arc_ind;
  this->Modified();
}

int vtkArcDepressFilter::RequestData(vtkInformation *vtkNotUsed(request),
                                     vtkInformationVector **inputVector,
                                     vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData *input =
      vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output =
      vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkSmartPointer<vtkPolyDataNormals> normalGenerator;
  vtkDataArray* normals = NULL;
  if(UseNormalDirection)
  {
    normals = input->GetPointData()->GetNormals();
    if(normals == NULL)
    {
      normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
      normalGenerator->SetInputData(input);
      normalGenerator->ComputePointNormalsOn();
      normalGenerator->ComputeCellNormalsOff();
      normalGenerator->SplittingOff();
      normalGenerator->Update();
      input = normalGenerator->GetOutput();
      normals = input->GetPointData()->GetNormals();
    }
  }

  vtkIdType i;
  vtkCellArray *newVerts, *newLines, *newPolys;
  vtkPoints *newPoints;
  vtkIdType estimatedSize, numCells=input->GetNumberOfCells();
  vtkIdType numPts=input->GetNumberOfPoints();
  vtkPoints *inPts=input->GetPoints();
  vtkDataArray* normalsGeneric = input->GetPointData()->GetNormals();
  if(normalsGeneric)
  {
  }

  // Initialize self; create output objects
  //
  if ( numPts < 1 || inPts == NULL)
    {
    vtkWarningMacro(<<"No data to clip");
    return 1;
    }

  this->IsProcessing = true;
  bool enabled = !ApplyOrder.empty();

  if(!enabled)
    {
    vtkPolyData *t = vtkPolyData::SafeDownCast(
                       inInfo->Get(vtkDataObject::DATA_OBJECT()));
    //Do nothing for now.
    output->ShallowCopy(t);
    this->IsProcessing = false;
    return 1;
    }

  estimatedSize = numCells;
  estimatedSize = estimatedSize / 1024 * 1024; //multiple of 1024
  if (estimatedSize < 1024)
    {
    estimatedSize = 1024;
    }

  newPoints = vtkPoints::New();
  newPoints->Allocate(numPts,numPts/2);
  newVerts = vtkCellArray::New();
  newVerts->Allocate(estimatedSize,estimatedSize/2);
  newLines = vtkCellArray::New();
  newLines->Allocate(estimatedSize,estimatedSize/2);
  newPolys = vtkCellArray::New();
  newPolys->Allocate(estimatedSize,estimatedSize/2);

  //Transform points
  double point[3];
  double normal[3];
  for(size_t t = 0; t < Arcs.size(); ++t)
  {
    Arcs[t]->setUpFunctions();
  }
  for ( i=0; i < numPts; i++ )
    {
    inPts->GetPoint(i, point);
    if(UseNormalDirection && normals != NULL )
      normals->GetTuple(i, normal);
    double pt2d[] = {point[0], point[1]};
    for(unsigned int j = 0; j < ApplyOrder.size(); ++j)
      {
      int id = ApplyOrder[j];
      if(id < 0 || Arcs[id] == NULL) continue;
      DepArcData const& dad = *Arcs[id];
      double d;
      size_t lsId;
      DepArcData::point closestPt;
      if(dad.getDistance(pt2d, d, lsId, closestPt))
        {
        if(UseNormalDirection && normals != NULL )
          closestPt.apply(d,point,normal);
        else
          point[2] = closestPt.apply(d, point[2]);
        }
      }
    newPoints->InsertNextPoint(point);
    }
  newVerts->DeepCopy(input->GetVerts());
  newLines->DeepCopy(input->GetLines());
  newPolys->DeepCopy(input->GetPolys());

  if (newVerts->GetNumberOfCells())
    {
    output->SetVerts(newVerts);
    }
  newVerts->Delete();

  if (newLines->GetNumberOfCells())
    {
    output->SetLines(newLines);
    }
  newLines->Delete();

  if (newPolys->GetNumberOfCells())
    {
    output->SetPolys(newPolys);
    }
  newPolys->Delete();

  output->SetPoints(newPoints);

  if(UseNormalDirection && normals != NULL)
  {
    normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
    normalGenerator->SetInputData(output);
    normalGenerator->ComputePointNormalsOn();
    normalGenerator->ComputeCellNormalsOff();
    normalGenerator->SplittingOff();
    normalGenerator->Update();
    output->GetPointData()->SetNormals(normalGenerator->GetOutput()->GetPointData()->GetNormals());
  }

  newPoints->Delete();

  output->Squeeze();

  //copy reset of the data
  this->IsProcessing = false;
  return 1;
}

void vtkArcDepressFilter::setUseNormalDirection(int in)
{
  if( in == -1 ) return;
  if( in == 1 )
  {
    this->UseNormalDirection = true;
  }
  if( in == 0)
  {
    this->UseNormalDirection = false;
  }
}

void vtkArcDepressFilter::CreateManualFunction(int arc_ind, int funId,
                                               int desptFunctionType, int weightFunType,
                                               int isRelative, int isSymmetric)
{
  if(arc_ind < 0 || static_cast<size_t>(arc_ind) >= Arcs.size() || Arcs[arc_ind] == NULL)
    return;
  assert(funId >= 0);
  DepArcData * td = Arcs[funId];
  if(funId >= td->functions.size())
  {
    td->functions.resize(funId + 1);
  }
  td->functions[funId] = boost::shared_ptr<DepArcProfileFunction>(
                            new DepArcManualProfileFunction(
                                static_cast<DepArcProfileFunction::FunctionType>(desptFunctionType),
                                static_cast<DepArcProfileFunction::FunctionType>(weightFunType),
                                isRelative, isSymmetric));
}


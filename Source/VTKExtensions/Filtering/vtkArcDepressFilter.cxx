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
#include "vtkTetra.h"
#include "vtkIntersectionPolyDataFilter.h"
#include "vtkPlaneSource.h"

#include "smtk/extension/vtk/filter/vtkCMBApplyBathymetryFilter.h"

#include <vtkTriangleFilter.h>

#include <algorithm>
#ifdef __cplusplus
#  include <cmath>
using std::isnan;
#else
#include <math.h>
#endif
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
  virtual ArcDepressFunction* clone() const = 0;
};

class MixArcDepressFunction: public ArcDepressFunction
{
public:
  MixArcDepressFunction(ArcDepressFunction * f0, ArcDepressFunction * f1, double m)
  :fun0(f0),fun1(f1), mix(m)
  {
    assert(mix>=0 && mix <= 1);
  }

  ~MixArcDepressFunction()
  {
    delete fun0;
    delete fun1;
  }

  virtual double evaluate(double d, double minF, double maxF)
  {
    return (1-mix)*fun0->evaluate(d, minF, maxF) + (mix)*fun1->evaluate(d, minF, maxF);
  }

  virtual double evaluate(double d)
  {
    return (1-mix)*fun0->evaluate(d) + (mix)*fun1->evaluate(d);
  }

  virtual void addPoint(double w, double v, double s, double m)
  {assert(false && "Do not add point on mixing function");}

  virtual void clearPoints()
  {assert(false && "Do not clear point on mixing function");}

  ArcDepressFunction* clone() const
  {
    return new MixArcDepressFunction(fun0->clone(), fun1->clone(), mix);
  }

  ArcDepressFunction * fun0, * fun1;
  double mix;
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

  ArcDepressFunction* clone() const
  {
    ArcDepressPiecewiseFun* result = new ArcDepressPiecewiseFun();
    result->fun->DeepCopy(fun);
    return result;
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

  ArcDepressFunction* clone() const
  {
    ArcDepressSplineFun* result = new ArcDepressSplineFun();
    result->fun->DeepCopy(fun);
    return result;
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

class DepArcMixProfileFunction;
class DepArcWedgeProfileFunction;

class DepArcProfileFunction
{
public:
  enum FunctionType{Piecewise = 1, Spline = 0};
  enum Type {Manual, Mix, Wedge};
  virtual ~DepArcProfileFunction(){}
  virtual Type getType() const = 0;
  virtual double apply(double d, double pt, double & dir) const = 0;
  virtual double apply(double d, double *pt, double *n) const = 0;
  virtual void addWeightPoint(double w, double v, double s, double m) = 0;
  virtual bool inside(double d) const = 0;
  virtual void setFunctionRange(double /*minZ*/, double /*maxZ*/)
  {}
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

  virtual DepArcProfileFunction::Type getType() const
  {
    return DepArcProfileFunction::Mix;
  }

  virtual double apply(double d, double pt, double & dir) const
  {
    double d1, d2;
    double p1 = fun[0]->apply(d ,pt, d1);
    double p2 = fun[1]->apply(d, pt, d2);
    dir =  d1 * (1-mix) + d2 * mix;
    return p1 * (1-mix) + p2 * mix;
  }

  virtual double apply(double d, double *pt, double *n) const
  {
    double pta[3], ptb[3];
    double dra = fun[0]->apply(d, pta, n);
    double drb = fun[1]->apply(d, ptb, n);
    double t = mix;
    double tm1 = 1-t;
    pt[0] = tm1 * pta[0] + t*ptb[0];
    pt[1] = tm1 * pta[1] + t*ptb[1];
    pt[2] = tm1 * pta[2] + t*ptb[2];
    return tm1*dra + t*drb;
  }

  bool inside(double d) const
  {
    return fun[0]->inside(d) || fun[1]->inside(d);
  }
  virtual void setFunctionRange(double minZ, double maxZ)
  {
    fun[0]->setFunctionRange(minZ, maxZ);
    fun[1]->setFunctionRange(minZ, maxZ);
  }

  virtual void addWeightPoint(double w, double v, double s, double m)
  {
    assert(false && " do not use for the mixing function");
  }

  boost::shared_ptr<DepArcProfileFunction> fun[2];
  double mix;
};

class DepArcWedgeProfileFunction : public DepArcProfileFunction
{
public:
  enum {Left = 0, Right = 1};
  enum DisplacementMode {Dig = 1, Raise = 2, Level = 3};
  DepArcWedgeProfileFunction(DepArcProfileFunction::FunctionType weightFun,
                             bool r, DisplacementMode d, bool c, double bw, double disp,
                             double sl, double sr )
  : baseWidth(bw), displacement(disp), relative((r)?1:0), clamp(c), dispMode(d)
  {
    if(relative)
    {
      maxWidth[Right] = (sr == 0) ? bw : std::abs(bw) + std::abs( disp/ sr );
      maxWidth[Left]  = (sl == 0) ? bw : std::abs(bw) + std::abs( disp/ sl );
    }
    slope[Right] = sr;//:-sr;
    slope[Left] = sl;//:-sl;
    //maxWidth[Right] = maxwR;
    //maxWidth[Left] = maxwL;
    weightFuntion = (weightFun == DepArcProfileFunction::Piecewise) ?
                                  dynamic_cast<ArcDepressFunction*>(new ArcDepressPiecewiseFun()) :
                                  dynamic_cast<ArcDepressFunction*>(new ArcDepressSplineFun());
    assert(!relative || (relative && ((dispMode == Dig && disp < 0) ||
                                      (dispMode == Raise && disp > 0)) ));
  }
  ~DepArcWedgeProfileFunction()
  {
    delete weightFuntion;
  }
  virtual DepArcProfileFunction::Type getType() const
  {
    return DepArcProfileFunction::Wedge;
  }

  bool evalFun( double d, double & weight, double & despDig, double & despRaise ) const
  {
    double sign = 1.0;
    unsigned int side = static_cast<unsigned int>(Right);
    if(d < 0)
    {
      sign = -1;
      side = static_cast<unsigned int>(Left);
    }
    if(sign*d > maxWidth[side]) return false;
    weight = weightFuntion->evaluate(d/maxWidth[side]);
    despDig = despRaise = displacement;

    if(sign*d >= 0.5*baseWidth)
    {
      double p1 = (sign*d- 0.5*baseWidth) * slope[side];
      despDig   +=  p1;
      despRaise += -p1;
    }
    return true;
  }

  virtual double apply(double d, double pt, double & dirOut) const
  {
    dirOut = 0;
    double w, tmpD, tmpR;
    if(!evalFun(d, w, tmpD, tmpR)) return pt;
    double absolute = (1-relative); // 1 or 0
    double dir[] = { tmpD - pt*absolute, tmpR - pt*absolute };
    double ptW = (1-w*absolute);

    if(dispMode & Dig && !(clamp && dir[0] > 0))
    {
      dirOut += dir[0];
      pt = ptW*pt + w*tmpD;
    }
    if(dispMode & Raise && !(clamp && dir[1] < 0))
    {
      dirOut += dir[1];
      pt = ptW*pt + w*tmpR;
    }
    return pt;
  }

  virtual double apply(double d, double *pt, double *n) const
  {
    assert(relative && "Normal direction only makes sense in relative");
    double w, tmpD, tmpR;
    if(!evalFun(d, w, tmpD, tmpR)) return 0;
    double wT[] = {(clamp && tmpD > 0)?0:w*tmpD, (clamp && tmpR < 0)?0:w*tmpR};
    double r = 0;
    if(dispMode & Dig)
    {
      pt[0] += wT[0]*n[0];
      pt[1] += wT[0]*n[1];
      pt[2] += wT[0]*n[2];
      r += wT[0];
    }
    if(dispMode & Raise)
    {
      pt[0] += wT[1]*n[0];
      pt[1] += wT[1]*n[1];
      pt[2] += wT[1]*n[2];
      r += wT[1];
    }
    return r;
  }

  virtual void addWeightPoint(double w, double v, double s, double m)
  {
    weightFuntion->addPoint(w,v,s,m);
  }

  virtual bool inside(double d) const
  {
    if(d<0) return -d <= maxWidth[Left];
    return d <= maxWidth[Right];
  }

  DepArcProfileFunction * interpolate(DepArcWedgeProfileFunction * other, double t)
  {
    assert(other->weightFuntion != NULL);
    assert(this->weightFuntion != NULL);
    assert(t<=1.0 && t >= 0);
    assert(this->relative == other->relative);
    DepArcWedgeProfileFunction * result = new DepArcWedgeProfileFunction();
    result->baseWidth = (1-t)*this->baseWidth + t*other->baseWidth;
    result->displacement = (1-t)*this->displacement + t*other->displacement;
    double thisS[] = {(this->slope[0] == 0)?0:1/this->slope[0],
                      (this->slope[1] == 0)?0:1/this->slope[1]};
    double otherS[] = {(other->slope[0] == 0)?0:1/other->slope[0],
                       (other->slope[1] == 0)?0:1/other->slope[1]};
    result->slope[0] = ((1-t)*std::abs(thisS[0]) + t*std::abs(otherS[0]));
    result->slope[1] = ((1-t)*std::abs(thisS[1]) + t*std::abs(otherS[1]));
    if(result->slope[0] != 0) result->slope[0] = 1.0/result->slope[0];
    if(result->slope[1] != 0) result->slope[1] = 1.0/result->slope[1];
    if(this->relative && other->relative)
    {
      result->maxWidth[Left] = result->baseWidth * 0.5;
      if(result->slope[Left]  != 0)
      {
        result->maxWidth[Left] = std::abs(result->maxWidth[Left]) +
                                 std::abs(result->displacement/ result->slope[Left] );
      }
      result->maxWidth[Right]  = result->baseWidth * 0.5;
      if(result->slope[Right] != 0)
      {
        result->maxWidth[Right] = std::abs(result->maxWidth[Right]) +
                                  std::abs(result->displacement/result->slope[Right]);
      }
      result->dispMode = (result->displacement < 0)?Dig:Raise;
    }
    else
    {
      result->maxWidth[0] = (1-t)*this->maxWidth[0] + t*other->maxWidth[0];
      result->maxWidth[1] = (1-t)*this->maxWidth[1] + t*other->maxWidth[1];
      result->dispMode    = (t<0.5) ? this->dispMode : other->dispMode;
    }
    result->weightFuntion = new MixArcDepressFunction(this->weightFuntion->clone(),
                                                      other->weightFuntion->clone(), t);
    result->relative = this->relative;
    result->clamp = this->clamp;
    if(relative == other->relative && clamp == other->clamp && dispMode == other->dispMode)
    {
      assert(result->weightFuntion != NULL);
      return result;
    }
    else if(relative == other->relative && clamp == other->clamp && dispMode != other->dispMode)
    {
      if(t >= 0.5)
      {
        result->clamp = other->clamp;
      }
      assert(result->weightFuntion != NULL);
      return result;
    }
    DepArcWedgeProfileFunction * tmpO = new DepArcWedgeProfileFunction(result);
    tmpO->relative = other->relative;
    tmpO->clamp = other->clamp;
    tmpO->dispMode = other->dispMode;
    assert(result->weightFuntion != NULL);
    assert(tmpO->weightFuntion != NULL);
    return new DepArcMixProfileFunction(boost::shared_ptr<DepArcProfileFunction>(result),
                                        boost::shared_ptr<DepArcProfileFunction>(tmpO), t);
  }
  virtual void setFunctionRange(double minZ, double maxZ)
  {
    if(!relative)
    {
      double hBW = baseWidth * 0.5;
      for( int i = 0; i < 2; ++i)
      {
        if(this->slope[i]  != 0)
        {
          this->maxWidth[i] =
                        std::max( std::abs(hBW) + std::abs((minZ - displacement) / this->slope[i]),
                                  std::abs(hBW) + std::abs((maxZ - displacement) / this->slope[i]));
        }

      }
    }
  }
private:
  DepArcWedgeProfileFunction()
  {}
  DepArcWedgeProfileFunction(DepArcWedgeProfileFunction *other)
  {
    this->baseWidth = other->baseWidth;
    this->displacement = other->displacement;
    this->slope[0] = other->slope[0];
    this->slope[1] = other->slope[1];
    this->maxWidth[0] = other->maxWidth[0];
    this->maxWidth[1] = other->maxWidth[1];
    this->relative = other->relative;
    this->clamp = other->relative;
    this->dispMode = other->dispMode;
    weightFuntion = other->weightFuntion->clone();
    assert(weightFuntion != NULL);
  }
  double baseWidth, displacement, slope[2], maxWidth[2];
  int relative;
  bool clamp;
  DisplacementMode dispMode;
  ArcDepressFunction * weightFuntion;
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

  virtual DepArcProfileFunction::Type getType() const
  {
    return DepArcProfileFunction::Manual;
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

  bool evalFun( double d, double & weight, double & desp ) const
  {
    if(IsSymmetric) d = std::abs(d);
    if(d < getMinDistance() || d > getMaxDistance())
    {
      return false;
    }
    if(d < 0)
    {
      d = d/-getMinDistance();
    }
    else //if( d >= 0)
    {
      d = d/getMaxDistance();
    }
    weight = SelectedFunction[WeightFun]->evaluate(d);
    desp = SelectedFunction[DispFun]->evaluate(d, getMinDepth(), getMaxDepth());
    return true;
  }

  virtual double apply(double d, double pt, double & dir) const
  {
    double w, tmp;
    dir = 0;
    if(!evalFun(d, w, tmp)) return pt;
    double wTmp = w*tmp;
    double w1 = (1-w);
    int absolute = (IsRelative)?0:1;
    dir = wTmp - (1-w)*pt*absolute;
    return (1 - w*absolute)*pt + wTmp;
    /*if(IsRelative)
    {
      return pt + wTmp;
    }
    else
    {
      return wTmp + w1*pt;
    }*/
  }

  virtual double apply(double d, double *pt, double *n) const
  {
    double w, tmp;
    assert(IsRelative && "Normal direction only makes sense in relative");
    if(!evalFun(d, w, tmp)) return 0;
    pt[0] = pt[0] + w*tmp*n[0];
    pt[1] = pt[1] + w*tmp*n[1];
    pt[2] = pt[2] + w*tmp*n[2];
    return w*tmp;
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

  boost::shared_ptr<DepArcProfileFunction>
  interpolate_functions( boost::shared_ptr<DepArcProfileFunction> fun0,
                        boost::shared_ptr<DepArcProfileFunction> fun1, double weight)
  {
    if(fun0 == fun1) return fun0;
    assert(fun0 != boost::shared_ptr<DepArcProfileFunction>());
    assert(fun1 != boost::shared_ptr<DepArcProfileFunction>());
    if(fun0->getType() != fun1->getType() ||
       fun0->getType() == DepArcProfileFunction::Manual ||
       fun0->getType() == DepArcProfileFunction::Mix)
    {
      return boost::shared_ptr<DepArcProfileFunction>(new DepArcMixProfileFunction(fun0,
                                                                                   fun1, weight));
    }
    DepArcWedgeProfileFunction * w0 = dynamic_cast<DepArcWedgeProfileFunction*>(fun0.get());
    DepArcWedgeProfileFunction * w1 = dynamic_cast<DepArcWedgeProfileFunction*>(fun1.get());
    return boost::shared_ptr<DepArcProfileFunction>(w0->interpolate(w1, weight));
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

  struct line_seg;

  struct point
  {
    double pt[2];
    line_seg const* ls[2];

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
      ls[0] = pin.ls[0];
      ls[1] = pin.ls[1];
    }
    explicit point(double x = 0, double y = 0)
    {
      pt[0] = x;
      pt[1] = y;
      ls[0] = ls[1] = NULL;
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
      ls[0] = pin.ls[0];
      ls[1] = pin.ls[1];
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

    double apply(double d, double pt, double & dir) const
    {
      assert(pointFunction.get() != NULL);
      return pointFunction->apply(d, pt, dir);
    }

    double apply(double d, double *pt, double *n) const
    {
      assert(pointFunction.get() !=NULL);
      return pointFunction->apply(d, pt, n);
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
      pt1->ls[0] = this;
      pt2->ls[1] = this;
    }

    ~line_seg()
    {
      pt1->ls[0] = NULL;
      pt2->ls[1] = NULL;
    }

    point getPoint(double t) const
    {
      assert(t<=1.0);
      if(t == 1) return *pt2;
      else if(t == 0) return *pt1;
      point result = *(pt1) + dr * t;
      result.setFunction(interpolate_functions(pt1->getFunction(), pt2->getFunction(), t));
      result.ls[0] = this;
      result.ls[1] = NULL;
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

    static bool side(line_seg const* l1, line_seg const* l2, point & pt, point & closePt)
    {
      point cpDir = pt - closePt;
      point testDir = (l1->dr-l2->dr);
      double t = cpDir.dot(testDir);
      point tmp = closePt + testDir * t;
      return l1->side(tmp, closePt);
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
      DepArcData::line_seg * l = new DepArcData::line_seg(points[prev],points[at]);
      lines.push_back(l);
    }
  }

  void clear()
  {
    for(unsigned int i  = 0; i < lines.size(); ++i)
    {
      delete lines[i];
    }
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
    lsId = 0;
    for (unsigned int i = 0; i<lines.size(); ++i)
    {
      line_seg const* ls = lines[i];
      point tmppt = ls->findClosestPoint(pt);
      double tmp = pt.distSquared(tmppt);
      if(tmp < result)
      {
        result = tmp;
        closePt = tmppt;
        lsId = i;
      }
    }
    result = sqrt(result);
    resultPt = closePt;
    if(closePt.getFunction() == NULL)
    {
      return false;
    }
    if(closePt.ls[0] != NULL && closePt.ls[1] != NULL)
    {
      if(line_seg::side(closePt.ls[0], closePt.ls[1], pt, closePt)) result = -result;
      //bool s1 = closePt.ls[0]->side(pt, closePt);
      //bool s2 = closePt.ls[1]->side(pt, closePt);
      //if(s1 == s2) if(s1) result = -result;
      //else return false;
    }
    else
    {
      line_seg const* cls = (closePt.ls[0] != NULL)?closePt.ls[0]:closePt.ls[1];

      assert(cls != NULL);

      if(cls->side(pt, closePt)) result = -result;
    }
    return closePt.inside(result);
  }

  void closeArc()
  {
    if(lines.empty()) return;

    size_t prev = points.size()-1;
    DepArcData::line_seg * l = new DepArcData::line_seg(points[prev],points[0]);
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
      total_distance += lines[i]->length();
    }
    double currentLength = lines[b]->length();
    for (size_t i = b+1; i<e; ++i)
    {
      double t = currentLength/total_distance;
      points[i]->setFunction(interpolate_functions(fun0,fun1,t));
      currentLength += lines[i]->length();
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

  void updateBound(double minZ, double maxZ)
  {
    for(unsigned int i = 0; i < functions.size(); ++i)
    {
      if(functions[i]) functions[i]->setFunctionRange(minZ, maxZ);
    }
  }

  std::vector<line_seg *> lines;
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
  currentData = NULL;
  this->UseNormalDirection = false;
}

vtkArcDepressFilter::~vtkArcDepressFilter()
{
  for(unsigned int i = 0; i < this->Arcs.size(); ++i)
  {
    delete this->Arcs[i];
  }
  this->Arcs.clear();
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

namespace
{
  double wedgeVolume( double pts[6][3] )
  {
    static int tetra[][4] = { { 0, 2, 1, 3 }, { 1, 3, 5, 4 }, { 1, 2, 5, 3 } };
    double r = 0;
    for(unsigned int j = 0; j < 3; ++j)
    {
      r += std::abs(vtkTetra::ComputeVolume(pts[tetra[j][0]], pts[tetra[j][1]],
                                            pts[tetra[j][2]], pts[tetra[j][3]]));
    }
    return r;
  }
  double pyramidVolume(double pts[6][3])
  {
    double r = 0;
    r += std::abs(vtkTetra::ComputeVolume(pts[0], pts[1], pts[2], pts[4]));
    r += std::abs(vtkTetra::ComputeVolume(pts[1], pts[2], pts[3], pts[4]));
    return r;
  }
}

void vtkArcDepressFilter::computeDisplacement( vtkPolyData *input, vtkPolyData *output,
                                               std::vector<int> &pointChanged )
{
  vtkDataArray* normals = input->GetPointData()->GetNormals();

  vtkIdType i;
  vtkCellArray *newVerts, *newLines, *newPolys;
  vtkPoints *newPoints;
  vtkIdType estimatedSize, numCells=input->GetNumberOfCells();
  vtkIdType numPts=input->GetNumberOfPoints();
  vtkPoints *inPts=input->GetPoints();
  vtkDataArray* normalsGeneric = input->GetPointData()->GetNormals();

  pointChanged.clear();
  pointChanged.resize(numPts, 0);

  estimatedSize = numCells;
  estimatedSize = estimatedSize / 1024 * 1024; //multiple of 1024
  if (estimatedSize < 1024)
  {
    estimatedSize = 1024;
  }

  newPoints = vtkPoints::New();
  newPoints->SetDataType(inPts->GetDataType());
  newPoints->Allocate(numPts,numPts/2);
  newPoints->DeepCopy(inPts);
  newVerts = vtkCellArray::New();
  newVerts->Allocate(estimatedSize,estimatedSize/2);
  newLines = vtkCellArray::New();
  newLines->Allocate(estimatedSize,estimatedSize/2);
  newPolys = vtkCellArray::New();
  newPolys->Allocate(estimatedSize,estimatedSize/2);

  //Transform points
  double point[3], original[3];
  double normal[3];
  for(size_t t = 0; t < Arcs.size(); ++t)
  {
    if(Arcs[t]!=NULL) Arcs[t]->setUpFunctions();
  }
  bool useNorm = UseNormalDirection && normals != NULL;
  double currentBounds[2];
  {
    double bounds[6];
    newPoints->GetBounds(bounds);
    currentBounds[0] = bounds[4];
    currentBounds[1] = bounds[5];
  }

  for(unsigned int j = 0; j < ApplyOrder.size() && numPts != 0; ++j)
  {
    int id = ApplyOrder[j];
    if(id < 0 || Arcs[id] == NULL) continue;
    DepArcData & dad = *Arcs[id];
    dad.updateBound(currentBounds[0], currentBounds[1]);
    double d;
    size_t lsId;
    DepArcData::point closestPt;
    newPoints->GetPoint(0, point);
    double updateBounds[] = {point[2], point[2]};
    for ( i=0; i < numPts; i++ )
    {
      newPoints->GetPoint(i, point);
      inPts->GetPoint(i, original);
      double pt2d[] = {point[0], point[1]};

      if(dad.getDistance(pt2d, d, lsId, closestPt))
      {
        double dir = 0;
        if( useNorm )
        {
          normals->GetTuple(i, normal);
          dir = closestPt.apply(d,point,normal);
        }
        else
        {
          point[2] = closestPt.apply(d, point[2], dir);
        }
        if(point[2] < updateBounds[0]) updateBounds[0]=point[2];
        if(point[2] > updateBounds[1]) updateBounds[1]=point[2];
        if(dir != 0)
        {
          pointChanged[i] = (dir<0)?-1:1;
        }
      }
      assert(useNorm ||(point[0] == original[0] && point[1] == original[1]));
      newPoints->SetPoint(i,point);
    }
    currentBounds[0] = updateBounds[0];
    currentBounds[1] = updateBounds[1];
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
  newPoints->Delete();
}


void vtkArcDepressFilter::computeChange( vtkPolyData *input, vtkPoints *originalPts,
                                        vtkPoints *newPoints, std::vector<int> &pointChanged)
{
  double * mod[3] = {NULL,&amountAdded,&amountRemoved};
  amountRemoved = 0;
  amountAdded = 0;
  bool useNorm = UseNormalDirection && NULL != input->GetPointData()->GetNormals();
  for(unsigned int i = 0; i < input->GetNumberOfCells(); ++i)
  {
    vtkCell * cell = input->GetCell(i);
    bool changed = false;
    bool allChanged = true;
    int changeScore = 0;
    int digRaiseCheck[2] = {0,0};
    int notChanged = -1;
    for(unsigned int j = 0; j < 3; ++j)
    {
      int tmpPC = pointChanged[cell->GetPointId(j)];
      changeScore += tmpPC;
      allChanged = allChanged && tmpPC;
      changed = changed || tmpPC;
      if(tmpPC) ++digRaiseCheck[(tmpPC+1)/2];
      else notChanged = static_cast<int>(j);
    }
    if(changed)
    {
      double pts[6][3];
      double * modifier = (digRaiseCheck[0])?&amountRemoved:&amountAdded;
      vtkIdType ptIds[] = { cell->GetPointId(0), cell->GetPointId(1), cell->GetPointId(2) };
      if( digRaiseCheck[0] == 3 || digRaiseCheck[1] == 3)
      {
        for(unsigned int j = 0; j < 3; ++j)
        {
          newPoints->GetPoint(cell->GetPointId(j), pts[j + digRaiseCheck[0]]);
          originalPts->GetPoint(cell->GetPointId(j), pts[j + digRaiseCheck[1]]);
          assert(useNorm ||(pts[j + digRaiseCheck[0]][0] == pts[j + digRaiseCheck[1]][0] &&
                            pts[j + digRaiseCheck[0]][1] == pts[j + digRaiseCheck[1]][1]));
        }
        *modifier += wedgeVolume(pts);
      }
      else if( (digRaiseCheck[0] == 1 && digRaiseCheck[1] == 0) ||
              (digRaiseCheck[1] == 1 && digRaiseCheck[0] == 0) )
      {
        for(unsigned int j = 0; j < cell->GetNumberOfPoints(); ++j)
        {
          vtkIdType id = cell->GetPointId(j);
          if(pointChanged[id])
          {
            if(digRaiseCheck[0])
            {
              originalPts->GetPoint(id, pts[3]);
              newPoints->GetPoint(id, pts[j]);
            }
            else
            {
              originalPts->GetPoint(id, pts[j]);
              newPoints->GetPoint(id, pts[3]);
            }
          }
          else
          {
            newPoints->GetPoint(id, pts[j]);
          }
        }
        *modifier += std::abs(vtkTetra::ComputeVolume(pts[0], pts[1], pts[2], pts[3]));
      }
      else if(( digRaiseCheck[0] == 2 && digRaiseCheck[1] == 0 ) ||
              ( digRaiseCheck[1] == 2 && digRaiseCheck[0] == 0 ) )
      {
        int index = digRaiseCheck[1]/2;
        int i1Inx[][5] = {{0,1,2,3,4},{3,2,1,0,4}};
        int i2 = (!pointChanged[ptIds[0]])?0:(!pointChanged[ptIds[1]])?1:2;
        int i2Inx[][5] = {{2,1,1,2,0},{0,2,2,0,1},{1,0,0,1,2}};
        newPoints->GetPoint(ptIds[i2Inx[i2][0]],             pts[i1Inx[index][0]]);
        newPoints->GetPoint(ptIds[i2Inx[i2][1]],             pts[i1Inx[index][1]]);
        originalPts->GetPoint(ptIds[i2Inx[i2][2]], pts[i1Inx[index][2]]);
        originalPts->GetPoint(ptIds[i2Inx[i2][3]], pts[i1Inx[index][3]]);
        newPoints->GetPoint(ptIds[i2Inx[i2][4]],             pts[i1Inx[index][4]]);

        *modifier += pyramidVolume(pts);
      }
      else
      {
        for(unsigned int j = 0; j < 3; ++j)
        {
          newPoints->GetPoint(cell->GetPointId(j), pts[j]);
          originalPts->GetPoint(cell->GetPointId(j), pts[j+3]);
        }
        int coplanar;
        double intersection[2][3];
        double surfaceid[2];
        double tolerance = 0.;
        int r = vtkIntersectionPolyDataFilter::TriangleTriangleIntersection(pts[0], pts[1], pts[2],
                                                                            pts[3], pts[4], pts[5],
                                                                            coplanar,
                                                                            intersection[0],
                                                                            intersection[1],
                                                                            surfaceid, tolerance);

        if(( digRaiseCheck[0] == 2 && digRaiseCheck[1] == 1 )||
           ( digRaiseCheck[0] == 1 && digRaiseCheck[1] == 2 ))
        {
          assert(r != 0);
          static const int at[][2] = {{1,2},{0,2},{0,1}};
          static const int oatA[][3] = {{3,3,0}, {0,0,3}, {3,3,0}};
          static const int oatB[][3] = {{0,0,3}, {3,3,0}, {0,0,3}};
          static const int oatC[][3] = {{0,0,1}, {0,1,0}, {0,0,1}};

          static const int sign[] = {0, -1, 1};
          int t1 = 0;
          for(int i = 0; i < 3; ++i)
          {
            if(sign[digRaiseCheck[0]]*pointChanged[ptIds[i]] > 0)
            {
              t1 = i;
            }
          }

          double tmp = 0;
          tmp = std::abs(vtkTetra::ComputeVolume(pts[t1], intersection[0],
                                                 intersection[1], pts[t1+3]));
          *(mod[digRaiseCheck[1]]) += tmp;
          int const* tat = at[t1];
          int z = tat[0], o = tat[1];
          int ota = oatA[t1][digRaiseCheck[0]];
          int otb = oatB[t1][digRaiseCheck[0]];
          int otc = oatC[t1][digRaiseCheck[0]];
          int otd = (otc+1)%2;

          assert(pointChanged[ptIds[z]] == pointChanged[ptIds[o]]);
          double pts2[6][3] = {{pts[z+ota][0],      pts[z+ota][1],      pts[z+ota][2]},
            {pts[z+otb][0],      pts[z+otb][1],      pts[z+otb][2]},
            {intersection[otc][0], intersection[otc][1], intersection[otc][2]},
            {pts[o+ota][0],   pts[o+ota][1],   pts[o+ota][2]},
            {pts[o+otb][0],   pts[o+otb][1],   pts[o+otb][2]},
            {intersection[otd][0], intersection[otd][1], intersection[otd][2]}};
          assert( (pointChanged[ptIds[z]]<0 && mod[digRaiseCheck[0]] == &amountRemoved) ||
                 (pointChanged[ptIds[z]]>0 && mod[digRaiseCheck[0]] == &amountAdded) );
          *(mod[digRaiseCheck[0]]) += wedgeVolume(pts2);
        }
        else
        {
#define dist(X,Y) (std::pow(X[0]-Y[0],2) + std::pow(X[1]-Y[1],2) + std::pow(X[2]-Y[2],2))
          static const int at[][2] = {{1,2},{2,0},{0,1}};
          if(!r) //must intersect by design
          {
            //move point a small amount on z of the shared point to handel the numeric instablity
            double tmpInter[4][3];
            double surfaceid[2];
            double tolerance = 0.;
            double z = pts[notChanged][2];
            pts[notChanged][2] = z + 1e-8;
            r = vtkIntersectionPolyDataFilter::TriangleTriangleIntersection( pts[3], pts[4], pts[5],
                                                                            pts[0], pts[1], pts[2],
                                                                            coplanar,
                                                                            tmpInter[0],
                                                                            tmpInter[1],
                                                                            surfaceid,
                                                                            tolerance);
            assert(r);
            pts[notChanged][2] = z - 1e-8;
            r = vtkIntersectionPolyDataFilter::TriangleTriangleIntersection( pts[3], pts[4], pts[5],
                                                                            pts[0], pts[1], pts[2],
                                                                            coplanar,
                                                                            tmpInter[2],
                                                                            tmpInter[3],
                                                                            surfaceid,
                                                                            tolerance);
            assert(r);
            pts[notChanged][2] = z;
            for(unsigned int i = 0; i < 3; ++i)
            {
              intersection[0][i] = (tmpInter[0][i] + tmpInter[2][i])*0.5;
              intersection[1][i] = (tmpInter[1][i] + tmpInter[3][i])*0.5;
            }
          }
          double d1 = dist(pts[notChanged],intersection[0]);
          double d2 = dist(pts[notChanged],intersection[1]);
          int otherPt = (d2 < d1)?0:1;
          int npt = (notChanged+1)%3;
          int addA[] = {0,3};
          double * mN = &amountAdded;
          double * mO = &amountRemoved;
          if(pointChanged[ptIds[at[notChanged][0]]]<0)
          {
            addA[0] = 3; addA[1] = 0;
            mN = &amountRemoved;
            mO = &amountAdded;
          }
          *mN += std::abs(vtkTetra::ComputeVolume(pts[at[notChanged][0]+addA[0]],
                                                  pts[notChanged], intersection[otherPt],
                                                  pts[at[notChanged][0]+addA[1]]));

          *mO += std::abs(vtkTetra::ComputeVolume(pts[at[notChanged][1]+addA[0]],
                                                  pts[notChanged], intersection[otherPt],
                                                  pts[at[notChanged][1]+addA[1]]));
          assert( (pointChanged[ptIds[at[notChanged][1]]]<0 && mO == &amountRemoved) ||
                 (pointChanged[ptIds[at[notChanged][1]]]>0 && mO == &amountAdded) );
        }
      }
    }
  }
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
  currentData = input;
  vtkSmartPointer<vtkPolyDataNormals> normalGenerator;
  vtkSmartPointer<vtkTriangleFilter> triangleFilter;

  if(input == NULL) return 1;

  if(input->GetNumberOfPoints() < 1) return 1;

  this->IsProcessing = true;

  bool noValidFunctions = true;
  for(unsigned int j = 0; j < ApplyOrder.size(); ++j)
  {
    int id = ApplyOrder[j];
    if(id < 0 || Arcs[id] == NULL) continue;
    DepArcData & dad = *Arcs[id];
    noValidFunctions &= dad.points.empty();
  }

  if(ApplyOrder.empty() || noValidFunctions)
  {
    //Do nothing for now.
    output->ShallowCopy(input);
    this->IsProcessing = false;
    return 1;
  }

  if(input->GetNumberOfCells() != 0)
  {
    triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
    triangleFilter->SetInputData(input);
    triangleFilter->Update();
    input = triangleFilter->GetOutput();
  }

  bool inputHasNormals = input->GetPointData()->GetNormals() != NULL;
  if(UseNormalDirection && !inputHasNormals)
  {
    normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
    normalGenerator->SetInputData(input);
    normalGenerator->ComputePointNormalsOn();
    normalGenerator->ComputeCellNormalsOff();
    normalGenerator->SplittingOff();
    normalGenerator->Update();
    input = normalGenerator->GetOutput();
  }

  std::vector<int> pointChanged; //0 no change, 1 raise, -1 dig
  this->computeDisplacement( input, output, pointChanged );

  if(inputHasNormals)
  {
    normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
    normalGenerator->SetInputData(output);
    normalGenerator->ComputePointNormalsOn();
    normalGenerator->ComputeCellNormalsOff();
    normalGenerator->SplittingOff();
    normalGenerator->Update();
    output->GetPointData()->SetNormals(normalGenerator->GetOutput()->GetPointData()->GetNormals());
  }

  output->Squeeze();

  amountAdded = -1;
  amountRemoved = -1;
  if(input->GetNumberOfCells() != 0 && input->GetCell(0)->GetNumberOfPoints() == 3)
  {
    this->computeChange( input, input->GetPoints(), output->GetPoints(), pointChanged );
  }

  //copy reset of the data
  this->IsProcessing = false;
  return 1;
}

void vtkArcDepressFilter
::computeDisplacementChangeOnPointsViaBathymetry(double stepSize, double radius)
{
  if(stepSize <= 0 || radius <= 0) return;
  vtkSmartPointer<vtkPlaneSource> planeSource = vtkSmartPointer<vtkPlaneSource>::New();
  double bounds[6];
  vtkPoints * points = this->currentData->GetPoints();
  points->GetBounds(bounds);
  double w = bounds[1]-bounds[0];
  double h = bounds[3]-bounds[2];
  int ws = static_cast<int>( std::ceil(w/stepSize) );
  int hs = static_cast<int>( std::ceil(h/stepSize) );
  planeSource->SetPoint1( bounds[1], bounds[2], 0 );
  planeSource->SetPoint2( bounds[0], bounds[3], 0 );
  planeSource->SetOrigin( bounds[0], bounds[2], 0 );
  planeSource->SetResolution( ws, hs );
  planeSource->SetNormal(0,0,1);
  planeSource->Update();
  vtkPolyData * plane = planeSource->GetOutput();
  vtkSmartPointer<vtkCMBApplyBathymetryFilter> bathyFilter =
                                                vtkSmartPointer<vtkCMBApplyBathymetryFilter>::New();
  double invalidVal = NAN; //TODO This
  bathyFilter->SetElevationRadius(radius);
  bathyFilter->SetInvalidValue(invalidVal); //TODO This
  bathyFilter->SetInputData(0, plane);
  bathyFilter->SetInputData(1, this->currentData);
  bathyFilter->Update();
  vtkPolyData * bethPlane = static_cast<vtkPolyData*>(bathyFilter->GetOutput());
  vtkSmartPointer<vtkTriangleFilter> triangleFilter;
  triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
  triangleFilter->SetInputData(bethPlane);
  triangleFilter->Update();
  bethPlane = triangleFilter->GetOutput();
  bethPlane->BuildLinks();
  vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();

  //filter out invalid points
  for(unsigned int i = 0; i < bethPlane->GetNumberOfPoints(); i++)
  {
    double * pt = bethPlane->GetPoint(i);
    if(isnan(pt[2]))// == invalidVal)
    {
      bethPlane->GetPointCells(i, cellIds);
      for(unsigned int j = 0; j < cellIds->GetNumberOfIds(); ++j)
      {
        bethPlane->DeleteCell(cellIds->GetId(j));
      }
    }
  }
  bethPlane->RemoveDeletedCells();
  vtkPolyData * out = bethPlane->NewInstance();
  //Apply the displacment
  std::vector<int> pointChanged; //0 no change, 1 raise, -1 dig
  this->computeDisplacement( bethPlane, out, pointChanged );
  //calculate displacment vol change
  this->computeChange( bethPlane, bethPlane->GetPoints(), out->GetPoints(), pointChanged );
  out->Delete();
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
  DepArcData * td = Arcs[arc_ind];
  if(funId >= td->functions.size())
  {
    td->functions.resize(funId + 1);
  }
  td->functions[funId] = boost::shared_ptr<DepArcProfileFunction>(
                            new DepArcManualProfileFunction(
                                static_cast<DepArcProfileFunction::FunctionType>(desptFunctionType),
                                static_cast<DepArcProfileFunction::FunctionType>(weightFunType),
                                isSymmetric, isRelative));
  this->Modified();
}

void vtkArcDepressFilter
::CreateWedgeFunction( double arc_ind_in, double funId_in, double weightFunType_in,
                       double relative_in, double mode_in, double clamp_in, double basewidth,
                       double displacement, double slopeLeft, double slopeRight)
{
  int arc_ind = static_cast<int>(arc_ind_in);
  int funId = static_cast<int>(funId_in);
  DepArcProfileFunction::FunctionType weightFunType=
              static_cast<DepArcProfileFunction::FunctionType>(static_cast<int>(weightFunType_in));
  bool relative = relative_in != 0;
  DepArcWedgeProfileFunction::DisplacementMode disMode =
                            static_cast< DepArcWedgeProfileFunction::DisplacementMode > (static_cast<int>(mode_in+1));
  bool clamp = clamp_in != 0;

  if(arc_ind < 0 || static_cast<size_t>(arc_ind) >= Arcs.size() || Arcs[arc_ind] == NULL)
    return;

  assert(funId >= 0);
  DepArcData * td = Arcs[arc_ind];
  if(funId >= td->functions.size())
  {
    td->functions.resize(funId + 1);
  }

  td->functions[funId] =
    boost::shared_ptr<DepArcProfileFunction>(
        new DepArcWedgeProfileFunction(weightFunType, relative, disMode, clamp, basewidth,
                                       displacement, slopeLeft, slopeRight ));
  this->Modified();
}

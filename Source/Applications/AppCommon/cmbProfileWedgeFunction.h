//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef cmbProfileWedgeFunction_h_
#define cmbProfileWedgeFunction_h_

#include "cmbProfileFunction.h"

class CMBAPPCOMMON_EXPORT cmbProfileWedgeFunction : public cmbProfileFunction
{
public:
  friend class cmbProfileFunction;
  enum DisplacmentMode{Dig,Raise,Level};
  cmbProfileWedgeFunction();
  ~cmbProfileWedgeFunction();
  virtual cmbProfileFunction::FunctionType getType() const;
  virtual cmbProfileFunction * clone(std::string const& name) const;
  virtual void sendDataToProxy(int arc_ID, int pointID,
                               vtkBoundingBox bbox,
                               vtkSMSourceProxy* source) const;

  virtual vtkPiecewiseFunction * getWeightingFunction() const
  {
    return WeightingFunction;
  }

  double getDepth() const;
  double getBaseWidth() const;
  double getSlopeLeft() const;
  double getSlopeRight() const;

  void setDepth(double d);
  void setBaseWidth(double d);
  void setSlopeLeft(double d);
  void setSlopeRight(double d);

  bool isRelative() const;
  virtual void setRelative(bool ir);

  bool isSymmetric() const;
  void setSymmetry(bool);

  bool isWeightSpline() const;
  void setWeightSpline(bool w);

  bool isClamped() const;
  void setClamped(bool w);

  void setMode(DisplacmentMode d);
  DisplacmentMode getMode() const;

protected:
  virtual bool readData(std::ifstream & in, int version);
  virtual bool writeData(std::ofstream & out) const;
private:
  double depth;
  double baseWidth;
  double slopeLeft;
  double slopeRight;

  vtkPiecewiseFunction * WeightingFunction;

  bool Relative;
  bool Symmetry;
  bool WeightUseSpline;

  bool clamp;
  DisplacmentMode dispMode;

  cmbProfileWedgeFunction(cmbProfileWedgeFunction const* other);
};

#endif

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef cmbManualProfileFunction_h_
#define cmbManualProfileFunction_h_

#include "cmbProfileFunction.h"
#include "pqCMBModifierArc.h"
class cmbManualProfileFunction;

class CMBAPPCOMMON_EXPORT cmbManualProfileFunctionParameters : public cmbProfileFunctionParameters
{
public:
  friend class cmbManualProfileFunction;
  cmbManualProfileFunctionParameters();
  ~cmbManualProfileFunctionParameters() override;
  cmbProfileFunctionParameters * clone() override;

  double getDistanceRange(pqCMBModifierArc::RangeLable i);
  double getDepthRange(pqCMBModifierArc::RangeLable i);

  void setDistanceRange(pqCMBModifierArc::RangeLable i, double v);
  void setDepthRange(pqCMBModifierArc::RangeLable i, double v);

protected:
  double DistanceRange[2];
  double DisplacementDepthRange[2];

  cmbManualProfileFunctionParameters(cmbManualProfileFunctionParameters const* other);
};

class CMBAPPCOMMON_EXPORT cmbManualProfileFunction : public cmbProfileFunction
{
public:
  friend class cmbProfileFunction;
  cmbManualProfileFunction();
  ~cmbManualProfileFunction() override;
  cmbProfileFunction::FunctionType getType() const override;
  virtual vtkPiecewiseFunction * getDisplacementProfile() const;
  virtual vtkPiecewiseFunction * getWeightingFunction() const;
  cmbProfileFunction * clone(std::string const& name) const override;
  void sendDataToProxy(int arc_ID, int pointID,
                               vtkBoundingBox bbox,
                               vtkSMSourceProxy* source) const override;
  virtual cmbProfileFunctionParameters * getParameters() const;

  bool isSymmetric() const;
  bool isRelative() const;
  void setSymmetric(bool is);
  void setRelative(bool ir) override;
  bool isDispSpline() const;
  bool isWeightSpline() const;
  void setDispSpline(bool s);
  void setWeightSpline(bool w);

  double getDistanceRange(pqCMBModifierArc::RangeLable i) const;
  double getDepthRange(pqCMBModifierArc::RangeLable i) const;

  void setDistanceRange(pqCMBModifierArc::RangeLable i, double v);
  void setDepthRange(pqCMBModifierArc::RangeLable i, double v);

  void setDistanceRange(double min, double max);
  void setDepthRange(double min, double max);

protected:
  bool readData(std::ifstream & in, int version) override;
  bool writeData(std::ofstream & out) const override;
private:
  vtkPiecewiseFunction * DisplacementProfile;
  vtkPiecewiseFunction * WeightingFunction;

  cmbManualProfileFunctionParameters * parameters;

  cmbManualProfileFunction(cmbManualProfileFunction const* other);

  bool Symmetric;
  bool Relative;

  bool DispUseSpline;
  bool WeightUseSpline;
};

#endif

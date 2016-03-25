#ifndef cmbProfileWedgeFunction_h_
#define cmbProfileWedgeFunction_h_

#include "cmbProfileFunction.h"

class cmbProfileWedgeFunction;

class CMBAPPCOMMON_EXPORT cmbProfileWedgeFunctionParameters : public cmbProfileFunctionParameters
{
public:
  friend class cmbProfileWedgeFunction;
  cmbProfileWedgeFunctionParameters();
  virtual ~cmbProfileWedgeFunctionParameters();
  virtual cmbProfileWedgeFunctionParameters * clone();

  double getDepth() const;
  double getBaseWidth() const;
  double getSlopeLeft() const;
  double getSlopeRight() const;

  void setDepth(double d);
  void setBaseWidth(double d);
  void setSlopeLeft(double d);
  void setSlopeRight(double d);

protected:
  double depth;
  double baseWidth;
  double slopeLeft;
  double slopeRight;

  cmbProfileWedgeFunctionParameters(cmbProfileWedgeFunctionParameters const* other);
};

class CMBAPPCOMMON_EXPORT cmbProfileWedgeFunction : public cmbProfileFunction
{
public:
  friend class cmbProfileFunction;
  cmbProfileWedgeFunction();
  ~cmbProfileWedgeFunction();
  virtual cmbProfileFunction::FunctionType getType() const;
  virtual cmbProfileFunction * clone(std::string const& name) const;
  virtual void sendDataToProxy(int arc_ID, int pointID,
                               vtkSMSourceProxy* source) const;
  virtual cmbProfileFunctionParameters * getParameters() const;

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
  void setRelative(bool ir);

  bool isSymmetric() const;
  void setSymmetry(bool);

  bool isWeightSpline() const;
  void setWeightSpline(bool w);

protected:
  virtual bool readData(std::ifstream & in, int version);
  virtual bool writeData(std::ofstream & out) const;
private:
  cmbProfileWedgeFunctionParameters * parameters;
  vtkPiecewiseFunction * WeightingFunction;

  bool Relative;
  bool Symmetry;
  bool WeightUseSpline;

  cmbProfileWedgeFunction(cmbProfileWedgeFunction const* other);
};

#endif

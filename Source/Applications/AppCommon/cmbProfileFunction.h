#ifndef cmbProfileFunction_h_
#define cmbProfileFunction_h_

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"

#include "pqCMBModifierArc.h"

#include <string>
#include <fstream>

class CMBAPPCOMMON_EXPORT cmbProfileFunctionParameters
{
public:
  virtual ~cmbProfileFunctionParameters(){}
  virtual cmbProfileFunctionParameters * clone() = 0;
};

class CMBAPPCOMMON_EXPORT cmbProfileFunction
{
public:
  //Add here when there are more than one function
  enum FunctionType{ WEDGE = 0, MANUAL = 1 };
  virtual ~cmbProfileFunction(){}
  virtual FunctionType getType() const = 0;
  virtual cmbProfileFunction * clone(std::string const& name) const = 0;
  void setName(std::string const& n);
  std::string const& getName() const;
  bool write(std::ofstream & out) const;
  static cmbProfileFunction * read(std::ifstream & in, size_t count);
  virtual void setRelative(bool b) = 0;

  virtual void sendDataToProxy(int arc_ID, int pointID,
                               vtkSMSourceProxy* source) const = 0;
protected:
  virtual bool readData(std::ifstream & in, int version) = 0;
  virtual bool writeData(std::ofstream & out) const = 0;
  std::string name;
};

#endif

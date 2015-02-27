// cOutputs.h: interface for the cOutputs class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(COUTPUTS_H)
#define COUTPUTS_H

#include <string>

using namespace std;

class cOutputs
{
private:
  bool OmicronOutput;
  string OmicronOutputName;
  bool VegOutput;
  string VegOutputName;
  bool POVRayOutput;
  string POVRayOutputName;
  bool WavefrontOutput;
  string WavefrontOutputName;
public:
  cOutputs();
  virtual ~cOutputs();
  void Init();

  void AddOmicronFileName( string fname );
  bool GetOmicronFileName( string& fname );

  void AddVegFileName( string fname );
  bool GetVegFileName( string& fname );

  void AddPOVRayFileName( string fname );
  bool GetPOVRayFileName( string& fname );

  void AddWavefrontFileName( string fname );
  bool GetWavefrontFileName( string& fname );

};

#endif // !defined(COUTPUTS_H)

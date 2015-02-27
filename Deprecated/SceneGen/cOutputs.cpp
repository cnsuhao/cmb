// cOutputs.cpp: implementation of the cOutputs class.
//
//////////////////////////////////////////////////////////////////////

#include "cOutputs.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cOutputs::cOutputs()
{
  Init();
}

cOutputs::~cOutputs()
{
  Init();
}

void cOutputs::Init()
{
  OmicronOutput = false;
  OmicronOutputName = "";

  VegOutput = false;
  VegOutputName = "";
}
//////////////////////////////////////////////////////////////////////
// Main Processing
//////////////////////////////////////////////////////////////////////

void cOutputs::AddOmicronFileName( string fname )
{
  OmicronOutput = true;
  OmicronOutputName = fname;
}

bool cOutputs::GetOmicronFileName( string& fname )
{
  if (OmicronOutput) fname = OmicronOutputName;
  return (OmicronOutput);
}

void cOutputs::AddVegFileName( string fname )
{
  VegOutput = true;
  VegOutputName = fname;
}

bool cOutputs::GetVegFileName( string& fname )
{
  if (VegOutput) fname = VegOutputName;
  return (VegOutput);
}

void cOutputs::AddPOVRayFileName( string fname )
{
  POVRayOutput = true;
  POVRayOutputName = fname;
}

bool cOutputs::GetPOVRayFileName( string& fname )
{
  if (POVRayOutput) fname = POVRayOutputName;
  return (POVRayOutput);
}

void cOutputs::AddWavefrontFileName( string fname )
{
  WavefrontOutput = true;
  WavefrontOutputName = fname;
}

bool cOutputs::GetWavefrontFileName( string& fname )
{
  if (WavefrontOutput) fname = WavefrontOutputName;
  return (WavefrontOutput);
}

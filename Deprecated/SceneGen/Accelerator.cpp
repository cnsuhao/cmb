// Accelerator.cpp: implementation of the CAccelerator class.
//
//////////////////////////////////////////////////////////////////////

#include "Accelerator.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAccelerator::CAccelerator()
{
  current = 0.0;
  acc = 0.001f;
  top = 1.0;
  tcnt = 0;
  wait = 30;

}

CAccelerator::~CAccelerator()
{

}

float CAccelerator::Next()
{
  if (current==0.0)
  {
    current = acc;
    tcnt = 0;
  }

  if (tcnt<wait)
  {
    tcnt++;
    return current;
  }

  if (current>=top) return top;
  current += acc;

  return current;
}

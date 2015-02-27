// Accelerator.h: interface for the CAccelerator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(ACCELERATOR_H)
#define ACCELERATOR_H

class CAccelerator
{
private:
  float current;
  float acc;
  float top;
  int tcnt;
  int wait;

public:
  CAccelerator();
  virtual ~CAccelerator();

  void SetAccel( float accel ) { acc = accel; }
  void SetTop( float tp ) { top = tp; }
  void SetWait( int lt ) { wait = lt; }

  void Restart( ) { current=0.0; tcnt = 0; }
  float Next( );

};

#endif // !defined(ACCELERATOR_H)

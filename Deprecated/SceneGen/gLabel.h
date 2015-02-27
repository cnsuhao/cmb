// gLabel.h: interface for the gLabel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_GLABEL_H__)
#define _GLABEL_H__

#include "gWnd.h"

class gLabel : public gWnd
{
protected:
  //borderstyletype borderstyle;
  //textpostype textpos;

public:
  gLabel();
  virtual ~gLabel();

  void OnDraw( void );
  void OnReshape( int w, int h );

};

#endif // !defined(_GLABEL_H__)

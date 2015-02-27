// cTWI.cpp: implementation of the cTWI class.
//
//////////////////////////////////////////////////////////////////////

#include "cTWI.h"

#include <iostream>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cTWI::cTWI( QTreeWidget *parent, const QStringList & strings, int type) : QTreeWidgetItem( parent, strings, type)
{
  idx=-1000;
}

cTWI::cTWI( const QStringList & strings, int type) : QTreeWidgetItem( strings, type)
{
  idx=-1000;
}

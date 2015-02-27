// cTWI.h: interface for the cTWI class.
//          inherits QTreeWidgetItem to store index info
//////////////////////////////////////////////////////////////////////

#if !defined(CTWI_H)
#define CTWI_H

#include <QTreeWidgetItem>

class cTWI : public QTreeWidgetItem
{
private:
  long idx;
public:
  cTWI( QTreeWidget * parent, const QStringList & strings, int type = 1001 );
  cTWI( const QStringList & strings, int type = 1001 );

  void SetIdx( long id ) { idx=id; }
  long GetIdx( ) { return idx; }
};

#endif

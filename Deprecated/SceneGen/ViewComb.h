// ViewComb.h: definition of the ViewComb class.
//
//   Combines a ViewWnd with a TreeView for selection
//
//////////////////////////////////////////////////////////////////////

#if !defined(_VIEWCOMB_H__)
#define _VIEWCOMB_H__

#
#include <QSplitter>
#include "SceneGen.h"

class ViewWnd;

class ViewComb : public QSplitter //QWidget
{
      Q_OBJECT
private:
  // Pointer to the main data structure
  MainSceneData *msd;

  // Our combined objects
  QTreeWidget *msdtree;
  ViewWnd *vwnd;

public:

signals:
//  void changeMouseWorld(double wx, double wy);
  void changeMessage( char *msg );
  void clearStatus();

  void changeExtents( long x );  //passthrough to vwnd

  void checkChanged( long isset, bool bval );

public slots:
  void extentsChanged( long x );

  void messageChange( char *msg );
  void statusClear();

  void triggerSelection( QTreeWidgetItem *qtwi, int col );
  void selectCheck( long isset, bool bval );

public:
  ViewComb(QWidget *parent = 0);

  void SetMSD( MainSceneData *xmsd );

  void Prime( void );
  void SetAccel( void );
  QTreeWidget *GetTree() { return msdtree; }
  void ResetView();
  void SetCameraPosition( double x, double y, double z, double heading, double pitch, double yaw );
  void saveScreen( const char *fname );
  void SaveCameraSettings( const char *fname );
  void LoadCameraSettings( const char *fname );
};

#endif

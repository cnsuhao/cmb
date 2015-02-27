// ViewComb.cpp: implementation of the ViewComb class.
//
//////////////////////////////////////////////////////////////////////

#include <QtGui>
#include <QTreeWidgetItem>
#include "cTWI.h"

#include "ViewComb.h"
#include "ViewWnd.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ViewComb::ViewComb(QWidget *parent)
    : QSplitter(parent)
{
  msdtree = new QTreeWidget;
  vwnd = new ViewWnd;

  msdtree->setColumnCount(2);
  QStringList headers;
  headers << QString("Regions") << QString("MatID");
  msdtree->setHeaderLabels( QStringList(headers) );

  addWidget(msdtree);
  addWidget(vwnd);

  //passthrough events
  connect(vwnd, SIGNAL(changeMessage(char *)), this, SLOT(messageChange(char *)));
  connect(vwnd, SIGNAL(clearStatus()), this, SLOT(statusClear()));
  connect(this, SIGNAL(changeExtents(long)), vwnd, SLOT(extentsChanged(long)));
  connect(msdtree, SIGNAL(itemChanged( QTreeWidgetItem *, int )),
           this, SLOT(triggerSelection( QTreeWidgetItem *, int )));
}

void ViewComb::extentsChanged( long x )
{
  emit changeExtents(x);
}

void ViewComb::messageChange( char *msg )
{
  emit changeMessage( msg );
}

void ViewComb::statusClear()
{
  emit clearStatus();
}

void ViewComb::SetMSD( MainSceneData *xmsd )
{
  msd=xmsd;
  vwnd->SetMSD(xmsd);
  connect(this, SIGNAL(checkChanged( long , bool )),
           &(msd->osdl), SLOT(onSelection( long, bool )) );
  connect(&(msd->osdl), SIGNAL(selection( long, bool )),
            this, SLOT(selectCheck( long , bool )) );
}

void ViewComb::Prime( void )
{ vwnd->Prime(); }

void ViewComb::SetAccel( void )
{ vwnd->SetAccel(); }

void ViewComb::triggerSelection( QTreeWidgetItem *qtwi, int col)
{
  //slot code
  cTWI *ctwi;
  bool cs;

  ctwi = (cTWI *) qtwi;
  cs = (ctwi->checkState(col) == Qt::Checked);
  //printf("  triggerSelection - id=%ld   checked=%d \n",ctwi->GetIdx(), cs);


  emit checkChanged(ctwi->GetIdx(), cs);
  vwnd->Refresh();
}

void ViewComb::selectCheck( long isset, bool bval)
{
  cTWI *ctwiP, *ctwiC;
  long ii, ij;
  bool cs;

  //check every child idx
  for (ii = 0; ii < msdtree->topLevelItemCount(); ii++)
  {
    ctwiP = (cTWI *)msdtree->topLevelItem(ii);
    for (ij = 0; ij < ctwiP->childCount(); ij++)
    {
      ctwiC = (cTWI *)ctwiP->child(ij);

      if (isset == ctwiC->GetIdx())
      {
        //stop recursion
        cs = (ctwiC->checkState(0)==Qt::Checked);
        if (bval!=cs)
        {
          if (bval) ctwiC->setCheckState(0, Qt::Checked);
          else ctwiC->setCheckState(0, Qt::Unchecked);
          //set the correct check value
          emit checkChanged(ctwiC->GetIdx(), bval);
        }
        return;
      }
    }
  }
}

void ViewComb::ResetView()
{ vwnd->ResetView(); }

void ViewComb::SetCameraPosition( double x, double y, double z, double heading, double pitch, double yaw )
{ vwnd->SetCameraPosition(x,y,z,heading,pitch, yaw); }

void ViewComb::saveScreen( const char *fname )
{ vwnd->saveScreen(fname); }

void ViewComb::SaveCameraSettings( const char *fname )
{ vwnd->SaveCameraSettings(fname); }

void ViewComb::LoadCameraSettings( const char *fname )
{ vwnd->LoadCameraSettings(fname); }

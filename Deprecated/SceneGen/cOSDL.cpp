// cOSDL.cpp: implementation of the cOSDL class.
//
//////////////////////////////////////////////////////////////////////

#include "cOSDL.h"
#include "cTWI.h"

#include "PathFix.h"
//////////////////////////////////////////////////////////////////////
// UNDO Construction/Destruction
//////////////////////////////////////////////////////////////////////
cOSDL_Undo::cOSDL_Undo()
{
  UndoInit();
}

cOSDL_Undo::~cOSDL_Undo()
{
  UndoInit();
}

void cOSDL_Undo::UndoInit()
{
  op = 0;

  objectid.clear();
  matid.clear();
  sclid.clear();

  doff[0] = doff[1] = doff[2] = 0.0;
  drot = drot.identity();
  scale[0] = scale[1] = scale[2] = 1.0;

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
cOSDL::cOSDL()
{
  Init();
}

cOSDL::~cOSDL()
{
  Init();
}

void cOSDL::Init()
{
  long ii;

  loading=false;

  version = 1.0;

  outputs.Init();
  OmicronModel.Initialize();
  VegModel.Initialize();

//  aoi.Init();
  aoi.clear();

  for (ii=0; ii<(long)oglist.size(); ii++) delete oglist[ii];
  oglist.clear();

  surfaces.clear();

  tessobj.clear();
  orienttessobj.clear();

  occluder.clear();
  orientoccluder.clear();

  ulist.clear();  //clear the UNDO list
}

void cOSDL::ClearDisplayLists()
{
  long ii;

  for (ii=0; ii<(long)oglist.size(); ii++)
    oglist[ii]->ClearDisplayLists();
}

//////////////////////////////////////////////////////////////////////
// Clean-up routines
//////////////////////////////////////////////////////////////////////
bool cOSDL::ObjectGeometryReferenced( long idx )
{
  long ii;

  if (primary_surface.GetLink()==oglist[idx]) return true;

  for (ii=0; ii<(long)surfaces.size(); ii++)
    if (surfaces[ii].GetLink()==oglist[idx]) return true;

  for (ii=0; ii<(long)tessobj.size(); ii++)
    if (tessobj[ii].GetLink()==oglist[idx]) return true;

  for (ii=0; ii<(long)orienttessobj.size(); ii++)
    if (orienttessobj[ii].GetLink()==oglist[idx]) return true;

  for (ii=0; ii<(long)occluder.size(); ii++)
    if (occluder[ii].GetLink()==oglist[idx]) return true;

  for (ii=0; ii<(long)orientoccluder.size(); ii++)
    if (orientoccluder[ii].GetLink()==oglist[idx]) return true;

  return false;
}

void cOSDL::cleanObjectGeometry( long idx )
{
  // The purpose of this routine is to remove unused object geometries
  //   and reassign existing ones for memory efficiency  (call sparingly)
  long ii;

  if (idx==-1)  //cleanup all
  {
    for (ii=0; ii<(long)oglist.size(); ii++)
    {
      //check if referenced
      if (!ObjectGeometryReferenced( ii ))
      {
        //if not remove and adjust
        oglist.erase(oglist.begin()+ii);
      }
    }
  }
  else  //cleanup a particular id
  {
    //check if referenced
    if (!ObjectGeometryReferenced( idx ))
    {
      //if not remove and adjust
      oglist.erase(oglist.begin()+idx);
    }
  }

}

void cOSDL::Clean( void )
{
  long ii;

  ii=0;
  while ( ii<(long)surfaces.size() )
  {
    if (surfaces[ii].IsDeleted())
    {
      surfaces.erase(surfaces.begin()+ii);
    }
    else ii++;
  }

  ii=0;
  while ( ii<(long)tessobj.size() )
  {
    if (tessobj[ii].IsDeleted())
    {
      tessobj.erase(tessobj.begin()+ii);
    }
    else ii++;
  }

  ii=0;
  while ( ii<(long)orienttessobj.size() )
  {
    if (orienttessobj[ii].IsDeleted())
    {
      orienttessobj.erase(orienttessobj.begin()+ii);
    }
    else ii++;
  }

  ii=0;
  while ( ii<(long)occluder.size() )
  {
    if (occluder[ii].IsDeleted())
    {
      occluder.erase(occluder.begin()+ii);
    }
    else ii++;
  }

  ii=0;
  while ( ii<(long)orientoccluder.size() )
  {
    if (orientoccluder[ii].IsDeleted())
    {
      orientoccluder.erase(orientoccluder.begin()+ii);
    }
    else ii++;
  }

  //removes deleted items and any unused geometries
  cleanObjectGeometry();
}

//////////////////////////////////////////////////////////////////////
// Plotting routines
//////////////////////////////////////////////////////////////////////
void cOSDL::FillTree( QTreeWidget *qtree )
{
  cTWI *ctwi0, *ctwi1;
  QString matid;
  long ii, idx;

  qtree->clear();

  idx = BASE_SURFACE; //base address for surfaces
  //primary surface
  ctwi0 = new cTWI(qtree, QStringList(QString("primary surface")));
  if (!IsEmpty() || IsLoading())
  {
    ctwi1 = new cTWI(QStringList(QString("surface")) );
    ctwi0->addChild((QTreeWidgetItem *)ctwi1);
    ctwi1->SetIdx(idx);
    idx++;
  }

  //secondary surfaces
  ctwi0 = new cTWI(qtree, QStringList(QString("secondary surfaces")));
  for (ii=0; ii<(long)surfaces.size(); ii++)
  {
    if (!surfaces[ii].IsDeleted())
    {
      matid.setNum(surfaces[ii].GetMatID());
      ctwi1 = new cTWI(QStringList(QString(surfaces[ii].GetName()))<<matid );
      ctwi0->addChild((QTreeWidgetItem *)ctwi1);
      ctwi1->SetIdx(idx);
      if (surfaces[ii].IsSelected())
      {
        ctwi1->setCheckState(0,Qt::Checked);
      }
      else
      {
        ctwi1->setCheckState(0,Qt::Unchecked);
      }
    }
    idx++;
  }

  idx = BASE_TESSOBJ; //base address for tesselated objects
  //tesselated objects
  ctwi0 = new cTWI(qtree, QStringList(QString("tesselated objects")));
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    if (!tessobj[ii].IsDeleted())
    {
      matid.setNum(tessobj[ii].GetMatID());
      ctwi1 = new cTWI(QStringList(QString(tessobj[ii].GetName()))<<matid );
      ctwi0->addChild((QTreeWidgetItem *)ctwi1);
      ctwi1->SetIdx(idx);
      if (tessobj[ii].IsSelected())
      {
        ctwi1->setCheckState(0,Qt::Checked);
      }
      else
      {
        ctwi1->setCheckState(0,Qt::Unchecked);
      }
    }
    idx++;
  }

  idx = BASE_ORIENTTESSOBJ; //base address for oriented tesselated objects
  //oriented tesselated objects
  ctwi0 = new cTWI(qtree, QStringList(QString("oriented tessobj")));
  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    if (!orienttessobj[ii].IsDeleted())
    {
      matid.setNum(orienttessobj[ii].GetMatID());
      ctwi1 = new cTWI(QStringList(QString(orienttessobj[ii].GetName()))<<matid );
      ctwi0->addChild((QTreeWidgetItem *)ctwi1);
      ctwi1->SetIdx(idx);
      if (orienttessobj[ii].IsSelected())
      {
        ctwi1->setCheckState(0,Qt::Checked);
      }
      else
      {
        ctwi1->setCheckState(0,Qt::Unchecked);
      }
    }
    idx++;
  }

  idx = BASE_OCCLUDER; //base address for occluders
  //occluders
  ctwi0 = new cTWI(qtree, QStringList(QString("occluder")));
  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    if (!occluder[ii].IsDeleted())
    {
      matid.setNum(occluder[ii].GetMatID());
      ctwi1 = new cTWI(QStringList(QString(occluder[ii].GetName()))<<matid );
      ctwi0->addChild((QTreeWidgetItem *)ctwi1);
      ctwi1->SetIdx(idx);
      if (occluder[ii].IsSelected())
      {
        ctwi1->setCheckState(0,Qt::Checked);
      }
      else
      {
        ctwi1->setCheckState(0,Qt::Unchecked);
      }
    }
    idx++;
  }

  idx = BASE_ORIENTOCCLUDER; //base address for oriented occluders
  //oriented occluders
  ctwi0 = new cTWI(qtree, QStringList(QString("oriented occluder")));
  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    if (!orientoccluder[ii].IsDeleted())
    {
      matid.setNum(orientoccluder[ii].GetMatID());
      ctwi1 = new cTWI(QStringList(QString(orientoccluder[ii].GetName()))<<matid );
      ctwi0->addChild((QTreeWidgetItem *)ctwi1);
      ctwi1->SetIdx(idx);
      if (orientoccluder[ii].IsSelected())
      {
        ctwi1->setCheckState(0,Qt::Checked);
      }
      else
      {
        ctwi1->setCheckState(0,Qt::Unchecked);
      }
    }
    idx++;
  }

//areas of interest
/*/
  ctwi0 = new cTWI(qtree, QStringList(QString("areas of interest")));

  ctwi1 = new cTWI((QTreeWidgetItem *) ctwi0,QStringList(QString("full surface")));
  ctwi1->setCheckState(0,Qt::Unchecked);
  ctwi1 = new cTWI((QTreeWidgetItem *) ctwi0,QStringList(QString("all areas")));
  ctwi1->setCheckState(0,Qt::Checked);
  for (ii=0; ii<(long)aoi.size(); ii++)
  {
    ctwi1 = new cTWI((QTreeWidgetItem *) ctwi0,QStringList(QString("area #%1").arg(ii+1)));
    ctwi1->setCheckState(0,Qt::Unchecked);
  }
/*/
}

void cOSDL::PlotScene( long dlbase, gmMatrix4 drot, gmVector3 doff, bool xy )
{
  long ii;
  gmMatrix4 m_id;

  if (IsEmpty() || IsLoading()) return;

// plot area of interest
  for (ii=0; ii<(long)aoi.size(); ii++) aoi[ii].Plot();
  m_id = m_id.identity();

// plot primary surface
  if (!primary_surface.IsDeleted()) primary_surface.Display(dlbase);

// plot surfaces
  for (ii=0; ii<(long)surfaces.size(); ii++)
    if (!surfaces[ii].IsDeleted()) surfaces[ii].Display(dlbase);

// plot objects
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    if (!tessobj[ii].IsDeleted())
    {
      if (tessobj[ii].IsSelected())
      {
        tessobj[ii].Display(dlbase, drot, doff );
      }
      else
      {
        // plot surface
        tessobj[ii].Display(dlbase);
      }
    }
  }

  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    if (!orienttessobj[ii].IsDeleted())
    {
      if (orienttessobj[ii].IsSelected())
      {
        if (xy) orienttessobj[ii].Display(dlbase, drot, doff );
        else orienttessobj[ii].Display(dlbase, m_id, doff );
      }
      else
      {
        // plot surface
        tessobj[ii].Display(dlbase);
      }
    }
  }

  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    if (!occluder[ii].IsDeleted())
    {
      if (occluder[ii].IsSelected())
      {
        occluder[ii].Display(dlbase, drot, doff );
      }
      else
      {
        // plot surface
        occluder[ii].Display(dlbase);
      }
    }
  }

  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    if (!orientoccluder[ii].IsDeleted())
    {
      if (orientoccluder[ii].IsSelected())
      {
        if (xy) orientoccluder[ii].Display(dlbase, drot, doff );
        else orientoccluder[ii].Display(dlbase, m_id, doff );
      }
      else
      {
        // plot surface
        orientoccluder[ii].Display(dlbase);
      }

    }
  }
// finished

}

void cOSDL::PlotSceneAbout( long dlbase, gmMatrix4 drot, gmVector3 drotabout, gmVector3 doff, bool xy )
{
  long ii;
  gmMatrix4 m_id;

  gmVector3 aoff, dpos, npos;

  if (IsEmpty() || IsLoading() ) return;

// plot area of interest
  for (ii=0; ii<(long)aoi.size(); ii++) aoi[ii].Plot();
  m_id = m_id.identity();

// plot primary surface
  if (!primary_surface.IsDeleted()) primary_surface.Display(dlbase);

// plot surfaces
  for (ii=0; ii<(long)surfaces.size(); ii++)
    if (!surfaces[ii].IsDeleted()) surfaces[ii].Display(dlbase);

// plot objects
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    if (!tessobj[ii].IsDeleted())
    {
      if (tessobj[ii].IsSelected())
      {
        //compute offset based on rotation about the rotation point
        dpos = tessobj[ii].GetPos() - drotabout;
        npos = drot.transform(dpos);
        aoff = (drotabout+npos)-tessobj[ii].GetPos()+doff;

        tessobj[ii].Display(dlbase, drot, aoff );
        //tessobj[ii].Display(dlbase, drot, doff );
      }
      else
      {
        // plot surface
        tessobj[ii].Display(dlbase);
      }
    }
  }

  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    if (!orienttessobj[ii].IsDeleted())
    {
      if (orienttessobj[ii].IsSelected())
      {
        if (xy)
        {
          //compute offset based on rotation about the rotation point
          dpos = orienttessobj[ii].GetPos() - drotabout;
          npos = drot.transform(dpos);
          aoff = (drotabout+npos)-orienttessobj[ii].GetPos()+doff;

          orienttessobj[ii].Display(dlbase, drot, aoff );
        }
        //if (xy) orienttessobj[ii].Display(dlbase, drot, doff );
        else orienttessobj[ii].Display(dlbase, m_id, doff );
      }
      else
      {
        // plot surface
        orienttessobj[ii].Display(dlbase);
      }
    }
  }

  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    if (!occluder[ii].IsDeleted())
    {
      if (occluder[ii].IsSelected())
      {
        //compute offset based on rotation about the rotation point
        dpos = occluder[ii].GetPos() - drotabout;
        npos = drot.transform(dpos);
        aoff = (drotabout+npos)-occluder[ii].GetPos()+doff;

        occluder[ii].Display(dlbase, drot, aoff );
        //occluder[ii].Display(dlbase, drot, doff );
      }
      else
      {
        // plot surface
        occluder[ii].Display(dlbase);
      }
    }
  }

  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    if (!orientoccluder[ii].IsDeleted())
    {
      if (orientoccluder[ii].IsSelected())
      {
        if (xy)
        {
          //compute offset based on rotation about the rotation point
          dpos = orientoccluder[ii].GetPos() - drotabout;
          npos = drot.transform(dpos);
          aoff = (drotabout+npos)-orientoccluder[ii].GetPos()+doff;

          orientoccluder[ii].Display(dlbase, drot, aoff );
        }
        //if (xy) orientoccluder[ii].Display(dlbase, drot, doff );
        else orientoccluder[ii].Display(dlbase, m_id, doff );
      }
      else
      {
        // plot surface
        orientoccluder[ii].Display(dlbase);
      }
    }
  }
// finished
}

void cOSDL::PlotSceneSelect( long dlbase, gmMatrix4 drot, gmVector3 doff, bool xy )
{
  long ii, idx;
  gmMatrix4 m_id;

  if (IsEmpty() || IsLoading() ) return;

// plot area of interest
  for (ii=0; ii<(long)aoi.size(); ii++) aoi[ii].Plot();
  m_id = m_id.identity();

  idx = BASE_SURFACE;
// plot primary surface
  glLoadName(idx);
  if (!primary_surface.IsDeleted()) primary_surface.Display(dlbase);

  idx++;
// plot surfaces
  for (ii=0; ii<(long)surfaces.size(); ii++)
  {
    glLoadName(idx);
    if (!surfaces[ii].IsDeleted()) surfaces[ii].Display(dlbase);
    idx++;
  }

// plot objects
  idx = BASE_TESSOBJ;
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    glLoadName(idx);
    if (!tessobj[ii].IsDeleted())
    {
      if (tessobj[ii].IsSelected())
      {
        tessobj[ii].Display(dlbase, drot, doff );
      }
      else
      {
        // plot surface
        tessobj[ii].Display(dlbase);
      }
    }
    idx++;
  }

// plot orient objects
  idx = BASE_ORIENTTESSOBJ;
  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    glLoadName(idx);
    if (!orienttessobj[ii].IsDeleted())
    {
      if (orienttessobj[ii].IsSelected())
      {
        orienttessobj[ii].Display(dlbase, drot, doff );
      }
      else
      {
        // plot surface
        orienttessobj[ii].Display(dlbase);
      }
    }
    idx++;
  }

// plot occluders
  idx = BASE_OCCLUDER;
  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    glLoadName(idx);
    if (!occluder[ii].IsDeleted())
    {
      if (occluder[ii].IsSelected())
      {
        occluder[ii].Display(dlbase, drot, doff );
      }
      else
      {
        // plot surface
        occluder[ii].Display(dlbase);
      }
    }
    idx++;
  }

// plot orient occluders
  idx = BASE_ORIENTOCCLUDER;
  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    glLoadName(idx);
    if (!orientoccluder[ii].IsDeleted())
    {
      if (orientoccluder[ii].IsSelected())
      {
        if (xy) orientoccluder[ii].Display(dlbase, drot, doff );
        else orientoccluder[ii].Display(dlbase, m_id, doff );
      }
      else
      {
        // plot surface
        orientoccluder[ii].Display(dlbase);
      }
    }
    idx++;
  }
// finished
}

//////////////////////////////////////////////////////////////////////
// Object selection routines
//////////////////////////////////////////////////////////////////////
bool cOSDL::IDSelected( long isset )
{
  if (IsEmpty() || IsLoading() ) return (false);

  if (isset>=BASE_ORIENTOCCLUDER)
  {
    return (orientoccluder[isset-BASE_ORIENTOCCLUDER].IsSelected());
  }

  if (isset>=BASE_OCCLUDER)
  {
    return (occluder[isset-BASE_OCCLUDER].IsSelected());
  }

  if (isset>=BASE_ORIENTTESSOBJ)
  {
    return (orienttessobj[isset-BASE_ORIENTTESSOBJ].IsSelected());
  }

  if (isset>=BASE_TESSOBJ)
  {
    return (tessobj[isset-BASE_TESSOBJ].IsSelected());
  }

  if (isset>BASE_SURFACE)
  {
    return (surfaces[isset-BASE_SURFACE+1].IsSelected());
  }

  return false;
}

void cOSDL::ToggleSelectIndex( long isset )
{
  if (IsEmpty() || IsLoading()) return;

  if (isset>=BASE_ORIENTOCCLUDER)
  {
    SetSelect(isset,!orientoccluder[isset-BASE_ORIENTOCCLUDER].IsSelected());
    return;
  }

  if (isset>=BASE_OCCLUDER)
  {
    SetSelect(isset,!occluder[isset-BASE_OCCLUDER].IsSelected());
    return;
  }

  if (isset>=BASE_ORIENTTESSOBJ)
  {
    SetSelect(isset,!orienttessobj[isset-BASE_ORIENTTESSOBJ].IsSelected());
    return;
  }

  if (isset>=BASE_TESSOBJ)
  {
    SetSelect(isset,!tessobj[isset-BASE_TESSOBJ].IsSelected());
    return;
  }

  if (isset>BASE_SURFACE)
  {
    SetSelect(isset,!surfaces[isset-BASE_SURFACE+1].IsSelected());
    return;
  }
}

void cOSDL::SetSelect( long isset, bool bval )
{
  cerr << "Size of Surfaces is " << surfaces.size() << "\n";
  if (IsEmpty() || IsLoading()) return;

  if (isset>=BASE_ORIENTOCCLUDER)
  {
    if (bval==orientoccluder[isset-BASE_ORIENTOCCLUDER].IsSelected()) return;  //stop recursing
    orientoccluder[isset-BASE_ORIENTOCCLUDER].SetSelect(bval);
    emit selection(isset,bval);
    return;
  }

  if (isset>=BASE_OCCLUDER)
  {
    if (bval==occluder[isset-BASE_OCCLUDER].IsSelected()) return;  //stop recursing
    occluder[isset-BASE_OCCLUDER].SetSelect(bval);
    emit selection(isset,bval);
    return;
  }

  if (isset>=BASE_ORIENTTESSOBJ)
  {
    if (bval==orienttessobj[isset-BASE_ORIENTTESSOBJ].IsSelected()) return;  //stop recursing
    orienttessobj[isset-BASE_ORIENTTESSOBJ].SetSelect(bval);
    emit selection(isset,bval);
    return;
  }

  if (isset>=BASE_TESSOBJ)
  {
    if (bval==tessobj[isset-BASE_TESSOBJ].IsSelected()) return;  //stop recursing
    tessobj[isset-BASE_TESSOBJ].SetSelect(bval);
    emit selection(isset,bval);
    return;
  }

  if (isset>BASE_SURFACE)
  {
    if (bval==surfaces[isset-BASE_SURFACE+1].IsSelected()) return;  //stop recursing
    surfaces[isset-BASE_SURFACE+1].SetSelect(bval);
    emit selection(isset,bval);
    return;
  }
}

void cOSDL::SelectInBoundingBox( cAABB tester )
{
  gmVector3 pos;
  long ii;

  if (IsEmpty() || IsLoading()) return;

  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    pos = orientoccluder[ii].GetPos();
    if (tester.Inside(pos))
    {
      //orientoccluder[ii].SetSelect(!orientoccluder[ii].IsSelected());
      orientoccluder[ii].SetSelect(true);
      emit selection(ii+BASE_ORIENTOCCLUDER,orientoccluder[ii].IsSelected());
    }
  }

  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    pos = occluder[ii].GetPos();
    if (tester.Inside(pos))
    {
      //occluder[ii].SetSelect(!occluder[ii].IsSelected());
      occluder[ii].SetSelect(true);
      emit selection(ii+BASE_OCCLUDER,occluder[ii].IsSelected());
    }
  }

  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    pos = orienttessobj[ii].GetPos();
    if (tester.Inside(pos))
    {
      //orienttessobj[ii].SetSelect(!orienttessobj[ii].IsSelected());
      orienttessobj[ii].SetSelect(true);
      emit selection(ii+BASE_ORIENTTESSOBJ,orienttessobj[ii].IsSelected());
    }
  }

  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    pos = tessobj[ii].GetPos();
    if (tester.Inside(pos))
    {
      //tessobj[ii].SetSelect(!tessobj[ii].IsSelected());
      tessobj[ii].SetSelect(true);
      emit selection(ii+BASE_TESSOBJ,tessobj[ii].IsSelected());
    }
  }
}

void cOSDL::DeselectAll()
{
  long ii;

  primary_surface.SetSelect(false);

  for (ii=0; ii<(long)surfaces.size(); ii++)
  {
    surfaces[ii].SetSelect(false);
  }

  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    tessobj[ii].SetSelect(false);
  }

  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    orienttessobj[ii].SetSelect(false);
  }

  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    occluder[ii].SetSelect(false);
  }

  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    orientoccluder[ii].SetSelect(false);
  }
}

//////////////////////////////////////////////////////////////////////
// Read routines
//////////////////////////////////////////////////////////////////////
bool cOSDL::ReadObjectGeometry( ifstream& fin )
{
  string tag = "";
  bool no_name = true;

  string name="", id="";
  gmVector3 offset;
  char inln[200];

// order doesn't matter
  while (!(tag=="}") && !fin.eof())
  {
    fin >> tag;

    if ((tag[0]=='/') && (tag[1]=='/'))
    {
        fin.getline(inln, 199);  //finish the line
        continue;
    }

    if (tag == "file_name")
    {
      fin >> name;
      no_name = false;
    }

    if (tag == "id")
    {
      fin >> id;
    }

    if (tag == "base_offset")
    {
      fin >> offset[0] >> offset[1] >> offset[2];  //not used at present
    }

  }

  if (!fin.eof())  //no error - commit
  {
    if (no_name)
    {
      cout << "ReadObjectGeometry:  Object filename not found!\n";
    }
    else
    {
      if (id=="") id = name;
      AddGeometry( name );
    }
  }

  return (!fin.eof());
}

bool cOSDL::Read2DPolygon( ifstream& fin, cAOI& iaoi )
{
  string tag = "";
  double px, py;
  char inln[200];

  // order doesn't matter
  while (!(tag=="}") && !fin.eof())
  {
    fin >> tag;

    if ((tag[0]=='/') && (tag[1]=='/'))
    {
        fin.getline(inln, 199);  //finish the line
        continue;
    }

    if (tag == "pt2")
    {
      fin >> px >> py;
      iaoi.AddPoint(px,py);
    }
  }

  if (iaoi.GetNumPoints()<3) return false;

  return (!fin.eof());
}

bool cOSDL::ReadAreaOfInterest( ifstream& fin )
{
  string tag = "";

  gmVector3 offset;
  char inln[200];
  string nm;
  long mt;
  double bs, ht;
  cAOI iaoi;

// order doesn't matter
  while (!(tag=="}") && !fin.eof())
  {
    fin >> tag;

    if ((tag[0]=='/') && (tag[1]=='/'))
    {
        fin.getline(inln, 199);  //finish the line
        continue;
    }

    if (tag == "name")
    {
      fin >> nm;
    }

    if (tag == "matid")
    {
      fin >> mt;
    }

    if (tag == "base")
    {
      fin >> bs;
    }

    if (tag == "height")
    {
      fin >> ht;
    }

    if (tag == "poly2d")
    {
       if (!Read2DPolygon(fin, iaoi)) return false;
    }

  }

  if (!fin.eof())  //no error - commit
  {
    iaoi.SetName(nm.c_str());
    iaoi.SetMatID(mt);
    iaoi.SetHeights(bs, ht);
    aoi.push_back(iaoi);
  }
  SetAABB(AOI_FULL);

  return (!fin.eof());
}

bool cOSDL::Read( const char *fname )
{
  ifstream fin(pathfix(fname));  //our input file stream
  string tag;
  char inln[200];

  if(!fin)
  {
  cout << "Problem opening OSDL file named: " << fname << ".\n"
       << "Its pathfix name is : " << pathfix(fname) << "\n";
    return false;
  }

//the first line is our File ID
  fin >> tag;
  if (tag.find("OSDL") == string::npos)
  {
    cout << "Warning: Not an OSD file.\n";
    return false;
  }

// order doesn't matter
  while (!(tag=="}") && !fin.eof())
  {
    fin >> tag;

    if ((tag[0]=='/') && (tag[1]=='/'))
    {
        fin.getline(inln, 199);  //finish the line
        continue;
    }

    if (tag == "version")
    {
      fin >> version; //compare version #
    }

    if (tag == "object_geometry")
    {
      if (!ReadObjectGeometry( fin )) return false;
    }

    if (tag == "area_of_interest")
    {
      if (!ReadAreaOfInterest( fin )) return false;
    }

    if (tag == "%Omicron_Input")
    {
      fin >> tag;
      cout << "Reading Omicron Input File: "  << tag << "\n";
      ReadOmicronFile(tag.c_str());
    }

    if (tag == "%Veg_Input")
    {
      fin >> tag;
      cout << "Reading Veg Input File: "  << tag << "\n";
      ReadVegFile(tag.c_str());
    }

    if (tag == "%Omicron_Output")
    {
      fin >> tag;
      outputs.AddOmicronFileName( tag );
      cout << "Omicron Output: " << tag << "\n";
    }

    if (tag == "%Veg_Output")
    {
      fin >> tag;
      outputs.AddVegFileName( tag );
      cout << "Veg Output: " << tag << "\n";
    }

    if (tag == "%POVRay_Output")
    {
      fin >> tag;
      outputs.AddPOVRayFileName( tag );
      cout << "POV-Ray Output: " << tag << "\n";
    }

    if (tag == "%Wavefront_Output")
    {
      fin >> tag;
      outputs.AddWavefrontFileName( tag );
      cout << "Wavefront Output: " << tag << "\n";
    }

  }

  fin.close();
  return true;
}

cObjectGeometry * cOSDL::AddGeometry( string nfname )
{
  long ii;
  string ogid;
  cObjectGeometry *OG;

  for (ii=0; ii<(long)oglist.size(); ii++)
  {
    ogid.assign(oglist[ii]->GetObjectID());
    if (nfname == ogid)
    {
      return oglist[ii];
    }
  }

  OG = new cObjectGeometry;

  //progress message passthrough connections
  connect(OG, SIGNAL(startProgress( char * )), this, SLOT(startingProgress( char * )));
  connect(OG, SIGNAL(setProgressMax(int)), this, SLOT(getProgressMax(int)));
  connect(OG, SIGNAL(setProgressVal(int)), this, SLOT(getProgressVal(int)));
  connect(OG, SIGNAL(finishProgress()), this, SLOT(finishingProgress()));

  OG->ReadData(nfname.c_str());
  oglist.push_back(OG);
  ii=oglist.size()-1;

  return oglist[ii];
}

void cOSDL::ReadOmicronFile( const char *fname )
{
  string nfname;
  long ii, nobj, ito;
  double posx, posy, posz, rotz, roty, rotx, scl;
  double bottom;
  double b[4][2];
  cAOI naoi;
  cObjectDefinition ntess;
  cAABB taabb;

  if (!OmicronModel.Read( fname )) return;

  // Initialize for Omicron input - omicron defines the surface and area of interest
  surfaces.clear();

  //aoi.Init();
  aoi.clear();
  naoi.Init();
  aoi.push_back(naoi);


  OmicronModel.GetSurfaceData( nfname, posx, posy, posz, rotz, roty, rotx, scl);
//  surfaces.push_back(ntess);

  primary_surface.MakeLink(AddGeometry(nfname));
  OmicronModel.GetBoundaryData( b[0][0], b[0][1],
                                b[1][0], b[1][1],
                                b[2][0], b[2][1],
                                b[3][0], b[3][1]);
  aoi[0].AddPoint(b[0][0]+posx, b[0][1]+posy);
  aoi[0].AddPoint(b[1][0]+posx, b[1][1]+posy);
  aoi[0].AddPoint(b[2][0]+posx, b[2][1]+posy);
  aoi[0].AddPoint(b[3][0]+posx, b[3][1]+posy);

  primary_surface.SetUniformScale( scl );
  primary_surface.FixPosition(posx, posy, posz);
  primary_surface.RotationEuler(rotz, roty, rotx);

  primary_surface.SetColor( 0.3, 0.2, 0.01 );
  primary_surface.SetHighLightColor( 0.6, 0.4, 0.02 );

  bottom = OmicronModel.GetBottom();

  taabb = primary_surface.GetTransformedAABB();
  aoi[0].SetHeights( bottom, ((taabb.GetMax()[2] - bottom)*2.0) );

//get the full aabb of the surface and box
  SetAABB(AOI_FULL);
  //aabb = aoi[0].GetBoundingBox();

//the objects themselves
  nobj = OmicronModel.NumObjects();

  for (ii=0; ii<nobj; ii++)
  {
    tessobj.push_back(ntess);
    ito = tessobj.size()-1;

    OmicronModel.GetObjectData( ii, nfname, posx, posy, posz, rotz, roty, rotx, scl);

    tessobj[ito].MakeLink(AddGeometry(nfname));
    tessobj[ito].SetUniformScale(scl);
    tessobj[ito].ChangePosition(posx, posy, posz);
    tessobj[ito].RotationEuler(rotz, roty, rotx);
    tessobj[ito].SetColor( 0.3, 0.4, 0.3 );
    tessobj[ito].SetHighLightColor( 0.6, 0.8, 0.7 );
  }
}

void cOSDL::ReadVegFile( const char *fname )
{
  long ii, ito, numinst;
  string ifn;
  double iscl, irotz, iposx, iposy, iposz;
  cObjectDefinition ntess;

  if (!VegModel.Read(fname)) return;

  numinst = VegModel.GetNumberOfInstances();
  for (ii=0; ii<numinst; ii++)
  {
    VegModel.GetInstanceInformation( ii, ifn, iscl, irotz, iposx, iposy, iposz );
    orientoccluder.push_back(ntess);
    ito = orientoccluder.size()-1;
    orientoccluder[ito].MakeLink(AddGeometry(ifn));
    //orientoccluder[ito].ReadData(ifn.c_str());
    orientoccluder[ito].SetUniformScale(iscl);
    orientoccluder[ito].ChangePosition(iposx, iposy, iposz);
    orientoccluder[ito].RotationEuler(0.0, 0.0, 90.0);  //plants are sideways
    gmVector3 dvec;
    gmMatrix4 drot;
    drot = drot.identity();
    dvec.assign(0.0, 0.0, 1.0);
    drot = drot.rotate( 360.0 - irotz, dvec);
    orientoccluder[ito].ChangeRotation(drot);
    orientoccluder[ito].MakeSurfaceItem();
    orientoccluder[ito].SetColor( 0.2, 0.4, 0.2 );
    orientoccluder[ito].SetHighLightColor( 0.5, 0.8, 0.5 );
  }
}

//////////////////////////////////////////////////////////////////////
// Write routines
//////////////////////////////////////////////////////////////////////
bool cOSDL::WriteXML( const char *fname )
{
  ofstream fout(pathfix(fname));  //our input file stream
  string tag;
  //char inln[200];
  long ii;

  if(!fout)
  {
    cout << "Problem opening OSDL file.\n";
    return false;
  }

  //put out XML Header
  fout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  //Start the OSDL process
  fout << "<OSDL version=\""<< OSDL_VERSION << "\">\n";

  //Put out the time extents
  //Put out the list of Geometries
  for (ii=0; ii<(long)oglist.size(); ii++)
    oglist[ii]->WriteXML(fout);

  //Put out the Areas of Interest
  //Put out the Primary Surface
  fout << "  <surface>\n";
  fout << "    <primary />\n";
  primary_surface.WriteXML(fout);
  fout << "  </surface>\n";

  //Put out Ancillary Surfaces
  for (ii=0; ii<(long)surfaces.size(); ii++)
  {
    fout << "  <surface>\n";
    surfaces[ii].WriteXML(fout);
    fout << "  </surface>\n";
  }

  //Put out Tesselated Objects
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    fout << "  <mobject>\n";
    tessobj[ii].WriteXML(fout);
    fout << "  </mobject>\n";
  }

  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    fout << "  <mobject>\n";
    orienttessobj[ii].WriteXML(fout);
    fout << "    <FixRotZ />\n";
    fout << "  </mobject>\n";
  }

  //Put out Occluders
  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    fout << "  <occluder>\n";
    occluder[ii].WriteXML(fout);
    fout << "  </occluder>\n";
  }

  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    fout << "  <occluder>\n";
    orientoccluder[ii].WriteXML(fout);
    fout << "    <FixRotZ />\n";
    fout << "  </occluder>\n";
  }

  //Finish the OSDL process
  fout << "</OSDL>\n";

  fout.close();
  return true;
}

void cOSDL::ProcessOutputs()
{
  string tag;

  Clean();

  if (outputs.GetOmicronFileName( tag ) )
    WriteOmicronFile(tag.c_str());
  if (outputs.GetVegFileName( tag ) )
    WriteVegFile(tag.c_str());
  if (outputs.GetPOVRayFileName( tag ) )
    WritePOVRayFile(tag.c_str());
  if (outputs.GetWavefrontFileName( tag ) )
    WriteWavefrontFile(tag.c_str());
}

void cOSDL::WriteOmicronFile( const char *fname )
{
  double dx, dy, dz, rotz, roty, rotx, px, py, pz;
  long ii, numreg;
  string dummy;

  primary_surface.GetOffset( dx, dy, dz );
  primary_surface.GetPos( px, py, pz );
  dx += px; dy += py; dz += pz;
  primary_surface.MakeEuler( rotz, roty, rotx );

  printf("  Omicron - %s\n",primary_surface.GetLink()->GetFileName());
  cout << dummy << endl;

  OmicronModel.SetSurfaceData(primary_surface.GetLink()->GetFileName(), dx, dy, dz, rotz, roty, rotx, primary_surface.GetUniformScale());

  OmicronModel.MakeRegionList( tessobj.size()+orienttessobj.size() );
  numreg = 0;
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
      tessobj[ii].GetOffset( dx, dy, dz );
      tessobj[ii].GetPos( px, py, pz );
      dx += px; dy += py; dz += pz;
      tessobj[ii].MakeEuler( rotz, roty, rotx );
      OmicronModel.SetObjectData(numreg, tessobj[ii].GetLink()->GetFileName(), dx, dy, dz, rotz, roty, rotx, tessobj[ii].GetUniformScale());
      numreg++;
  }
  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
      orienttessobj[ii].GetOffset( dx, dy, dz );
      orienttessobj[ii].GetPos( px, py, pz );
      dx += px; dy += py; dz += pz;
      orienttessobj[ii].MakeEuler( rotz, roty, rotx );
      OmicronModel.SetObjectData(numreg, orienttessobj[ii].GetLink()->GetFileName(), dx, dy, dz, rotz, roty, rotx, tessobj[ii].GetUniformScale());
      numreg++;
  }

  OmicronModel.Write( fname );
}

void cOSDL::WriteVegFile( const char *fname )
{
  long ii, numinst;
  double px, py, pz, dx, dy, dz, yaw;

  numinst=0;
  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    if (orientoccluder[ii].IsSurfaceItem())
    {
      orientoccluder[ii].GetOffset( dx, dy, dz );
      orientoccluder[ii].GetPos( px, py, pz );
      dx += px; dy += py; dz += pz;
      yaw = orientoccluder[ii].GetRotZ();
      VegModel.FixInstanceInformation(numinst, orientoccluder[ii].GetUniformScale(), yaw, dx, dy, dz);
      numinst++;
    }
  }

  VegModel.Write(fname);
}

void cOSDL::WritePOVRayFile( const char *fname )
{
  ofstream fout(pathfix(fname));  //our input file stream
  string cmd;
  cAABB aabb;
  gmVector3 midv, deltv;
  gmVector3 center;
  gmVector3 vnd1, vnd2, vnd3;
  double adj, opp, hyp, adjn, oppn;
  long ii;

  if(!fout)
  {
    cout << "Problem opening POVRay output file: " << fname <<".\n";
    return;
  }

  // aabb = aoi[0].GetBoundingBox();
  aabb.Reset();
  for (ii=0; ii<(long)aoi.size(); ii++)
  {
    cAABB naabb;
    naabb=aoi[ii].GetBoundingBox();
    aabb.Adjust(naabb.GetMin());
    aabb.Adjust(naabb.GetMax());
  }

  deltv = aabb.GetRange();
  midv = aabb.GetMid();

  adj = 1.5 * deltv[0];
  //opp = 0.5 * deltv[2];
  //opp = deltv[2];
  opp = adj;
  hyp = sqrt( ( adj * adj ) + ( opp * opp ) );
  adjn = adj / hyp;
  oppn = opp / hyp;

  fout << "// === OSDL POVRay output ===\n\n";
  fout << "// === Change camera settings here ===\n";
  fout << "  camera\n  {\n    perspective\n";
  fout << "    location <" << midv[0] << " + ( "<< adj << " * cos(clock*2*pi) ), "
                           << midv[1] << " + ( "<< adj << " * sin(clock*2*pi) ), "
                           << midv[2] << " + "<< opp << ">\n";
  fout << "    direction <" << -adjn << " * cos(clock*2*pi), "
                            << -adjn << " * sin(clock*2*pi), "
                            << -oppn << ">\n";
  fout << "    up <" << -oppn << " * cos(clock*2*pi), "
                     << -oppn << " * sin(clock*2*pi), "
                     << adjn << ">\n";
  fout << "    right < 1.25 * sin(clock*2*pi), 1.25 * -cos(clock*2*pi), 0>\n";
  fout << "  }\n\n";
  fout << "// === Change light settings here ===\n";
  fout << "  light_source\n  {\n";
  fout << "    <" << -deltv[0] << ", " << -deltv[1] << ", " << deltv[2] << ">\n";
  fout << "    color rgb <1.0, 1.0, 1.0>\n    parallel\n";
  fout << "    point_at <0, 0, 0>\n";
  fout << "  }\n\n";
  fout << "  light_source\n  {\n";
  fout << "    <" << deltv[0] << ", " << deltv[1] << ", " << deltv[2] << ">\n";
  fout << "    color rgb <0.5, 0.5, 0.5>\n    parallel\n";
  fout << "    point_at <0, 0, 0>\n";
  fout << "  }\n\n";


  //Geometry
  fout << "union\n{\n";
  // primary surface
  fout << "  union\n{\n";

  primary_surface.WritePOVRay( fout );

  fout << " texture { pigment { color <0.6, 0.4, 0.02> } } }\n\n";

  // surfacevv
  for (ii=0; ii<(long)surfaces.size(); ii++)
  {
    fout << "  union\n{\n";

    surfaces[ii].WritePOVRay( fout );

    fout << " texture { pigment { color <0.6, 0.4, 0.02> } } }\n\n";
  }
  // tesselated objects
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    fout << "union\n{\n";

    tessobj[ii].WritePOVRay( fout );

    fout << "  texture { pigment { color <0.6, 0.8, 0.7> } } }\n\n";
  }
  // oriented tesselated objects
  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    fout << "  union\n{\n";

    orienttessobj[ii].WritePOVRay( fout );

    fout << "  texture { pigment { color <0.5, 0.8, 0.5> } } }\n\n";
  }
  // occluders
  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    fout << "union\n{\n";

    occluder[ii].WritePOVRay( fout );

    fout << "  texture { pigment { color <0.6, 0.8, 0.7> } } }\n\n";
  }
  // oriented occluder
  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    fout << "  union\n{\n";

    orientoccluder[ii].WritePOVRay( fout );

    fout << "  texture { pigment { color <0.5, 0.8, 0.5> } } }\n\n";
  }
  fout << "}\n\n";
  fout.close();
}

void cOSDL::WriteWavefrontFile( const char *fname )
{
  ofstream fout(pathfix(fname));  //our input file stream
  ofstream mtl("SceneGen.mtl");
  long ii, voff;
  char namer[200];

  if(!fout)
  {
    cout << "Problem opening Wavefront output file: " << fname <<".\n";
    return;
  }

// ********** Make sure we have a material file handy **********
  mtl << "# simple Wavefront Material file written by SceneGen\n";
  mtl << "#   by Barry C White\n\n";

  mtl << "newmtl surface\n";
  mtl << "	Kd 0.6 0.4 0.02\n";
  mtl << "	Ks 0.6 0.4 0.02\n";
  mtl << "	Ka 0.3 0.2 0.01\n\n";

  mtl << "newmtl object\n";
  mtl << "	Kd 0.6 0.8 0.7\n";
  mtl << "	Ks 0.6 0.8 0.7\n";
  mtl << "	Ka 0.3 0.4 0.3\n\n";

  mtl << "newmtl surfobj\n";
  mtl << "	Kd 0.5 0.8 0.5\n";
  mtl << "	Ks 0.5 0.8 0.5\n";
  mtl << "	Ka 0.2 0.4 0.2\n\n";

  mtl.close();


// ********** Now write the actual object file **********
  fout << "# Simplified Obj export function from OSDL by Barry C White\n\n";

  fout << "mtllib SceneGen.mtl\n\n";
  //Geometry

  voff=1;

  // primary surface
  fout << "# primary surface\n";

  sprintf(namer,"primary_surface");
  fout << "g " << namer << "\n";
  fout << "usemtl surface\n";

  primary_surface.WriteWavefront( fout, voff );

  fout << "\n\n";

  // surface
  for (ii=0; ii<(long)surfaces.size(); ii++)
  {
    fout << "# surface " << (ii+1) << "\n";

    sprintf(namer,"surface%03ld",(ii+1));
    fout << "g " << namer << "\n";
    fout << "usemtl surface\n";

    surfaces[ii].WriteWavefront( fout, voff );

    fout << "\n\n";
  }

  // tesselated object
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    fout << "# tesselated object " << (ii+1) << "\n";

    sprintf(namer,"tessobject%03ld",(ii+1));
    fout << "g " << namer << "\n";
    fout << "usemtl object\n";

    tessobj[ii].WriteWavefront( fout, voff );

    fout << "\n\n";
  }

  // oriented tesselated object
  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    fout << "# oriented-tessobject " << (ii+1) << "\n";

    sprintf(namer,"orienttessobj%03ld",(ii+1));
    fout << "g " << namer << "\n";
    fout << "usemtl surfobj\n";

    orienttessobj[ii].WriteWavefront( fout, voff );

    fout << "\n\n";
  }

  // occluder
  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    fout << "# occluder " << (ii+1) << "\n";

    sprintf(namer,"occluder%03ld",(ii+1));
    fout << "g " << namer << "\n";
    fout << "usemtl object\n";

    occluder[ii].WriteWavefront( fout, voff );

    fout << "\n\n";
  }

  // oriented occluders
  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    fout << "# oriented-occluder " << (ii+1) << "\n";

    sprintf(namer,"orientoccluder%03ld",(ii+1));
    fout << "g " << namer << "\n";
    fout << "usemtl surfobj\n";

    orientoccluder[ii].WriteWavefront( fout, voff );

    fout << "\n\n";
  }

  fout.close();
}

//////////////////////////////////////////////////////////////////////
// Selected object change routines - UNDO added here
//////////////////////////////////////////////////////////////////////

void cOSDL::Undo_Del( cOSDL_Undo *cundo )
{
  long ii, id;

  for (ii=0; ii<(long)(cundo->objectid.size()); ii++)
  {
    id = cundo->objectid[ii];

    if (id>=BASE_ORIENTOCCLUDER)
    {
      orientoccluder[id-BASE_ORIENTOCCLUDER].SetDeleted(false);
      continue;
    }

    if (id>=BASE_OCCLUDER)
    {
      occluder[id-BASE_OCCLUDER].SetDeleted(false);
      continue;
    }

    if (id>=BASE_ORIENTTESSOBJ)
    {
      orienttessobj[id-BASE_ORIENTTESSOBJ].SetDeleted(false);
      continue;
    }

    if (id>=BASE_TESSOBJ)
    {
      tessobj[id-BASE_TESSOBJ].SetDeleted(false);
      continue;
    }

    if (id>BASE_SURFACE)
    {
      surfaces[id-(BASE_SURFACE+1)].SetDeleted(false);
      continue;
    }
  }

}

void cOSDL::Undo_CSP( cOSDL_Undo *cundo )
{
  long ii, id;

  for (ii=0; ii<(long)(cundo->objectid.size()); ii++)
  {
    id = cundo->objectid[ii];

    if (id>=BASE_ORIENTOCCLUDER)
    {
      orientoccluder[id-BASE_ORIENTOCCLUDER].ChangePosition(-cundo->doff[0], -cundo->doff[1], -cundo->doff[2]);
      continue;
    }

    if (id>=BASE_OCCLUDER)
    {
      occluder[id-BASE_OCCLUDER].ChangePosition(-cundo->doff[0], -cundo->doff[1], -cundo->doff[2]);
      continue;
    }

    if (id>=BASE_ORIENTTESSOBJ)
    {
      orienttessobj[id-BASE_ORIENTTESSOBJ].ChangePosition(-cundo->doff[0], -cundo->doff[1], -cundo->doff[2]);
      continue;
    }

    if (id>=BASE_TESSOBJ)
    {
      tessobj[id-BASE_TESSOBJ].ChangePosition(-cundo->doff[0], -cundo->doff[1], -cundo->doff[2]);
      continue;
    }

    if (id>BASE_SURFACE)
    {
      surfaces[id-(BASE_SURFACE+1)].ChangePosition(-cundo->doff[0], -cundo->doff[1], -cundo->doff[2]);
      continue;
    }
  }

}

void cOSDL::Undo_CSR( cOSDL_Undo *cundo )
{
  long ii, id;
  gmMatrix4 drott;

  drott = cundo->drot.transpose();

  for (ii=0; ii<(long)(cundo->objectid.size()); ii++)
  {
    id = cundo->objectid[ii];

    if (id>=BASE_ORIENTOCCLUDER)
    {
      if (cundo->xy) orientoccluder[id-BASE_ORIENTOCCLUDER].ChangeRotation(drott);
      continue;
    }

    if (id>=BASE_OCCLUDER)
    {
      occluder[id-BASE_OCCLUDER].ChangeRotation(drott);
      continue;
    }

    if (id>=BASE_ORIENTTESSOBJ)
    {
      if (cundo->xy) orienttessobj[id-BASE_ORIENTTESSOBJ].ChangeRotation(drott);
      continue;
    }

    if (id>=BASE_TESSOBJ)
    {
      tessobj[id-BASE_TESSOBJ].ChangeRotation(drott);
      continue;
    }
  }

}

void cOSDL::Undo_CSRA( cOSDL_Undo *cundo )
{
  long ii, id;
  gmMatrix4 drott;
  gmVector3 dpos, npos, noff;

  drott = cundo->drot.transpose();

  for (ii=0; ii<(long)(cundo->objectid.size()); ii++)
  {
    id = cundo->objectid[ii];

    if (id>=BASE_ORIENTOCCLUDER)
    {
      if (cundo->xy)
      {
        //Get position relative to the rotation point
        dpos = orientoccluder[id-BASE_ORIENTOCCLUDER].GetPos() - cundo->doff;
        npos = drott.transform(dpos);
        noff = (cundo->doff+npos)-orientoccluder[id-BASE_ORIENTOCCLUDER].GetPos();
        orientoccluder[id-BASE_ORIENTOCCLUDER].ChangePosition(noff[0], noff[1], noff[2]);
        orientoccluder[id-BASE_ORIENTOCCLUDER].ChangeRotation(drott);
      }
      continue;
    }

    if (id>=BASE_OCCLUDER)
    {
      //Get position relative to the rotation point
      dpos = occluder[id-BASE_OCCLUDER].GetPos() - cundo->doff;
      npos = drott.transform(dpos);
      noff = (cundo->doff+npos)-occluder[id-BASE_OCCLUDER].GetPos();
      occluder[id-BASE_OCCLUDER].ChangePosition(noff[0], noff[1], noff[2]);
      occluder[id-BASE_OCCLUDER].ChangeRotation(drott);
      continue;
    }

    if (id>=BASE_ORIENTTESSOBJ)
    {
      if (cundo->xy)
      {
        //Get position relative to the rotation point
        dpos = orienttessobj[id-BASE_ORIENTTESSOBJ].GetPos() - cundo->doff;
        npos = drott.transform(dpos);
        noff = (cundo->doff+npos)-orienttessobj[id-BASE_ORIENTTESSOBJ].GetPos();
        orienttessobj[id-BASE_ORIENTTESSOBJ].ChangePosition(noff[0], noff[1], noff[2]);
        orienttessobj[id-BASE_ORIENTTESSOBJ].ChangeRotation(drott);
      }
      continue;
    }

    if (id>=BASE_TESSOBJ)
    {
      //Get position relative to the rotation point
      dpos = tessobj[id-BASE_TESSOBJ].GetPos() - cundo->doff;
      npos = drott.transform(dpos);
      noff = (cundo->doff+npos)-tessobj[id-BASE_TESSOBJ].GetPos();
      tessobj[id-BASE_TESSOBJ].ChangePosition(noff[0], noff[1], noff[2]);
      tessobj[id-BASE_TESSOBJ].ChangeRotation(drott);
      continue;
    }
  }

}

void cOSDL::Undo_CSID( cOSDL_Undo *cundo )
{
  long ii, id;

  for (ii=0; ii<(long)(cundo->objectid.size()); ii++)
  {
    id = cundo->objectid[ii];

    if (id>=BASE_ORIENTOCCLUDER)
    {
      orientoccluder[id-BASE_ORIENTOCCLUDER].SetMatID(cundo->matid[ii]);
      continue;
    }

    if (id>=BASE_OCCLUDER)
    {
      occluder[id-BASE_OCCLUDER].SetMatID(cundo->matid[ii]);
      continue;
    }

    if (id>=BASE_ORIENTTESSOBJ)
    {
      orienttessobj[id-BASE_ORIENTTESSOBJ].SetMatID(cundo->matid[ii]);
      continue;
    }

    if (id>=BASE_TESSOBJ)
    {
      tessobj[id-BASE_TESSOBJ].SetMatID(cundo->matid[ii]);
      continue;
    }
  }

}

void cOSDL::Undo_CSS( cOSDL_Undo *cundo )
{
  long ii, id;
  gmVector3 sscl;

  for (ii=0; ii<(long)(cundo->objectid.size()); ii++)
  {
    id = cundo->objectid[ii];

    if (id>=BASE_ORIENTOCCLUDER)
    {
      sscl = cundo->sclid[ii];
      orientoccluder[id-BASE_ORIENTOCCLUDER].SetScale(sscl[0],sscl[1],sscl[2]);
      continue;
    }

    if (id>=BASE_OCCLUDER)
    {
      sscl = cundo->sclid[ii];
      occluder[id-BASE_OCCLUDER].SetScale(sscl[0],sscl[1],sscl[2]);
      continue;
    }

    if (id>=BASE_ORIENTTESSOBJ)
    {
      sscl = cundo->sclid[ii];
      orienttessobj[id-BASE_ORIENTTESSOBJ].SetScale(sscl[0],sscl[1],sscl[2]);
      continue;
    }

    if (id>=BASE_TESSOBJ)
    {
      sscl = cundo->sclid[ii];
      tessobj[id-BASE_TESSOBJ].SetScale(sscl[0],sscl[1],sscl[2]);
      continue;
    }
  }

}

void cOSDL::Undo_CSRS( cOSDL_Undo *cundo )
{
  long ii, id;
  gmVector3 sscl, cscl;

  cscl[0] = 1.0 / cundo->scale[0];
  cscl[1] = 1.0 / cundo->scale[1];
  cscl[2] = 1.0 / cundo->scale[2];

  for (ii=0; ii<(long)(cundo->objectid.size()); ii++)
  {
    id = cundo->objectid[ii];

    if (id>=BASE_ORIENTOCCLUDER)
    {
      sscl = orientoccluder[id-BASE_ORIENTOCCLUDER].GetScale();
      orientoccluder[id-BASE_ORIENTOCCLUDER].SetScale(sscl[0]*cscl[0],sscl[1]*cscl[1],sscl[2]*cscl[2]);
      continue;
    }

    if (id>=BASE_OCCLUDER)
    {
      sscl = occluder[id-BASE_OCCLUDER].GetScale();
      occluder[id-BASE_OCCLUDER].SetScale(sscl[0]*cscl[0],sscl[1]*cscl[1],sscl[2]*cscl[2]);
      continue;
    }

    if (id>=BASE_ORIENTTESSOBJ)
    {
      sscl = orienttessobj[id-BASE_ORIENTTESSOBJ].GetScale();
      orienttessobj[id-BASE_ORIENTTESSOBJ].SetScale(sscl[0]*cscl[0],sscl[1]*cscl[1],sscl[2]*cscl[2]);
      continue;
    }

    if (id>=BASE_TESSOBJ)
    {
      sscl = tessobj[id-BASE_TESSOBJ].GetScale();
      tessobj[id-BASE_TESSOBJ].SetScale(sscl[0]*cscl[0],sscl[1]*cscl[1],sscl[2]*cscl[2]);
      continue;
    }
  }

}

void cOSDL::Undo_PS( cOSDL_Undo *cundo )
{
  long ii, id;
  gmVector3 opt;

  for (ii=0; ii<(long)(cundo->objectid.size()); ii++)
  {
    id = cundo->objectid[ii];

    if (id>=BASE_ORIENTOCCLUDER)
    {
      opt = cundo->sclid[ii];
      orientoccluder[id-BASE_ORIENTOCCLUDER].SetPosition(opt[0],opt[1],opt[2]);
      continue;
    }

    if (id>=BASE_OCCLUDER)
    {
      opt = cundo->sclid[ii];
      occluder[id-BASE_OCCLUDER].SetPosition(opt[0],opt[1],opt[2]);
      continue;
    }

    if (id>=BASE_ORIENTTESSOBJ)
    {
      opt = cundo->sclid[ii];
      orienttessobj[id-BASE_ORIENTTESSOBJ].SetPosition(opt[0],opt[1],opt[2]);
      continue;
    }

    if (id>=BASE_TESSOBJ)
    {
      opt = cundo->sclid[ii];
      tessobj[id-BASE_TESSOBJ].SetPosition(opt[0],opt[1],opt[2]);
      continue;
    }
  }

}

void cOSDL::Undo()
{
  cOSDL_Undo *cundo;

  //No need to do anything if the undo list is empty
  if (ulist.empty()) return;

  cout << "Num of undo items = " << ulist.size() << "\n";
  cundo = ulist[0];

  switch( cundo->op )
  {
    case UNDO_DEL:
      Undo_Del(cundo);
      break;
    case UNDO_CSP:
      Undo_CSP(cundo);
      break;
    case UNDO_CSR:
      Undo_CSR(cundo);
      break;
    case UNDO_CSRA:
      Undo_CSRA(cundo);
      break;
    case UNDO_CSID:
      Undo_CSID(cundo);
      break;
    case UNDO_CSS:
      Undo_CSS(cundo);
      break;
    case UNDO_CSRS:
      Undo_CSRS(cundo);
      break;
    case UNDO_PSPS:
    case UNDO_PSM:
      Undo_PS(cundo);
      break;
    default:
      break;
  }

  //remove from stack
  ulist.pop_front();
  delete cundo;
}

void cOSDL::DeleteSelectedItems( )
{
  long ii, idbase;
  cOSDL_Undo *cundo = new cOSDL_Undo;

  cundo->op = UNDO_DEL;

  idbase = BASE_SURFACE;
  for (ii=0; ii<(long)surfaces.size(); ii++)
  {
    if (surfaces[ii].IsSelected())
    {
      surfaces[ii].SetDeleted(true);
      cundo->objectid.push_back(idbase+1+ii);
    }
  }

  idbase = BASE_TESSOBJ;
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    if (tessobj[ii].IsSelected())
    {
      tessobj[ii].SetDeleted(true);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  idbase = BASE_ORIENTTESSOBJ;
  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    if (orienttessobj[ii].IsSelected())
    {
      orienttessobj[ii].SetDeleted(true);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  idbase = BASE_OCCLUDER;
  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    if (occluder[ii].IsSelected())
    {
      occluder[ii].SetDeleted(true);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  idbase = BASE_ORIENTOCCLUDER;
  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    if (orientoccluder[ii].IsSelected())
    {
      orientoccluder[ii].SetDeleted(true);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  if (!cundo->objectid.empty()) ulist.push_front(cundo);
}

void cOSDL::ChangeSelectPosition( gmVector3 doff )
{
  long ii, idbase;
  cOSDL_Undo *cundo = new cOSDL_Undo;

  cundo->op = UNDO_CSP;
  cundo->doff = doff;

  idbase = BASE_SURFACE;
  for (ii=0; ii<(long)surfaces.size(); ii++)
  {
    if (surfaces[ii].IsSelected())
    {
      surfaces[ii].ChangePosition(doff[0], doff[1], doff[2]);
      cundo->objectid.push_back(idbase+ii+1);
    }
  }

  idbase = BASE_TESSOBJ;
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    if (tessobj[ii].IsSelected())
    {
      tessobj[ii].ChangePosition(doff[0], doff[1], doff[2]);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  idbase = BASE_ORIENTTESSOBJ;
  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    if (orienttessobj[ii].IsSelected())
    {
      orienttessobj[ii].ChangePosition(doff[0], doff[1], doff[2]);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  idbase = BASE_OCCLUDER;
  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    if (occluder[ii].IsSelected())
    {
      occluder[ii].ChangePosition(doff[0], doff[1], doff[2]);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  idbase = BASE_ORIENTOCCLUDER;
  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    if (orientoccluder[ii].IsSelected())
    {
      orientoccluder[ii].ChangePosition(doff[0], doff[1], doff[2]);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  if (!cundo->objectid.empty()) ulist.push_front(cundo);
}

void cOSDL::ChangeSelectRotation( gmMatrix4 drot, bool xy )
{
  long ii, idbase;
  cOSDL_Undo *cundo = new cOSDL_Undo;

  cundo->op = UNDO_CSR;
  cundo->drot = drot;
  cundo->xy = xy;

  idbase = BASE_TESSOBJ;
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    if (tessobj[ii].IsSelected())
    {
      tessobj[ii].ChangeRotation(drot);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  idbase = BASE_ORIENTTESSOBJ;
  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    if (orienttessobj[ii].IsSelected())
    {
      if (xy) orienttessobj[ii].ChangeRotation(drot);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  idbase = BASE_OCCLUDER;
  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    if (occluder[ii].IsSelected())
    {
      occluder[ii].ChangeRotation(drot);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  idbase = BASE_ORIENTOCCLUDER;
  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    if (orientoccluder[ii].IsSelected())
    {
      if (xy) orientoccluder[ii].ChangeRotation(drot);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  if (!cundo->objectid.empty()) ulist.push_front(cundo);
}

void cOSDL::ChangeSelectRotationAbout( gmVector3 doff, gmMatrix4 drot, bool xy )
{
  long ii, idbase;
  gmVector3 dpos, npos, noff;
  cOSDL_Undo *cundo = new cOSDL_Undo;

  cundo->op = UNDO_CSRA;
  cundo->doff = doff;
  cundo->drot = drot;
  cundo->xy = xy;

  idbase = BASE_TESSOBJ;
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    if (tessobj[ii].IsSelected())
    {
      //Get position relative to the rotation point
      dpos = tessobj[ii].GetPos() - doff;
      npos = drot.transform(dpos);
      noff = (doff+npos)-tessobj[ii].GetPos();
      tessobj[ii].ChangePosition(noff[0], noff[1], noff[2]);

      tessobj[ii].ChangeRotation(drot);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  idbase = BASE_ORIENTTESSOBJ;
  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    if (orienttessobj[ii].IsSelected())
    {
      if (xy)
      {
        //Get position relative to the rotation point
        dpos = orienttessobj[ii].GetPos() - doff;
        npos = drot.transform(dpos);
        noff = (doff+npos)-orienttessobj[ii].GetPos();
        orienttessobj[ii].ChangePosition(noff[0], noff[1], noff[2]);
        orienttessobj[ii].ChangeRotation(drot);
      }
      cundo->objectid.push_back(idbase+ii);
    }
  }

  idbase = BASE_OCCLUDER;
  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    if (occluder[ii].IsSelected())
    {
      //Get position relative to the rotation point
      dpos = occluder[ii].GetPos() - doff;
      npos = drot.transform(dpos);
      noff = (doff+npos)-occluder[ii].GetPos();
      occluder[ii].ChangePosition(noff[0], noff[1], noff[2]);
      occluder[ii].ChangeRotation(drot);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  idbase = BASE_ORIENTOCCLUDER;
  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    if (orientoccluder[ii].IsSelected())
    {
      if (xy)
      {
        //Get position relative to the rotation point
        dpos = orientoccluder[ii].GetPos() - doff;
        npos = drot.transform(dpos);
        noff = (doff+npos)-orientoccluder[ii].GetPos();
        orientoccluder[ii].ChangePosition(noff[0], noff[1], noff[2]);
        orientoccluder[ii].ChangeRotation(drot);
      }
      cundo->objectid.push_back(idbase+ii);
    }
  }

  if (!cundo->objectid.empty()) ulist.push_front(cundo);
}

void cOSDL::ChangeSelectMatID( int BMID )
{
  long ii, idbase;
  cOSDL_Undo *cundo = new cOSDL_Undo;

  cundo->op = UNDO_CSID;

  idbase = BASE_TESSOBJ;
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    if (tessobj[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      cundo->matid.push_back(tessobj[ii].GetMatID());
      tessobj[ii].SetMatID(BMID);
    }
  }

  idbase = BASE_ORIENTTESSOBJ;
  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    if (orienttessobj[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      cundo->matid.push_back(orienttessobj[ii].GetMatID());
      orienttessobj[ii].SetMatID(BMID);
    }
  }

  idbase = BASE_OCCLUDER;
  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    if (occluder[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      cundo->matid.push_back(occluder[ii].GetMatID());
      occluder[ii].SetMatID(BMID);
    }
  }

  idbase = BASE_ORIENTOCCLUDER;
  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    if (orientoccluder[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      cundo->matid.push_back(orientoccluder[ii].GetMatID());
      orientoccluder[ii].SetMatID(BMID);
    }
  }

  if (!cundo->objectid.empty()) ulist.push_front(cundo);
}

void cOSDL::ChangeSelectScale( double sclx, double scly, double sclz )
{
  long ii, idbase;
  gmVector3 gscale;
  cOSDL_Undo *cundo = new cOSDL_Undo;

  cundo->op = UNDO_CSS;

  idbase = BASE_TESSOBJ;
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    if (tessobj[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      gscale = tessobj[ii].GetScale();
      cundo->sclid.push_back(gscale);
      tessobj[ii].SetScale(sclx, scly, sclz);
    }
  }

  idbase = BASE_ORIENTTESSOBJ;
  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    if (orienttessobj[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      gscale = orienttessobj[ii].GetScale();
      cundo->sclid.push_back(gscale);
      orienttessobj[ii].SetScale(sclx, scly, sclz);
    }
  }

  idbase = BASE_OCCLUDER;
  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    if (occluder[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      gscale = occluder[ii].GetScale();
      cundo->sclid.push_back(gscale);
      occluder[ii].SetScale(sclx, scly, sclz);
    }
  }

  idbase = BASE_ORIENTOCCLUDER;
  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    if (orientoccluder[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      gscale = orientoccluder[ii].GetScale();
      cundo->sclid.push_back(gscale);
      orientoccluder[ii].SetScale(sclx, scly, sclz);
    }
  }

  if (!cundo->objectid.empty()) ulist.push_front(cundo);
}

void cOSDL::ChangeSelectRelativeScale( double sclx, double scly, double sclz )
{
  long ii, idbase;
  double bsclx, bscly, bsclz;
  gmVector3 gscale(sclx,scly,sclz);
  cOSDL_Undo *cundo = new cOSDL_Undo;

  cundo->op = UNDO_CSRS;
  cundo->scale = gscale;

  idbase = BASE_TESSOBJ;
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    if (tessobj[ii].IsSelected())
    {
      tessobj[ii].GetScale(bsclx, bscly, bsclz);
      bsclx *= sclx;
      bscly *= scly;
      bsclz *= sclz;
      tessobj[ii].SetScale(bsclx, bscly, bsclz);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  idbase = BASE_ORIENTTESSOBJ;
  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    if (orienttessobj[ii].IsSelected())
    {
      orienttessobj[ii].GetScale(bsclx, bscly, bsclz);
      bsclx *= sclx;
      bscly *= scly;
      bsclz *= sclz;
      orienttessobj[ii].SetScale(bsclx, bscly, bsclz);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  idbase = BASE_OCCLUDER;
  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    if (occluder[ii].IsSelected())
    {
      occluder[ii].GetScale(bsclx, bscly, bsclz);
      bsclx *= sclx;
      bscly *= scly;
      bsclz *= sclz;
      occluder[ii].SetScale(bsclx, bscly, bsclz);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  idbase = BASE_ORIENTOCCLUDER;
  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    if (orientoccluder[ii].IsSelected())
    {
      orientoccluder[ii].GetScale(bsclx, bscly, bsclz);
      bsclx *= sclx;
      bscly *= scly;
      bsclz *= sclz;
      orientoccluder[ii].SetScale(bsclx, bscly, bsclz);
      cundo->objectid.push_back(idbase+ii);
    }
  }

  if (!cundo->objectid.empty()) ulist.push_front(cundo);
}

void cOSDL::ProjectSelectedToPrimarySurface()
{
  long ii, idbase;
  gmVector3 opt;
  cOSDL_Undo *cundo = new cOSDL_Undo;

  cundo->op = UNDO_PSPS;

  //for each selected tesselated object
  idbase = BASE_TESSOBJ;
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    if (tessobj[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      opt = tessobj[ii].GetPos();
      cundo->sclid.push_back(opt);

      gmVector3 pp, basept;
      //project the (0,0,0) local point to the surface, based on it's global (x,y) location
      pp = tessobj[ii].GetPos();
      //find the closest point
      basept = primary_surface.FindClosest(pp);
      basept[0] = 0.0; basept[1] = 0.0;  basept[2] -= pp[2];

      tessobj[ii].ChangePosition(basept[0],basept[1],basept[2]);
    }
  }

  //for each selected oriented tesselated object
  idbase = BASE_ORIENTTESSOBJ;
  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    if (orienttessobj[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      opt = orienttessobj[ii].GetPos();
      cundo->sclid.push_back(opt);

      gmVector3 pp, basept;
      //project the (0,0,0) local point to the surface, based on it's global (x,y) location
      pp = orienttessobj[ii].GetPos();
      //find the closest point
      basept = primary_surface.FindClosest(pp);
      basept[0] = 0.0; basept[1] = 0.0;  basept[2] -= pp[2];

      orienttessobj[ii].ChangePosition(basept[0],basept[1],basept[2]);
    }
  }

  //for each selected occluder
  idbase = BASE_OCCLUDER;
  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    if (occluder[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      opt = occluder[ii].GetPos();
      cundo->sclid.push_back(opt);

      gmVector3 pp, basept;
      //project the (0,0,0) local point to the surface, based on it's global (x,y) location
      pp = occluder[ii].GetPos();
      //find the closest point
      basept = primary_surface.FindClosest(pp);
      basept[0] = 0.0; basept[1] = 0.0;  basept[2] -= pp[2];

      occluder[ii].ChangePosition(basept[0],basept[1],basept[2]);
    }
  }

  //for each selected oriented occluder
  idbase = BASE_ORIENTOCCLUDER;
  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    if (orientoccluder[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      opt = orientoccluder[ii].GetPos();
      cundo->sclid.push_back(opt);

      gmVector3 pp, basept;
      //project the (0,0,0) local point to the surface, based on it's global (x,y) location
      pp = orientoccluder[ii].GetPos();
      //find the closest point
      basept = primary_surface.FindClosest(pp);
      basept[0] = 0.0; basept[1] = 0.0;  basept[2] -= pp[2];

      orientoccluder[ii].ChangePosition(basept[0],basept[1],basept[2]);
    }
  }

  if (!cundo->objectid.empty()) ulist.push_front(cundo);
}

void cOSDL::ProjectSelectedToMidScene()
{
  long ii, idbase;
  gmVector3 midpoint;
  gmVector3 opt;
  cOSDL_Undo *cundo = new cOSDL_Undo;

  cundo->op = UNDO_PSM;

  midpoint = aabb.GetMid();

  //for each selected tesselated object
  idbase = BASE_TESSOBJ;
  for (ii=0; ii<(long)tessobj.size(); ii++)
  {
    if (tessobj[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      opt = tessobj[ii].GetPos();
      cundo->sclid.push_back(opt);

      gmVector3 pp, basept;
      //project the (0,0,0) local point to the surface, based on it's global (x,y) location
      pp = tessobj[ii].GetPos();
      basept = midpoint - pp;

      tessobj[ii].ChangePosition(basept[0],basept[1],basept[2]);
    }
  }

  //for each selected oriented tesselated object
  idbase = BASE_ORIENTTESSOBJ;
  for (ii=0; ii<(long)orienttessobj.size(); ii++)
  {
    if (orienttessobj[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      opt = orienttessobj[ii].GetPos();
      cundo->sclid.push_back(opt);

      gmVector3 pp, basept;
      //project the (0,0,0) local point to the surface, based on it's global (x,y) location
      pp = orienttessobj[ii].GetPos();
      basept = midpoint - pp;

      orienttessobj[ii].ChangePosition(basept[0],basept[1],basept[2]);
    }
  }

  //for each selected occluder
  idbase = BASE_OCCLUDER;
  for (ii=0; ii<(long)occluder.size(); ii++)
  {
    if (occluder[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      opt = occluder[ii].GetPos();
      cundo->sclid.push_back(opt);

      gmVector3 pp, basept;
      //project the (0,0,0) local point to the surface, based on it's global (x,y) location
      pp = occluder[ii].GetPos();
      basept = midpoint - pp;

      occluder[ii].ChangePosition(basept[0],basept[1],basept[2]);
    }
  }

  //for each selected oriented occluder
  idbase = BASE_ORIENTOCCLUDER;
  for (ii=0; ii<(long)orientoccluder.size(); ii++)
  {
    if (orientoccluder[ii].IsSelected())
    {
      cundo->objectid.push_back(idbase+ii);
      opt = orientoccluder[ii].GetPos();
      cundo->sclid.push_back(opt);

      gmVector3 pp, basept;
      //project the (0,0,0) local point to the surface, based on it's global (x,y) location
      pp = orientoccluder[ii].GetPos();
      basept = midpoint - pp;

      orientoccluder[ii].ChangePosition(basept[0],basept[1],basept[2]);
    }
  }

  if (!cundo->objectid.empty()) ulist.push_front(cundo);
}

//////////////////////////////////////////////////////////////////////
// Import new data
//////////////////////////////////////////////////////////////////////
void cOSDL::ImportPrimarySurface( const char *fname, double posx, double posy, double posz, int basemat )
{
  string nfname(fname);
  cAABB saabb;
  gmVector3 mn;

  primary_surface.MakeLink(AddGeometry(nfname));
  primary_surface.SetUniformScale( 1.0 );
  primary_surface.RotationEuler(0.0, 0.0, 0.0);

  saabb = primary_surface.GetTransformedAABB();
  mn = saabb.GetMin();
  primary_surface.FixPosition(-mn[0], -mn[1], -mn[2]);
  primary_surface.SetPosition( posx, posy, posz );

  primary_surface.SetMatID( basemat );

  primary_surface.SetColor( 0.3, 0.2, 0.01 );
  primary_surface.SetHighLightColor( 0.6, 0.4, 0.02 );

  SetAABB(AOI_SURFACE);
  cleanObjectGeometry( );
}

void cOSDL::ImportSurface( const char *fname )
{
  string nfname(fname);
  cAABB saabb;
  gmVector3 mn;
  long idx;
  cObjectDefinition nobj;

  surfaces.push_back(nobj);
  idx = surfaces.size()-1;
  surfaces[idx].MakeLink(AddGeometry(nfname));
  surfaces[idx].SetUniformScale( 1.0 );
  surfaces[idx].RotationEuler(0.0, 0.0, 0.0);

  saabb = surfaces[idx].GetTransformedAABB();
  mn = saabb.GetMin();
  surfaces[idx].FixPosition(-mn[0], -mn[1], -mn[2]);

  surfaces[idx].SetColor( 0.3, 0.2, 0.01 );
  surfaces[idx].SetHighLightColor( 0.6, 0.4, 0.02 );

  cleanObjectGeometry( );
}

void cOSDL::ImportTesselatedObject( const string &fname, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ )
{
  string nfname(fname);
  cAABB saabb;
  long idx;
  cObjectDefinition nobj;

  printf("Importing %s\n%lf  <%lf,%lf,%lf> %d\n",fname.c_str(), scl, xpos, ypos, zpos, basemat);

  tessobj.push_back(nobj);
  idx = tessobj.size()-1;
  tessobj[idx].MakeLink(AddGeometry(nfname));
  tessobj[idx].SetUniformScale( scl );
  tessobj[idx].SetPosition( xpos, ypos, zpos );
  tessobj[idx].SetMatID( basemat );
  if (rotYtoZ)  tessobj[idx].RotationEuler( 0.0, 0.0, 90.0);
  else tessobj[idx].RotationEuler(0.0, 0.0, 0.0);

  tessobj[idx].SetColor( 0.3, 0.4, 0.3 );
  tessobj[idx].SetHighLightColor( 0.6, 0.8, 0.7 );

  cleanObjectGeometry( );
}

void cOSDL::ImportOrientedTesselatedObject( const string &fname, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ )
{
  string nfname(fname);
  cAABB saabb;
  long idx;
  cObjectDefinition nobj;

  orienttessobj.push_back(nobj);
  idx = orienttessobj.size()-1;
  orienttessobj[idx].MakeLink(AddGeometry(nfname));
  orienttessobj[idx].SetUniformScale( scl );
  orienttessobj[idx].SetPosition( xpos, ypos, zpos );
  orienttessobj[idx].SetMatID( basemat );
  if (rotYtoZ) orienttessobj[idx].RotationEuler(0.0, 0.0, 90.0);
  else orienttessobj[idx].RotationEuler(0.0, 0.0, 0.0);

  orienttessobj[idx].SetColor( 0.3, 0.4, 0.3 );
  orienttessobj[idx].SetHighLightColor( 0.6, 0.8, 0.7 );

  cleanObjectGeometry( );
}


void cOSDL::ImportOccluder( const string &fname, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ )
{
  string nfname(fname);
  cAABB saabb;
  long idx;
  cObjectDefinition nobj;

  occluder.push_back(nobj);
  idx = occluder.size()-1;
  occluder[idx].MakeLink(AddGeometry(nfname));
  occluder[idx].SetUniformScale( scl );
  occluder[idx].SetPosition( xpos, ypos, zpos );
  occluder[idx].SetMatID( basemat );
  if (rotYtoZ) occluder[idx].RotationEuler(0.0, 0.0, 90.0);
  else occluder[idx].RotationEuler(0.0, 0.0, 0.0);

  occluder[idx].SetColor( 0.2, 0.4, 0.2 );
  occluder[idx].SetHighLightColor( 0.5, 0.8, 0.5 );

  cleanObjectGeometry( );
}

void cOSDL::ImportOrientedOccluder( const string &fname, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ )
{
  string nfname(fname);
  cAABB saabb;
  long idx;
  cObjectDefinition nobj;

  orientoccluder.push_back(nobj);
  idx = orientoccluder.size()-1;
  orientoccluder[idx].MakeLink(AddGeometry(nfname));
  orientoccluder[idx].SetUniformScale( scl );
  orientoccluder[idx].SetPosition( xpos, ypos, zpos );
  orientoccluder[idx].SetMatID( basemat );
  if (rotYtoZ) orientoccluder[idx].RotationEuler(0.0, 0.0, 90.0);
  else orientoccluder[idx].RotationEuler(0.0, 0.0, 0.0);

  orientoccluder[idx].SetColor( 0.2, 0.4, 0.2 );
  orientoccluder[idx].SetHighLightColor( 0.5, 0.8, 0.5 );

  cleanObjectGeometry( );
}
// END UNDO TREE

void cOSDL::SetAABB( long aoii )
{
  long ii;
  gmVector3 vmn, vmx;

  aabb.Reset();

  //No Data
  if (IsEmpty() || IsLoading())
  {
    vmn.assign(0.0, 0.0, 0.0);
    vmx.assign(1.0, 1.0, 1.0);
    aabb.Adjust(vmn);
    aabb.Adjust(vmx);
    return;
  }

  if (aoii == AOI_SURFACE)
  {
    aabb = primary_surface.GetTransformedAABB();

    for (ii=0; ii<(long)aoi.size(); ii++)
    {
      vmn = aoi[ii].GetBoundingBox().GetMin();
      vmx = aoi[ii].GetBoundingBox().GetMax();
      aabb.Adjust(vmn);
      aabb.Adjust(vmx);
    }
  }
  else
  if (aoii == AOI_FULL)
  {
    for (ii=0; ii<(long)aoi.size(); ii++)
    {
      vmn = aoi[ii].GetBoundingBox().GetMin();
      vmx = aoi[ii].GetBoundingBox().GetMax();
      aabb.Adjust(vmn);
      aabb.Adjust(vmx);
    }
  }
  else
  if (aoii > AOI_FULL)
  {
    vmn = aoi[aoii].GetBoundingBox().GetMin();
    vmx = aoi[aoii].GetBoundingBox().GetMax();
    aabb.Adjust(vmn);
    aabb.Adjust(vmx);
  }

  if (aabb.GetMin()[0] > aabb.GetMax()[0])
  {
    aabb = primary_surface.GetTransformedAABB();
  }
}

//////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////
void cOSDL::onSelection( long idx, bool bval )
{  SetSelect( idx, bval ); }

//passthrough slots
void cOSDL::getProgressMax( int pmax )
{  emit setProgressMax( pmax ); }

void cOSDL::getProgressVal( int pval )
{  emit setProgressVal( pval ); }

void cOSDL::startingProgress( char *caption )
{  emit startProgress(caption); }

void cOSDL::finishingProgress()
{  emit finishProgress(); }

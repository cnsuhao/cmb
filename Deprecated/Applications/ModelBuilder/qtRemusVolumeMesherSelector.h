//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtRemusVolumeMesherSelector
// .SECTION Description
// Simply displays all the different volume meshers that are available to the
// user.
// .SECTION Caveats

#ifndef __qtRemusVolumeMesherSelector_h
#define __qtRemusVolumeMesherSelector_h

#include <QDialog>

//Don't let QMOC see remus headers that include boost headers
//or bad things happen
#ifndef Q_MOC_RUN
  #include <remus/client/ServerConnection.h>
  #include <remus/proto/JobRequirements.h>
#endif

class QListWidgetItem;

class qtRemusVolumeMesherSelector : public QDialog
{
  Q_OBJECT
public:
  qtRemusVolumeMesherSelector(QString serverEndpoint,
                           QWidget* parent);

  //returns true if the user has selected a mesher
  bool chooseMesher();

  bool useLegacyMesher() const;

  QString mesherName() const { return ActiveMesher; }
  const remus::proto::JobRequirements& mesherData() const { return ActiveMesherData; }

public slots:
  void mesherChanged(QListWidgetItem* item);

private:
  QString ActiveMesher;
  remus::proto::JobRequirements ActiveMesherData;
  remus::client::ServerConnection Connection;

};

#endif

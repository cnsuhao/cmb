/*=========================================================================

  Program:   CMB
  Module:    qtRemusMesherSelector.h

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME qtRemusMesherSelector
// .SECTION Description
// Simply displays all the different meshers that are available to the
// user, as a combo box widget
// .SECTION Caveats

#ifndef __qtRemusMesherSelector_h
#define __qtRemusMesherSelector_h

#include <QWidget>

#ifndef Q_MOC_RUN
# include <remus/client/ServerConnection.h>
# include <remus/proto/JobRequirements.h>
#endif

class QListWidgetItem;

class qtRemusMesherSelector : public QWidget
{
  Q_OBJECT
public:
  qtRemusMesherSelector(
                        const remus::client::ServerConnection& connection,
                        QWidget* parent);

  ~qtRemusMesherSelector();

  int currentIndex() const;
  QString currentMesherName() const;
  remus::proto::JobRequirements currentMesherRequirements() const;

signals:
  void currentMesherChanged(const QString & name, const remus::proto::JobRequirements& reqs);

protected slots:
  void mesherChanged( int index );

private:
  class pqInternal;
  pqInternal* Internal;
};

#endif

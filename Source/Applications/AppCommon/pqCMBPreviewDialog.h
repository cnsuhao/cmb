/*=========================================================================

  Program:   CMB
  Module:    pqCMBPreviewDialog.h

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
// .NAME pqCMBPreviewDialog - CMB preview dialog object.
// .SECTION Description
//  This class is the preview dialog widget for the CMB Application
// .SECTION Caveats

#ifndef __pqCMBPreviewDialog_h
#define __pqCMBPreviewDialog_h

#include "cmbAppCommonExport.h"
#include <QDialog>
#include "cmbSystemConfig.h"

namespace Ui { class qtPreviewDialog; }

class pqDataRepresentation;
class pqRenderView;
class pqCMBEnumPropertyWidget;

class CMBAPPCOMMON_EXPORT pqCMBPreviewDialog : public QDialog
{
  Q_OBJECT

public:
  pqCMBPreviewDialog(QWidget* Parent);
  ~pqCMBPreviewDialog();

  // Description:
  // Set teh representation and view for the preview dialog
  void setRepresentationAndView(
   pqDataRepresentation* dataRep, pqRenderView* renderView);

  // Description:
  // Sets the preview in "error mode" where this is no accept and cancel
  // becomes Close
  void enableErrorView(bool state);

private:
  pqCMBPreviewDialog(const pqCMBPreviewDialog&); // Not implemented.
  void operator=(const pqCMBPreviewDialog&); // Not implemented.

  // Description:
  // ivars
  Ui::qtPreviewDialog* const Ui;
  pqCMBEnumPropertyWidget* RepresentationWidget;
};

#endif

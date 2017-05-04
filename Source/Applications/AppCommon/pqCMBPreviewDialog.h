//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBPreviewDialog - CMB preview dialog object.
// .SECTION Description
//  This class is the preview dialog widget for the CMB Application
// .SECTION Caveats

#ifndef __pqCMBPreviewDialog_h
#define __pqCMBPreviewDialog_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <QDialog>

namespace Ui
{
class qtPreviewDialog;
}

class pqDataRepresentation;
class pqRenderView;
class pqCMBEnumPropertyWidget;

class CMBAPPCOMMON_EXPORT pqCMBPreviewDialog : public QDialog
{
  Q_OBJECT

public:
  pqCMBPreviewDialog(QWidget* Parent);
  ~pqCMBPreviewDialog() override;

  // Description:
  // Set teh representation and view for the preview dialog
  void setRepresentationAndView(pqDataRepresentation* dataRep, pqRenderView* renderView);

  // Description:
  // Sets the preview in "error mode" where this is no accept and cancel
  // becomes Close
  void enableErrorView(bool state);

private:
  pqCMBPreviewDialog(const pqCMBPreviewDialog&); // Not implemented.
  void operator=(const pqCMBPreviewDialog&);     // Not implemented.

  // Description:
  // ivars
  Ui::qtPreviewDialog* const Ui;
  pqCMBEnumPropertyWidget* RepresentationWidget;
};

#endif

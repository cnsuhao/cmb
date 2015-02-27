// .NAME pqCMBColorMapDialog - a CMB color map Dialog object.
// .SECTION Description
//  This class provides a color editor dialog for the representation
// .SECTION Caveats

#ifndef _pqCMBColorMapDialog_h
#define _pqCMBColorMapDialog_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include "cmbSystemConfig.h"

class pqDataRepresentation;
class QDialog;
class pqProxyWidget;

class CMBAPPCOMMON_EXPORT pqCMBColorMapDialog : public QObject
{
  Q_OBJECT

public:
  pqCMBColorMapDialog(pqDataRepresentation* display, QWidget* p = NULL);
  virtual ~pqCMBColorMapDialog();

public:
  int exec();

protected slots:
  void accept();

protected:
  int Status;
  QDialog *MainDialog;
  pqProxyWidget* ColorEditor;

};

#endif

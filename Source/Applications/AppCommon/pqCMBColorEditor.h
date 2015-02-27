// .NAME qtCMBPanelsManager -
// .SECTION Description

#ifndef __CmbColorMapEditor_h
#define __CmbColorMapEditor_h

#include <QWidget>
#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"

class vtkSMProxy;
class pqDataRepresentation;

/// pqCMBColorEditor is a widget that can be used to edit the active color-map,
/// if any. The panel is implemented as an auto-generated panel (similar to the
/// Properties panel) that shows the properties on the lookup-table proxy.
class CMBAPPCOMMON_EXPORT pqCMBColorEditor : public QWidget
{
  Q_OBJECT
  typedef QWidget Superclass;
public:
  pqCMBColorEditor(QWidget* parent=0);
  virtual ~pqCMBColorEditor();

public:
  void setDataRepresentation(pqDataRepresentation* repr);

protected slots:
  /// slot called to update the currently showing proxies.
  void updateRepresentation();

  /// slot called to update the visible widgets.
  void updatePanel();

  /// render's view when transfer function is modified.
  void renderViews();

protected:
  void setColorTransferFunction(vtkSMProxy* ctf);

private:
  Q_DISABLE_COPY(pqCMBColorEditor)
  class pqInternals;
  pqInternals* Internals;
};

#endif

// .NAME cmbScalarBarWidget - a CMB color map Dialog object.
// .SECTION Description
//  This class provides a color editor dialog for the representation
// .SECTION Caveats

#ifndef _cmbScalarBarWidget_h
#define _cmbScalarBarWidget_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QColor>
#include "cmbSystemConfig.h"

class pqDataRepresentation;
class QDialog;

class CMBAPPCOMMON_EXPORT cmbScalarBarWidget : public QObject
{
  Q_OBJECT

public:
  cmbScalarBarWidget(pqDataRepresentation* display, QWidget* p = NULL);
  virtual ~cmbScalarBarWidget();

public:

  void setIndexedColors( const QList<QColor>& colors );
  void setAnnotations(
    const QList<QVariant>& annotations );
  void setVisible(bool visible);

  void setTitle(const QString& title);
  void setPositionToLeft();
  void setPositionToRight();
protected:
  void init();

private:
  class cmbInternals;
  cmbInternals* Internals;
};

#endif

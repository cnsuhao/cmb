// .NAME qtScalarBarWidget - a CMB color map Dialog object.
// .SECTION Description
//  This class provides a color editor dialog for the representation
// .SECTION Caveats

#ifndef _qtScalarBarWidget_h
#define _qtScalarBarWidget_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QColor>
#include "cmbSystemConfig.h"

class pqDataRepresentation;
class QDialog;

class CMBAPPCOMMON_EXPORT qtScalarBarWidget : public QObject
{
  Q_OBJECT

public:
  qtScalarBarWidget(pqDataRepresentation* display, QWidget* p = NULL);
  virtual ~qtScalarBarWidget();

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

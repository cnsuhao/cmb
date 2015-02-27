#ifndef __pqCMBImportShapefile_h
#define __pqCMBImportShapefile_h

#include <QtGui/QDialog>
#include "cmbSystemConfig.h"

class pqServer;

/// options container for pages of model builder and sim builder options
class pqCMBImportShapefile : public QDialog
{
  Q_OBJECT

public:
  pqCMBImportShapefile(pqServer* activeServer, QWidget* parent = 0, Qt::WindowFlags f = 0);
  virtual ~pqCMBImportShapefile();

  int boundaryStyle();
  int marginStyle();
  QString marginSpecification();
  QString customBoundaryFilename();

protected slots:
  virtual void chooseCustomBoundaryFile();
  virtual void setCustomBoundaryFile(const QString&);

protected:
  class pqInternal;
  pqInternal* Internal;
};

#endif // __pqCMBImportShapefile_h

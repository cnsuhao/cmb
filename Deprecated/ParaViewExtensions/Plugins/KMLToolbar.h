#ifndef __KMLToolbar_h
#define __KMLToolbar_h

#include <QActionGroup>

class KMLToolbar : public QActionGroup
{
  Q_OBJECT
public:
  KMLToolbar(QObject* p);

public slots:

  // Callback for each action triggerred.
  void onAction(QAction* a);
};

#endif // __KMLToolbar_h


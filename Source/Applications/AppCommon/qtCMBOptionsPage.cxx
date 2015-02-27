#include "qtCMBOptionsPage.h"


qtCMBOptionsPage::qtCMBOptionsPage(QWidget *widgetParent)
  : QWidget(widgetParent)
{
}

void qtCMBOptionsPage::sendChangesAvailable()
{
  emit this->changesAvailable();
}

#include "qtCMBOptionsContainer.h"


qtCMBOptionsContainer::qtCMBOptionsContainer(QWidget *widgetParent)
  : qtCMBOptionsPage(widgetParent)
{
  this->Prefix = new QString();
}

qtCMBOptionsContainer::~qtCMBOptionsContainer()
{
  delete this->Prefix;
}

const QString &qtCMBOptionsContainer::getPagePrefix() const
{
  return *this->Prefix;
}

void qtCMBOptionsContainer::setPagePrefix(const QString &prefix)
{
  *this->Prefix = prefix;
}

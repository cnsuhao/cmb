#include "qtCMBOptionsDialog.h"
#include "ui_qtCMBOptionsDialog.h"

#include "qtCMBOptionsContainer.h"
#include "qtCMBOptionsPage.h"
#include "pqUndoStack.h"

#include <QAbstractItemModel>
#include <QHeaderView>
#include <QList>
#include <QMap>
#include <QString>


class qtCMBOptionsDialogModelItem
{
public:
  qtCMBOptionsDialogModelItem();
  qtCMBOptionsDialogModelItem(const QString &name);
  ~qtCMBOptionsDialogModelItem();

  qtCMBOptionsDialogModelItem *Parent;
  QString Name;
  QList<qtCMBOptionsDialogModelItem *> Children;
};


class qtCMBOptionsDialogModel : public QAbstractItemModel
{
public:
  qtCMBOptionsDialogModel(QObject *parent=0);
  virtual ~qtCMBOptionsDialogModel();

  virtual int rowCount(const QModelIndex &parent=QModelIndex()) const;
  virtual int columnCount(const QModelIndex &parent=QModelIndex()) const;
  virtual QModelIndex index(int row, int column,
      const QModelIndex &parent=QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex &child) const;

  virtual QVariant data(const QModelIndex &index,
      int role=Qt::DisplayRole) const;

  QModelIndex getIndex(const QString &path) const;
  QString getPath(const QModelIndex &index) const;
  void addPath(const QString &path);
  bool removeIndex(const QModelIndex &index);

private:
  QModelIndex getIndex(qtCMBOptionsDialogModelItem *item) const;

private:
  qtCMBOptionsDialogModelItem *Root;
};


class qtCMBOptionsDialogForm : public Ui::qtCMBOptionsFrame
{
public:
  qtCMBOptionsDialogForm();
  ~qtCMBOptionsDialogForm();

  QMap<QString, qtCMBOptionsPage *> Pages;
  qtCMBOptionsDialogModel *Model;
  int ApplyUseCount;
  bool ApplyNeeded;
};


//----------------------------------------------------------------------------
qtCMBOptionsDialogModelItem::qtCMBOptionsDialogModelItem()
  : Name(), Children()
{
  this->Parent = 0;
}

qtCMBOptionsDialogModelItem::qtCMBOptionsDialogModelItem(const QString &name)
  : Name(name), Children()
{
  this->Parent = 0;
}

qtCMBOptionsDialogModelItem::~qtCMBOptionsDialogModelItem()
{
  QList<qtCMBOptionsDialogModelItem *>::Iterator iter = this->Children.begin();
  for( ; iter != this->Children.end(); ++iter)
    {
    delete *iter;
    }
}


//----------------------------------------------------------------------------
qtCMBOptionsDialogModel::qtCMBOptionsDialogModel(QObject *parentObject)
  : QAbstractItemModel(parentObject)
{
  this->Root = new qtCMBOptionsDialogModelItem();
}

qtCMBOptionsDialogModel::~qtCMBOptionsDialogModel()
{
  delete this->Root;
}

int qtCMBOptionsDialogModel::rowCount(const QModelIndex &parentIndex) const
{
  qtCMBOptionsDialogModelItem *item = this->Root;
  if(parentIndex.isValid())
    {
    item = reinterpret_cast<qtCMBOptionsDialogModelItem *>(
        parentIndex.internalPointer());
    }

  return item->Children.size();
}

int qtCMBOptionsDialogModel::columnCount(const QModelIndex &) const
{
  return 1;
}

QModelIndex qtCMBOptionsDialogModel::index(int row, int column,
    const QModelIndex &parentIndex) const
{
  qtCMBOptionsDialogModelItem *item = this->Root;
  if(parentIndex.isValid())
    {
    item = reinterpret_cast<qtCMBOptionsDialogModelItem *>(
        parentIndex.internalPointer());
    }

  if(column == 0 && row >= 0 && row < item->Children.size())
    {
    return this->createIndex(row, column, item->Children[row]);
    }

  return QModelIndex();
}

QModelIndex qtCMBOptionsDialogModel::parent(const QModelIndex &child) const
{
  if(child.isValid())
    {
    qtCMBOptionsDialogModelItem *item =
        reinterpret_cast<qtCMBOptionsDialogModelItem *>(child.internalPointer());
    return this->getIndex(item->Parent);
    }

  return QModelIndex();
}

QVariant qtCMBOptionsDialogModel::data(const QModelIndex &idx, int role) const
{
  if(idx.isValid())
    {
    qtCMBOptionsDialogModelItem *item =
        reinterpret_cast<qtCMBOptionsDialogModelItem *>(idx.internalPointer());
    if(role == Qt::DisplayRole || role == Qt::ToolTipRole)
      {
      return QVariant(item->Name);
      }
    }

  return QVariant();
}

QModelIndex qtCMBOptionsDialogModel::getIndex(const QString &path) const
{
  qtCMBOptionsDialogModelItem *item = this->Root;
  QStringList names = path.split(".");
  QStringList::Iterator iter = names.begin();
  for( ; item && iter != names.end(); ++iter)
    {
    qtCMBOptionsDialogModelItem *child = 0;
    QList<qtCMBOptionsDialogModelItem *>::Iterator jter = item->Children.begin();
    for( ; jter != item->Children.end(); ++jter)
      {
      if((*jter)->Name == *iter)
        {
        child = *jter;
        break;
        }
      }

    item = child;
    }

  if(item && item != this->Root)
    {
    return this->getIndex(item);
    }

  return QModelIndex();
}

QString qtCMBOptionsDialogModel::getPath(const QModelIndex &idx) const
{
  if(idx.isValid())
    {
    QString path;
    qtCMBOptionsDialogModelItem *item =
        reinterpret_cast<qtCMBOptionsDialogModelItem *>(idx.internalPointer());
    if(item)
      {
      path = item->Name;
      item = item->Parent;
      }

    while(item && item != this->Root)
      {
      path.prepend(".").prepend(item->Name);
      item = item->Parent;
      }

    return path;
    }

  return QString();
}

void qtCMBOptionsDialogModel::addPath(const QString &path)
{
  qtCMBOptionsDialogModelItem *item = this->Root;
  QStringList names = path.split(".");
  QStringList::Iterator iter = names.begin();
  for( ; iter != names.end(); ++iter)
    {
    qtCMBOptionsDialogModelItem *child = 0;
    QList<qtCMBOptionsDialogModelItem *>::Iterator jter = item->Children.begin();
    for( ; jter != item->Children.end(); ++jter)
      {
      if((*jter)->Name == *iter)
        {
        child = *jter;
        break;
        }
      }

    if(!child)
      {
      child = new qtCMBOptionsDialogModelItem(*iter);
      child->Parent = item;
      QModelIndex parentIndex = this->getIndex(item);
      int row = item->Children.size();
      this->beginInsertRows(parentIndex, row, row);
      item->Children.append(child);
      this->endInsertRows();
      }

    item = child;
    }
}

bool qtCMBOptionsDialogModel::removeIndex(const QModelIndex &idx)
{
  if(idx.isValid())
    {
    qtCMBOptionsDialogModelItem *item =
        reinterpret_cast<qtCMBOptionsDialogModelItem *>(idx.internalPointer());
    if(item->Children.size() == 0)
      {
      QModelIndex parentIndex = this->getIndex(item->Parent);
      this->beginRemoveRows(parentIndex, idx.row(), idx.row());
      item->Parent->Children.removeAt(idx.row());
      this->endRemoveRows();
      delete item;
      return true;
      }
    }

  return false;
}

QModelIndex qtCMBOptionsDialogModel::getIndex(
    qtCMBOptionsDialogModelItem *item) const
{
  if(item && item->Parent)
    {
    return this->createIndex(item->Parent->Children.indexOf(item), 0, item);
    }

  return QModelIndex();
}


//----------------------------------------------------------------------------
qtCMBOptionsDialogForm::qtCMBOptionsDialogForm()
  : Ui::qtCMBOptionsFrame(), Pages()
{
  this->Model = new qtCMBOptionsDialogModel();
  this->ApplyUseCount = 0;
  this->ApplyNeeded = false;
}

qtCMBOptionsDialogForm::~qtCMBOptionsDialogForm()
{
  delete this->Model;
}


//----------------------------------------------------------------------------
qtCMBOptionsDialog::qtCMBOptionsDialog(QWidget *widgetParent)
  : QDialog(widgetParent)
{
  this->Form = new qtCMBOptionsDialogForm();
  this->Form->setupUi(this);
  this->Form->PageNames->setModel(this->Form->Model);

  // Hide the tree widget header view.
  this->Form->PageNames->header()->hide();

  // Hide the apply and reset buttons until they are needed.
  this->Form->ApplyButton->setEnabled(false);
  this->Form->ResetButton->setEnabled(false);
  this->Form->ApplyButton->hide();
  this->Form->ResetButton->hide();

  this->connect(this->Form->PageNames->selectionModel(),
      SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
      this, SLOT(changeCurrentPage()));
  this->connect(this->Form->ApplyButton, SIGNAL(clicked()),
      this, SLOT(applyChanges()));
  this->connect(this->Form->ResetButton, SIGNAL(clicked()),
      this, SLOT(resetChanges()));
  this->connect(this->Form->CloseButton, SIGNAL(clicked()),
      this, SLOT(accept()));
  this->connect(this->Form->RestoreDefaults, SIGNAL(clicked()),
      this, SLOT(restoreDefaults()));

}

qtCMBOptionsDialog::~qtCMBOptionsDialog()
{
  delete this->Form;
}

void qtCMBOptionsDialog::accept()
{
  this->applyChanges();
  this->QDialog::accept();
}

bool qtCMBOptionsDialog::isApplyNeeded() const
{
  return this->Form->ApplyNeeded;
}

void qtCMBOptionsDialog::setApplyNeeded(bool applyNeeded)
{
  if(applyNeeded != this->Form->ApplyNeeded)
    {
    if(!applyNeeded)
      {
      this->Form->ApplyNeeded = false;
      this->Form->ApplyButton->setEnabled(false);
      this->Form->ResetButton->setEnabled(false);
      }
    else if(this->Form->ApplyUseCount > 0)
      {
      this->Form->ApplyNeeded = true;
      this->Form->ApplyButton->setEnabled(true);
      this->Form->ResetButton->setEnabled(true);
      }
    }
}

void qtCMBOptionsDialog::addOptions(const QString &path, qtCMBOptionsPage *options)
{
  if(!options)
    {
    return;
    }

  // See if the page is a container.
  qtCMBOptionsContainer *container = qobject_cast<qtCMBOptionsContainer *>(options);
  if(!container && path.isEmpty())
    {
    return;
    }

  // See if the page/container uses the apply button.
  if(options->isApplyUsed())
    {
    this->Form->ApplyUseCount++;
    if(this->Form->ApplyUseCount == 1)
      {
      this->Form->ApplyButton->show();
      this->Form->ResetButton->show();
      QObject::connect(this, SIGNAL(accepted()), this, SLOT(applyChanges()));
      }

    this->connect(options, SIGNAL(changesAvailable()),
        this, SLOT(enableButtons()));
    }

  // Add the widget to the stack.
  this->Form->Stack->addWidget(options);

  // Add the page(s) to the map and the model.
  if(container)
    {
    // If the path is not empty, use it as the page prefix.
    QString prefix;
    if(!path.isEmpty())
      {
      prefix = path;
      prefix.append(".");
      }

    container->setPagePrefix(prefix);

    // Get the list of pages from the container.
    QStringList pathList = container->getPageList();
    QStringList::Iterator iter = pathList.begin();
    for( ; iter != pathList.end(); ++iter)
      {
      this->Form->Pages.insert(prefix + *iter, options);
      this->Form->Model->addPath(prefix + *iter);
      }
    }
  else
    {
    this->Form->Pages.insert(path, options);
    this->Form->Model->addPath(path);
    }

  this->Form->PageNames->expandAll();
}

void qtCMBOptionsDialog::addOptions(qtCMBOptionsContainer *options)
{
  this->addOptions(QString(), options);
}

void qtCMBOptionsDialog::removeOptions(qtCMBOptionsPage *options)
{
  if(!options)
    {
    return;
    }

  // Remove the widget from the stack.
  this->Form->Stack->removeWidget(options);

  // See if the options use the apply button.
  if(options->isApplyUsed())
    {
    this->Form->ApplyUseCount--;
    if(this->Form->ApplyUseCount == 0)
      {
      this->Form->ApplyNeeded = false;
      this->Form->ApplyButton->setEnabled(false);
      this->Form->ResetButton->setEnabled(false);
      this->Form->ApplyButton->hide();
      this->Form->ResetButton->hide();
      QObject::disconnect(this, SIGNAL(accepted()), this, SLOT(applyChanges()));
      }

    this->disconnect(options, 0, this, 0);
    }

  // Search the map for the paths to remove.
  QMap<QString, qtCMBOptionsPage *>::Iterator iter = this->Form->Pages.begin();
  while(iter != this->Form->Pages.end())
    {
    if(*iter == options)
      {
      QString path = iter.key();
      iter = this->Form->Pages.erase(iter);

      // Remove the item from the tree model as well.
      QModelIndex index = this->Form->Model->getIndex(path);
      QPersistentModelIndex parentIndex = index.parent();
      if(this->Form->Model->removeIndex(index))
        {
        // Remove any empty parent items.
        while(parentIndex.isValid() &&
            this->Form->Model->rowCount(parentIndex) == 0)
          {
          index = parentIndex;
          parentIndex = index.parent();

          // Make sure the index path isn't in the map.
          path = this->Form->Model->getPath(index);
          if(this->Form->Pages.find(path) == this->Form->Pages.end())
            {
            if(!this->Form->Model->removeIndex(index))
              {
              break;
              }
            }
          }
        }
      }
    else
      {
      ++iter;
      }
    }
}

void qtCMBOptionsDialog::setCurrentPage(const QString &path)
{
  QModelIndex current = this->Form->Model->getIndex(path);
  this->Form->PageNames->setCurrentIndex(current);
}

void qtCMBOptionsDialog::applyChanges()
{
  if(this->Form->ApplyNeeded)
    {
    BEGIN_UNDO_SET("Changed View Settings");
    emit this->aboutToApplyChanges();
    QMap<QString, qtCMBOptionsPage *>::Iterator iter = this->Form->Pages.begin();
    for( ; iter != this->Form->Pages.end(); ++iter)
      {
      (*iter)->applyChanges();
      }

    this->setApplyNeeded(false);
    emit this->appliedChanges();
    END_UNDO_SET();
    }
}

void qtCMBOptionsDialog::resetChanges()
{
  if(this->Form->ApplyNeeded)
    {
    QMap<QString, qtCMBOptionsPage *>::Iterator iter = this->Form->Pages.begin();
    for( ; iter != this->Form->Pages.end(); ++iter)
      {
      (*iter)->resetChanges();
      }

    this->setApplyNeeded(false);
    }
}

void qtCMBOptionsDialog::restoreDefaults()
{
  QMap<QString, qtCMBOptionsPage *>::Iterator iter = this->Form->Pages.begin();
  for( ; iter != this->Form->Pages.end(); ++iter)
    {
    (*iter)->restoreDefaults();
    }

  this->setApplyNeeded(true);
}

void qtCMBOptionsDialog::changeCurrentPage()
{
  // Get the current index from the view.
  QModelIndex current = this->Form->PageNames->currentIndex();

  // Look up the path for the current index.
  QString path = this->Form->Model->getPath(current);
  QMap<QString, qtCMBOptionsPage *>::Iterator iter = this->Form->Pages.find(path);
  if(iter == this->Form->Pages.end())
    {
    // If no page is found, show the blank page.
    this->Form->Stack->setCurrentWidget(this->Form->BlankPage);
    }
  else
    {
    this->Form->Stack->setCurrentWidget(*iter);
    qtCMBOptionsContainer *container = qobject_cast<qtCMBOptionsContainer *>(*iter);
    if(container)
      {
      // Get the path prefix from the container.
      QString prefix = container->getPagePrefix();
      if(!prefix.isEmpty())
        {
        // Remove the prefix from the path.
        path.remove(0, prefix.length());
        }

      // Set the page on the container object.
      container->setPage(path);
      }
    }
}

void qtCMBOptionsDialog::enableButtons()
{
  this->setApplyNeeded(true);
}

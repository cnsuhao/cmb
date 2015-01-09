#ifndef _qtCMBOptionsDialog_h
#define _qtCMBOptionsDialog_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <QDialog>

class qtCMBOptionsContainer;
class qtCMBOptionsDialogForm;
class qtCMBOptionsPage;
class QString;


/// \class qtCMBOptionsDialog
/// \brief
///   The qtCMBOptionsDialog class is a generic options dialog.
///
/// Pages can be added to the dialog using the qtCMBOptionsPage and
/// qtCMBOptionsContainer interfaces. The options dialog has apply and
/// reset buttons that the pages can use.
class CMBAPPCOMMON_EXPORT qtCMBOptionsDialog : public QDialog
{
  Q_OBJECT

public:
  qtCMBOptionsDialog(QWidget *parent=0);
  virtual ~qtCMBOptionsDialog();

  /// \brief
  ///   Gets whether or not there are changes to apply.
  /// \return
  ///   True if there are changes to apply.
  bool isApplyNeeded() const;

  /// \brief
  ///   Sets whether or not there are changes to apply.
  /// \param applyNeeded True if there are changes to apply.
  void setApplyNeeded(bool applyNeeded);

  /// \brief
  ///   Adds a page to the options dialog.
  ///
  /// When the options object is a page container, the path parameter
  /// becomes the path prefix for the container pages.
  ///
  /// \param path The name hierarchy for the options page.
  /// \param options The options page.
  void addOptions(const QString &path, qtCMBOptionsPage *options);

  /// \brief
  ///   Adds a container to the options dialog.
  ///
  /// Each page listed for the container is added to the root of the
  /// selection tree.
  ///
  /// \param options The options container to add.
  void addOptions(qtCMBOptionsContainer *options);

  /// \brief
  ///   Removes the options page from the options dialog.
  ///
  /// The page name is removed from the selection tree. If the page
  /// is an options container, all the names are removed.
  ///
  /// \param options The options page/container to remove.
  void removeOptions(qtCMBOptionsPage *options);

public slots:
  /// when OK button is clicked
  virtual void accept();

  /// \brief
  ///   Sets the current options page.
  /// \param path The name of the options page to show.
  void setCurrentPage(const QString &path);

  /// Calls each page to apply any changes.
  void applyChanges();

  /// Calls each page to reset any changes.
  void resetChanges();

  void restoreDefaults();

signals:
  /// Emitted before the option changes are applied.
  void aboutToApplyChanges();

  /// Emitted after the option changes have been applied.
  void appliedChanges();

private slots:
  /// Changes the current page to match the user selection.
  void changeCurrentPage();

  /// Enabled the apply and reset buttons.
  void enableButtons();

private:
  qtCMBOptionsDialogForm *Form; /// Stores the form and class data.
};

#endif

#ifndef _qtCMBOptionsPage_h
#define _qtCMBOptionsPage_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <QWidget>


/// \class qtCMBOptionsPage
/// \brief
///   The qtCMBOptionsPage class is used to add a single page of options
///   to the qtCMBOptionsDialog.
class CMBAPPCOMMON_EXPORT qtCMBOptionsPage : public QWidget
{
  Q_OBJECT

public:
  /// \brief
  ///   Constructs an options page.
  /// \param parent The parent widget.
  qtCMBOptionsPage(QWidget *parent=0);
  virtual ~qtCMBOptionsPage() {}

  /// \brief
  ///   Gets whether or not the apply button is used by the options.
  /// \return
  ///   True if the apply button is used by the options.
  virtual bool isApplyUsed() const {return false; }

  /// Sends a signal that changes are available to apply.
  void sendChangesAvailable();

public slots:
  /// \brief
  ///   Applies changes to the options data.
  ///
  /// The apply handler is used to save the changes. Sub-classes can
  /// override this method to save the changes directly instead of
  /// using an apply handler.
  virtual void applyChanges()=0;

  /// \brief
  ///   Resets the changes to the options data.
  /// \sa qtCMBOptionsPage::applyChanges()
  virtual void resetChanges()=0;

signals:
  /// Emitted when there are changes to be applied.
  void changesAvailable();

private:
};

#endif

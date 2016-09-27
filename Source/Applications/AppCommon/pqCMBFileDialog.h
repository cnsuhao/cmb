//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#ifndef _pqCMBFileDialog_h
#define _pqCMBFileDialog_h

#include "cmbAppCommonExport.h"
#include <QStringList>
#include <QDialog>

class QModelIndex;
class QPoint;
class pqServer;
class QShowEvent;

/**
  Provides a standard file dialog "front-end" for the pqFileDialogModel
  "back-end", i.e. it can be used for both local and remote file browsing.

  pqCMBFileDialog can be used in both "modal" and "non-modal" operations.
  For "non-modal" operation, create an instance of pqCMBFileDialog on the heap,
  set the Qt::WA_DeleteOnClose flag, connect to the fileSelected() signal,
  and show the dialog.  The dialog will be automatically destroyed when the
  user completes their file selection, and your slot will be called with
  the files the user selected:

  /code
  pqCMBFileDialog* dialog = new pqCMBFileDialog(NULL, this);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  QObject::connect(
    dialog,
    SIGNAL(filesSelected(const QList<QStringList>&)),
    this,
    SLOT(onOpenSessionFile(const QList<QStringList>&)));

  dialog->show();
  /endcode

  For "modal" operation, create an instance of pqCMBFileDialog on the stack,
  call its exec() method, and retrieve the user's file selection with the
  getSelectedFiles() method:

  /code
  pqCMBFileDialog dialog(NULL, this);
  if(Qt::Accepted == dialog.exec())
    {
    //each string list holds a list of files that represent a file-series
    QList<QStringList> files = dialog.getAllSelectedFiles();
    }
  /endcode

  \sa pqFileDialogModel
*/

class CMBAPPCOMMON_EXPORT pqCMBFileDialog :
  public QDialog
{
  typedef QDialog Superclass;
  Q_OBJECT
public:

  /// choose mode for selecting file/folder.
  /// AnyFile: The name of a file, whether it exists or not.
  ///   Typically used by "Save As..."
  /// ExistingFile: The name of a single existing file.
  ///   Typically used by "Open..."
  ///   This mode allows the user to select a single file, or one time series group of files.
  /// ExistingFiles: The names of zero or more existing files.
  ///   Typically used by "Open..." when you want multiple file selection
  ///   This mode allows the user to select multiples files, and multiple time series groups at the same time.
  /// Directory: The name of a directory.
  enum FileMode { AnyFile, ExistingFile, ExistingFiles, Directory };

  /// Creates a file dialog with the specified server
  /// if the server is NULL, files are browsed locally
  /// the title, and start directory may be specified
  /// the filter is a string of semi-colon separated filters
  pqCMBFileDialog(pqServer*,
    QWidget* Parent,
    const QString& Title = QString(),
    const QString& Directory = QString(),
    const QString& Filter = QString());
  ~pqCMBFileDialog() override;

  /// set the file mode
  void setFileMode(FileMode);

  /// set the most recently used file extension
  void setRecentlyUsedExtension(const QString& fileExtension);

  /// Returns the group of files for the given index
  QStringList getSelectedFiles(int index=0);

  /// Returns all the file groups
  QList<QStringList> getAllSelectedFiles();

  /// accept this dialog
  void accept() override;

  /// set a file current to support test playback
  bool selectFile(const QString&);

  /// set if we show hidden files and holders
  void setShowHidden( const bool& hidden);

  ///returns the state of the show hidden flag
  bool getShowHidden();

  /// static method similar to QFileDialog::getSaveFileName(...) to make it
  /// easier to get a file name to save a file as.
  static QString getSaveFileName(
    pqServer* server, QWidget* parentWdg,
    const QString& title = QString(),
    const QString& directory = QString(),
    const QString& filter = QString());
public slots:
  void setMetadata(QStringList const& meta);
signals:
  /// Signal emitted when the user has chosen a set of files
  void filesSelected(const QList<QStringList> &);

  /// Signal emitted when the user has chosen a set of files
  /// NOTE:
  /// The mode has to be not ExistingFiles for this signal to be emitted!
  /// This signal is deprecated and should not be used anymore. Instead
  /// use the fileSelected(const QList<QStringList> &)
  void filesSelected(const QStringList &);

  /// signal emitted when user has chosen a set of files and accepted the
  /// dialog.  This signal includes only the path and file string as is
  /// This is to support test recording
  void fileAccepted(const QString&);

  void currentSelectedFilesChanged(const QStringList& );

protected:
  bool acceptExistingFiles();
  bool acceptDefault(const bool &checkForGrouping);

  QStringList buildFileGroup(const QString &filename);

  void showEvent( QShowEvent *showEvent ) override;

private slots:
  void onModelReset();
  void onNavigate(const QString&);
  void onNavigateUp();
  void onNavigateBack();
  void onNavigateForward();
  void onNavigateDown(const QModelIndex&);
  void onFilterChange(const QString&);

  void onClickedRecent(const QModelIndex&);
  void onClickedFavorite(const QModelIndex&);
  void onClickedFile(const QModelIndex&);

  void onActivateFavorite(const QModelIndex&);
  void onActivateRecent(const QModelIndex&);
  void onDoubleClickFile( const QModelIndex& );

  void onTextEdited(const QString&);

  void onShowHiddenFiles( const bool &hide );

  // Called when the user changes the file selection.
  void fileSelectionChanged();

  // Called when the user right-clicks in the file qtreeview
  void onContextMenuRequested(const QPoint &pos);

  // Called when the user requests to create a new directory in the cwd
  void onCreateNewFolder();

  /// Adds this grouping of files to the files selected list
  void addToFilesSelected(const QStringList&);

  /// Emits the filesSelected() signal and closes the dialog,
  void emitFilesSelectionDone();

private:
  pqCMBFileDialog(const pqCMBFileDialog&);
  pqCMBFileDialog& operator=(const pqCMBFileDialog&);

  class pqImplementation;
  pqImplementation* const Implementation;

  //returns if true if files are loaded
  bool acceptInternal(const QStringList& selected_files, const bool &doubleclicked);
  QString fixFileExtension(const QString& filename, const QString& filter);
};

#endif // !_pqCMBFileDialog_h


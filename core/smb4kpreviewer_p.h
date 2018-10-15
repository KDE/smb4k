/***************************************************************************
    Private helper classes for Smb4KPreviewer class
                             -------------------
    begin                : So Dez 21 2008
    copyright            : (C) 2008-2017 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KPREVIEWER_P_H
#define SMB4KPREVIEWER_P_H

// application specific includes
#include "smb4kpreviewer.h"
#include "smb4kprocess.h"
#include "smb4kglobal.h"

// Qt includes
#include <QStringList>
#include <QListIterator>
#include <QUrl>
#include <QIcon>
#include <QDialog>
#include <QListWidget>
#include <QPushButton>

// KDE includes
#include <KCoreAddons/KJob>
#include <KCompletion/KHistoryComboBox>
#include <KWidgetsAddons/KDualAction>



class Smb4KPreviewFileItem
{
  public:
    /**
     * Constructor
     */
    Smb4KPreviewFileItem();
    
    /**
     * Destructor
     */
    ~Smb4KPreviewFileItem();
    
    /**
     * Set the item name.
     */
    void setItemName(const QString &name);
    
    /**
     * Returns the name of the item.
     */
    QString itemName() const;
    
    /**
     * Set the modification date of the file or directory.
     */
    void setDate(const QString &date);
    
    /**
     * Returns the modification date.
     */
    QString date() const;
    
    /**
     * Set the file size.
     */
    void setItemSize(const QString &size);
    
    /**
     * Returns the file size.
     */
    QString itemSize() const;
    
    /**
     * Mark the item as directory.
     */
    void setDir(bool dir);
    
    /**
     * Returns TRUE if the item is a directory.
     */
    bool isDir() const;
    
    /**
     * Returns TRUE if the item is a file.
     */
    bool isFile() const;
    
    /**
     * Returns TRUE if the file or directory is hidden.
     */
    bool isHidden() const;
    
    /**
     * Returns the icon for the item.
     */
    QIcon itemIcon() const;
    
  private:
    QString m_name;
    QString m_date;
    QString m_size;
    bool m_dir;
};


class Smb4KPreviewJob : public KJob
{
  Q_OBJECT

  public:
    /**
     * Constructor
     */
    explicit Smb4KPreviewJob(QObject *parent = 0);

    /**
     * Destructor
     */
    ~Smb4KPreviewJob();

    /**
     * Returns TRUE if the job has been started and FALSE otherwise
     *
     * @returns TRUE if the job has been started
     */
    bool isStarted() { return m_started; }

    /**
     * Starts the job
     */
    void start();

    /**
     * This function sets up the preview job.
     *
     * You must run this function before start() is called.
     *
     * @param share       The share for which the preview should be acquired
     *
     * @param url         The location that should be listed
     *
     * @param parent      The parent widget
     */
    void setupPreview(const SharePtr &share, const QUrl &url, QWidget *parent);

    /**
     * Returns the share object
     *
     * @returns the share object
     */
    const SharePtr &share() { return m_share; }

    /**
     * Returns the parent widget
     *
     * @returns the parent widget
     */
    QWidget *parentWidget() { return m_parent_widget; }

    /**
     * Returns the url
     *
     * @returns the path
     */
    const QUrl &location() { return m_url; }

  Q_SIGNALS:
    /**
     * Emitted when an authentication error happened.
     */
    void authError(Smb4KPreviewJob *job);

    /**
     * Emitted when the printing is about to begin.
     */
    void aboutToStart(const SharePtr &share, const QUrl &url);

    /**
     * Emitted after the printing finished.
     */
    void finished(const SharePtr &share, const QUrl &url);

    /**
     * Emits the contents of the directory just listed
     * 
     * @param url       The URL that should be previewed
     *
     * @param contents  The contents of the URL
     */
    void preview(const QUrl &url, const QList<Smb4KPreviewFileItem> &contents);

  protected:
    bool doKill();

  protected Q_SLOTS:
    void slotStartPreview();
    void slotProcessFinished(int exitCode, QProcess::ExitStatus status);

  private:
    void processOutput(const QString &stdOut);
    void processErrors(const QString &stdErr);
    bool m_started;
    SharePtr m_share;
    QWidget *m_parent_widget;
    Smb4KProcess *m_process;
    QUrl m_url;
};


class Q_DECL_EXPORT Smb4KPreviewDialog : public QDialog
{
  Q_OBJECT

  public:
    /**
     * This is the constructor of the preview dialog.
     *
     * @param share         The Smb4KShare object.
     *
     * @param parent        The parent of this widget
     */
    explicit Smb4KPreviewDialog(const SharePtr &share, QWidget *parent = 0);

    /**
     * The destructor.
     */
    ~Smb4KPreviewDialog();

    /**
     * Returns the share for which the preview is to be generated
     *
     * @returns the share
     */
    const SharePtr &share() { return m_share; }

  Q_SIGNALS:
    /**
     * Emitted when the dialog closes.
     *
     * @param dialog        This dialog
     */
    void aboutToClose(Smb4KPreviewDialog *dialog);

    /**
     * This signal requests a preview for the given share and URL.
     * Use the share for identification and authentication and the
     * URL to determine the path.
     * 
     * @param share         The share
     *
     * @param url           The location
     *
     * @param parent        The parent widget that should be used
     */
    void requestPreview(const SharePtr &share, const QUrl &url, QWidget *parent = 0);

    /**
     * This signal is emitted when the user wants to abort the acquisition
     * of the preview for the share.
     *
     * @param share         The share
     */
    void abortPreview(const SharePtr &share);

  protected Q_SLOTS:
    void slotReloadAbortActionTriggered(bool);
    void slotBackActionTriggered(bool);
    void slotForwardActionTriggered(bool);
    void slotUpActionTriggered(bool);

    /**
     * This slot can be called to request a preview. However, it only
     * can use the current URL.
     */
    void slotRequestPreview();

    /**
     * This slot takes the contents of a certain directory and displays
     * the list of files and directories.
     *
     * @param url           The URL that was queried
     * 
     * @param contents      The contents of a certain directory
     */
    void slotDisplayPreview(const QUrl &url, const QList<Smb4KPreviewFileItem> &contents);
    
    /**
     * This slot is called when the preview process is about to start.
     *
     * @param item          The share item
     *
     * @param url           The location for which the preview should be 
     *                      acquired
     */
    void slotAboutToStart(const SharePtr &share, const QUrl &url);

    /**
     * This slot is called when the preview process finished.
     *
     * @param item          The Smb4KShare item
     *
     * @param url           The location for which the preview should be
     *                      acquired
     */
    void slotFinished(const SharePtr &share, const QUrl &url);

    /**
     * Is called, if an item has been executed.
     *
     * @param item          The item that has been executed.
     */
    void slotItemExecuted(QListWidgetItem *item);

    /**
     * Is called, if an item in the combo box is activated.
     */
    void slotItemActivated(const QString &item);

    /**
     * This slot is called when the close button was clicked.
     */
    void slotCloseClicked();

    /**
     * This slot is called if the icon size was changed.
     *
     * @param group               The icon group
     */
    void slotIconSizeChanged(int group);

  private:
    QPushButton *m_close_button;
    
    /**
     * The share object
     */
    SharePtr m_share;

    /**
     * The current URL
     */
    QUrl m_url;
    
    /**
     * Enumeration for the items in the list view.
     */
    enum ItemType{ File = 1000,
                   Directory = 1001 };

    /**
     * Sets up the file view.
     */
    void setupView();

    /**
     * The icon view.
     */
    QListWidget *m_view;

    /**
     * The combo box.
     */
    KHistoryComboBox *m_combo;

    /**
     * Reload/Abort dual action
     */
    KDualAction *m_reload_abort;

    /**
     * Back action
     */
    QAction *m_back;

    /**
     * Forward action
     */
    QAction *m_forward;

    /**
     * Up action
     */
    QAction *m_up;
    
    /**
     * The history
     */
    QList<QUrl> m_history;
    
    /**
     * The iterator used to iterate through the history
     */
    QList<QUrl>::const_iterator m_iterator;

    /**
     * An action from the toolbar was used
     */
    bool m_actionUsed;
};


class Smb4KPreviewerPrivate
{
  public:
    QList<Smb4KPreviewDialog *> dialogs;
};


class Smb4KPreviewerStatic
{
  public:
    Smb4KPreviewer instance;
};

#endif

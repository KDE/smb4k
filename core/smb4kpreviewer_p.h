/***************************************************************************
    smb4kpreviewer_p  -  Private helper classes for Smb4KPreviewer class.
                             -------------------
    begin                : So Dez 21 2008
    copyright            : (C) 2008-2010 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KPREVIEWER_P_H
#define SMB4KPREVIEWER_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QStringList>
#include <QListIterator>

// KDE includes
#include <kjob.h>
#include <kdialog.h>
#include <klistwidget.h>
#include <khistorycombobox.h>
#include <kaction.h>

// application specific includes
#include <smb4kpreviewer.h>
#include <smb4kprocess.h>

// forward declarations
class Smb4KShare;

#define FileItem            100
#define HiddenFileItem      101
#define DirectoryItem       102
#define HiddenDirectoryItem 103

// Use this as follows:
// - The integer defines the type of the item (see above)
// - The key of the map is one of the following entries:
//   + name - The name of the file or directory (mandatory)
//   + date - The modification date of the file or directory (optional)
//   + size - The file size (optional)
typedef QPair< int,QMap<QString,QString> > Item;

class Smb4KPreviewJob : public KJob
{
  Q_OBJECT

  public:
    /**
     * Constructor
     */
    Smb4KPreviewJob( QObject *parent = 0 );

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
    void setupPreview( Smb4KShare *share,
                       const QUrl &url,
                       QWidget *parent );

    /**
     * Returns the share object
     *
     * @returns the share object
     */
    Smb4KShare *share() { return m_share; }

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

  signals:
    /**
     * Emitted when an authentication error happened.
     */
    void authError( Smb4KPreviewJob *job );

    /**
     * Emitted when the printing is about to begin.
     */
    void aboutToStart( Smb4KShare *share,
                       const QUrl &url );

    /**
     * Emitted after the printing finished.
     */
    void finished( Smb4KShare *share,
                   const QUrl &url );

    /**
     * Emits the contents of the directory just listed
     * 
     * @param url       The URL that should be previewed
     *
     * @param contents  The contents of the URL
     */
    void preview( const QUrl &url,
                  const QList<Item> &contents );

  protected:
    bool doKill();

  protected slots:
    void slotStartPreview();
    void slotReadStandardOutput();
    void slotReadStandardError();
    void slotProcessFinished( int exitCode, QProcess::ExitStatus status );

  private:
    bool m_started;
    Smb4KShare *m_share;
    QWidget *m_parent_widget;
    Smb4KProcess *m_proc;
    QUrl m_url;
};


class KDE_EXPORT Smb4KPreviewDialog : public KDialog
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
    Smb4KPreviewDialog( Smb4KShare *share,
                        QWidget *parent = 0 );

    /**
     * The destructor.
     */
    ~Smb4KPreviewDialog();

    /**
     * Returns the share for which the preview is to be generated
     *
     * @returns the share
     */
    Smb4KShare *share() { return m_share; }

  signals:
    /**
     * Emitted when the dialog closes.
     *
     * @param dialog        This dialog
     */
    void aboutToClose( Smb4KPreviewDialog *dialog );

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
    void requestPreview( Smb4KShare *share,
                         const QUrl &url,
                         QWidget *parent = 0 );

    /**
     * This signal is emitted when the user wants to abort the acquisition
     * of the preview for the share.
     *
     * @param share         The share
     */
    void abortPreview( Smb4KShare *share );

  protected slots:
    /**
     * This slot is called when an action has been triggered.
     *
     * @param action        The triggered action
     */
    void slotActionTriggered( QAction *action );

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
    void slotDisplayPreview( const QUrl &url,
                             const QList<Item> &contents );
    
    /**
     * This slot is called when the preview process is about to start.
     *
     * @param item          The Smb4KShare item
     *
     * @param url           The location for which the preview should be 
     *                      acquired
     */
    void slotAboutToStart( Smb4KShare *share,
                           const QUrl &url );

    /**
     * This slot is called when the preview process finished.
     *
     * @param item          The Smb4KShare item
     *
     * @param url           The location for which the preview should be
     *                      acquired
     */
    void slotFinished( Smb4KShare *share,
                       const QUrl &url );

    /**
     * Is called, if an item has been executed.
     *
     * @param item          The item that has been exected.
     */
    void slotItemExecuted( QListWidgetItem *item );

    /**
     * Is called, if an item in the combo box is activated.
     */
    void slotItemActivated( const QString &item );

    /**
     * This slot is called when the close button was clicked.
     */
    void slotCloseClicked();

    /**
     * This slot is called if the icon size was changed.
     *
     * @param group               The icon group
     */
    void slotIconSizeChanged( int group );

  private:
    /**
     * The share object
     */
    Smb4KShare *m_share;

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
    KListWidget *m_view;

    /**
     * The combo box.
     */
    KHistoryComboBox *m_combo;

    /**
     * Reload action
     */
    KAction *m_reload;

    /**
     * Abort action
     */
    KAction *m_abort;

    /**
     * Back action
     */
    KAction *m_back;

    /**
     * Forward action
     */
    KAction *m_forward;

    /**
     * Up action
     */
    KAction *m_up;

    /**
     * The current history
     */
    QStringList m_history;

    /**
     * The current position in the history.
     */
    QStringListIterator m_iterator;
};


class Smb4KPreviewerPrivate
{
  public:
    Smb4KPreviewerPrivate();
    ~Smb4KPreviewerPrivate();
    Smb4KPreviewer instance;
};

#endif

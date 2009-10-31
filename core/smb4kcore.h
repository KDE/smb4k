/***************************************************************************
    smb4kcore  -  The main core class of Smb4K.
                             -------------------
    begin                : Do Apr 8 2004
    copyright            : (C) 2004-2009 by Alexander Reinholdt
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

#ifndef SMB4KCORE_H
#define SMB4KCORE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>
#include <QList>

// KDE includes
#include <kdemacros.h>

// application specific includes
#include <smb4kscanner.h>
#include <smb4kmounter.h>
#include <smb4kbookmarkhandler.h>
#include <smb4kprint.h>
#include <smb4ksynchronizer.h>
#include <smb4kpreviewer.h>
#include <smb4ksearch.h>
#include <smb4ksudowriterinterface.h>
#include <smb4ksolidinterface.h>
#include <smb4kipaddressscanner.h>

// forward declarations
class Smb4KWorkgroup;
class Smb4KHost;
class Smb4KShare;
class Smb4KCorePrivate;


/**
 * This is the top-level core class. It inherits most of the core classes.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KCore : public QObject
{
  Q_OBJECT

  friend class Smb4KCorePrivate;

  public:
    /**
     * This enumeration determines with which application the mount point
     * of the current mounted share is to be opened.
     */
    enum OpenWith { FileManager,
                    Konsole };

    /**
     * Returns a static pointer to this class.
     */
    static Smb4KCore *self();

    /**
     * Returns TRUE if one of the core classes is doing something.
     */
    static bool isRunning();

    /**
     * Returns the current state the core is in.
     */
    static int currentState() { return self()->m_current_state; }

    /**
     * Returns a pointer to the scanner object.
     */
    static Smb4KScanner *scanner() { return Smb4KScanner::self(); }

    /**
     * Returns a pointer to the mounter object.
     */
    static Smb4KMounter *mounter() { return Smb4KMounter::self(); }

    /**
     * Returns a pointer to the bookmark handler object.
     */
    static Smb4KBookmarkHandler *bookmarkHandler() { return self()->m_bookmarkHandler; }

    /**
     * Returns a pointer to the printer handler object.
     */
    static Smb4KPrint *print() { return Smb4KPrint::self(); }

    /**
     * Returns a pointer to the synchronizer object.
     */
    static Smb4KSynchronizer *synchronizer() { return Smb4KSynchronizer::self(); }

    /**
     * Returns a pointer to the previewer object.
     */
    static Smb4KPreviewer *previewer() { return Smb4KPreviewer::self(); }

    /**
     * Returns a pointer to the search object.
     */
    static Smb4KSearch *search() { return Smb4KSearch::self(); }

    /**
     * Returns a pointer to the Smb4KSudoWriterInterface object.
     */
    static Smb4KSudoWriterInterface *sudoWriter() { return Smb4KSudoWriterInterface::self(); }

    /**
     * Returns a pointer to the Smb4KSolidInterface object.
     */
    static Smb4KSolidInterface *solidInterface() { return Smb4KSolidInterface::self(); }

    /**
     * Returns a pointer to the Smb4KIPAddressScanner object.
     */
    static Smb4KIPAddressScanner *ipScanner() { return Smb4KIPAddressScanner::self(); }

    /**
     * Aborts any action of the core.
     */
    static void abort();

    /**
     * Open the mount point of a share. Which application is used is determined by
     * the value of @p openWith and - maybe - by settings that were defined by the
     * user.
     *
     * @param share         The share that is to be opened.
     *
     * @param openWith      Integer of type Smb4KCore::OpenWith. Determines with which
     *                      application the share should be opened.
     */
    static void open( Smb4KShare *share,
                      OpenWith openWith = FileManager );

    /**
     * This function initializes the core classes.
     */
    void init();

  signals:
    /**
     * This signal is emitted, if one of the core objects
     * starts or stops running.
     */
    void runStateChanged();

  protected slots:
    /**
     * This slot is connected to the QApplication::aboutToQuit() signal. It is invoked
     * when the application is shut down by the KDE logout or by pressing CTRL+Q, etc.
     */
    void slotAboutToQuit();

    /**
     * This slot sets the current state of the core.
     */
    void slotSetCurrentState();

  private:
    /**
     * The constructor.
     */
    Smb4KCore();

    /**
     * The destructor.
     */
    ~Smb4KCore();

    /**
     * The bookmark handler object.
     */
    Smb4KBookmarkHandler *m_bookmarkHandler;

    /**
     * Holds the current state.
     */
    int m_current_state;

    /**
     * Searches for the needed programs and emits an error
     * if mandatory ones are missing.
     */
    void searchPrograms();

    /**
     * Set default values for settings that depend on the system Smb4K is
     * running on and that have to be set dynamically.
     */
    void setDefaultSettings();
};

#endif

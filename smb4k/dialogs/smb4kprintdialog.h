/***************************************************************************
    smb4kprintdialog  -  The print dialog for Smb4K
                             -------------------
    begin                : So Apr 11 2004
    copyright            : (C) 2004-2008 by Alexander Reinholdt
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

#ifndef SMB4KPRINTDIALOG_H
#define SMB4KPRINTDIALOG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QString>

// KDE includes
#include <kdialog.h>
#include <kurlrequester.h>
#include <knuminput.h>
#include <kdemacros.h>

// application specific includes
#include <core/smb4kshare.h>
#include <core/smb4kprintinfo.h>


/**
 * This class provides the print dialog for Smb4K. You can choose the
 * file that is to be printed and you can define the number of copies
 * you want to have.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KPrintDialog : public KDialog
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param share       The Smb4KShare item representing the printer.
     *
     * @param parent      The parent widget of this dialog.
     */
    Smb4KPrintDialog( Smb4KShare *share,
                      QWidget *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KPrintDialog();

  protected slots:
    /**
     * This slot is called when the User1 (i.e. the "Cloase") button
     * has been clicked.
     */
    void slotUser1Clicked();

    /**
     * This slot is called when the User2 (i.e. the "Print") button has been clicked.
     */
    void slotUser2Clicked();

    /**
     * This slot is being enabled if there is input text.
     *
     * @param text        The input text.
     */
    void slotInputValueChanged( const QString &text );

    /**
     * This slot is called when the print process represented by @p info is about
     * to be started.
     *
     * @param info        The Smb4KPrintInfo object
     */
    void slotAboutToStart( Smb4KPrintInfo *info );

    /**
     * This slot is called when the print process represented by @p info finished.
     *
     * @param info        The Smb4KPrintInfo object
     */
    void slotFinished( Smb4KPrintInfo *info );

  private:
    /**
     * Set up the view.
     */
    void setupView();

    /**
     * The url requester
     */
    KUrlRequester *m_file;

    /**
     * The copies input
     */
    KIntNumInput *m_copies;

    /**
     * The Smb4KPrintInfo object
     */
    Smb4KPrintInfo *m_info;
};

#endif

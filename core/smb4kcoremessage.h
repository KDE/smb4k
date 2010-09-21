/***************************************************************************
    smb4kcoremessage  -  This class provides messages for use with the
    core classes of Smb4K.
                             -------------------
    begin                : Sa Mär 8 2008
    copyright            : (C) 2008 by Alexander Reinholdt
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

#ifndef SMB4KCOREMESSAGE_H
#define SMB4KCOREMESSAGE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QString>
#include <QProcess>

// KDE includes
#include <kdemacros.h>

//
// Error codes:
//

// (8) Errors that occur when working with programs
// #define ERROR_MISSING_PROGRAMS              120

// (9) Process errors
#define ERROR_PROCESS_EXIT                  122
#define ERROR_PROCESS_ERROR                 123

// (10) Other errors
#define ERROR_GETTING_HOSTNAME              125
#define ERROR_FEATURE_NOT_ENABLED           126
#define ERROR_XML_ERROR                     127

/**
 * This class provides messages for use with the core classes of Smb4K.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KCoreMessage
{
  public:
    /**
     * Show an error that occurred in the core.
     *
     * Note: If you want to report an error in  one of the shell processes, please use
     * Smb4KCoreMessage::processError().
     *
     * @param code        The error code as defined in smb4kcoremessage.h
     *
     * @param text        Short text which will be included in the message that's shown
     *                    to the user. Normally, a file name or similar is entered here.
     *                    May be left blank if you do not need to fill text into the error
     *                    message. Please note, that this text is not used to show details.
     *
     * @param details     The text passed here is used to show details. Please note, that
     *                    it depends on the kind of error if it is indeed shown.
     */
    static void error( int code,
                       const QString &text = QString(),
                       const QString &details = QString() );

    /**
     * Show a process error that occurred in the core.
     *
     * Note: If you want to report an error of a different type, please use
     * Smb4KCoreMessage::error()
     *
     * @param code        The error code as defined in smb4kcoremessage.h
     *
     * @param error       The process error code as reported by QProcess::error().
     */
    static void processError( int code,
                              QProcess::ProcessError error );
};

#endif

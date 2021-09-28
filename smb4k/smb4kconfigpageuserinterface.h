/*
    This configuration page takes care of all settings concerning the
    user interface
    -------------------
    begin                : Mi Aug 30 2006
    SPDX-FileCopyrightText: 2006-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGPAGEUSERINTERFACE_H
#define SMB4KCONFIGPAGEUSERINTERFACE_H

// Qt includes
#include <QTabWidget>

/**
 * The configuration page for the user interface of Smb4K.
 *
 * @author Alexander Reinholdt  <alexander.reinholdt@kdemail.net>
 */

class Smb4KConfigPageUserInterface : public QWidget
{
    Q_OBJECT

public:
    /**
     * The constructor
     *
     * @param parent          The parent widget of this class.
     */
    explicit Smb4KConfigPageUserInterface(QWidget *parent = 0);

    /**
     * The destructor
     */
    ~Smb4KConfigPageUserInterface();
};

#endif

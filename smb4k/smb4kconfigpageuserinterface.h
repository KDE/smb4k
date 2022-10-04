/*
    This configuration page takes care of all settings concerning the
    user interface

    SPDX-FileCopyrightText: 2006-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGPAGEUSERINTERFACE_H
#define SMB4KCONFIGPAGEUSERINTERFACE_H

// Qt includes
#include <QWidget>

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
    explicit Smb4KConfigPageUserInterface(QWidget *parent = nullptr);

    /**
     * The destructor
     */
    ~Smb4KConfigPageUserInterface();
};

#endif

/*
 *  Password dialog
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SMB4KPASSWORDDIALOG_H
#define SMB4KPASSWORDDIALOG_H

// application specific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QMap>

// KDE includes
#include <KPasswordDialog>

class Q_DECL_EXPORT Smb4KPasswordDialog : public KPasswordDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KPasswordDialog(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    virtual ~Smb4KPasswordDialog();

    /**
     * Setup the dialog.
     *
     * @param networkItem       The network item
     */
    bool setNetworkItem(const NetworkItemPtr &networkItem);

protected:
    void accept() override;

private:
    NetworkItemPtr m_networkItem;
};

#endif

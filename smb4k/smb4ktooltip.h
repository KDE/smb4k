/*
    smb4ktooltip  -  Provides tooltips for Smb4K

    SPDX-FileCopyrightText: 2020-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KTOOLTIP_H
#define SMB4KTOOLTIP_H

// application specific includes
#include <smb4kglobal.h>

// Qt includes
#include <QHBoxLayout>
#include <QPoint>
#include <QWindow>
#include <QLabel>
#include <QFormLayout>

// KDE includes
#include <KToolTipWidget>

// Forward declaration
class Smb4KBasicNetworkItem;

class Smb4KToolTip : public KToolTipWidget
{
    Q_OBJECT

public:
    /**
     * The constructor
     */
    Smb4KToolTip(QWidget *parent = nullptr);

    /**
     * The destructor
     */
    ~Smb4KToolTip();

    /**
     * This enumeration determines the kind of tooltip
     * this is to be shown.
     *
     * @enum NetworkItem  Tooltip reflecting a remote network item
     * @enum MountedShare Tooltip reflecting a mounted share
     */
    enum Type { NetworkItem, MountedShare, Unknown };

    /**
     * Set up the tooltip.
     */
    void setupToolTip(Type type, NetworkItemPtr item);

    /**
     * Show the tooltip
     */
    void show(const QPoint &pos, QWindow *transientParent);

    /**
     * Update the tooltip
     */
    void update();

private:
    void setupNetworkItemContents();
    void setupMountedShareContents();
    NetworkItemPtr m_item;
    Type m_type;
    QWidget *m_contentsWidget;
    QLabel *m_iconLabel;
    QFormLayout *m_formLayout;
};

#endif

/*
    smb4ktooltip  -  Provides tooltips for Smb4K
    -------------------
    begin                : Mi Mai 2020
    SPDX-FileCopyrightText: 2020-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
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

// KDE includes
#include <KWidgetsAddons/KToolTipWidget>

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
    /**
     * Setup the contents widget for a remote network item
     */
    void setupNetworkItemContents();

    /**
     * Setup the contents widget for a mounted share
     */
    void setupMountedShareContents();

    /**
     * The network item
     */
    NetworkItemPtr m_item;

    /**
     * The type
     */
    Type m_type;

    /**
     * The contents widget for the tooltip
     */
    QWidget *m_contentsWidget;

    /**
     * The main layout
     */
    QHBoxLayout *m_mainLayout;
};

#endif

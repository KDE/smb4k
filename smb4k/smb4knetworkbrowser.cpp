/*
    smb4knetworkbrowser  -  The network browser widget of Smb4K.

    SPDX-FileCopyrightText: 2007-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4knetworkbrowser.h"
#include "core/smb4kglobal.h"
#include "core/smb4ksettings.h"
#include "core/smb4kshare.h"
#include "smb4knetworkbrowseritem.h"
#include "smb4ktooltip.h"

// Qt includes
#include <QApplication>
#include <QHeaderView>
#include <QMouseEvent>
#include <QTimer>

#include <QLayout>

// KDE includes
#include <KLocalizedString>

using namespace Smb4KGlobal;

Smb4KNetworkBrowser::Smb4KNetworkBrowser(QWidget *parent)
    : QTreeWidget(parent)
{
    setRootIsDecorated(true);
    setAllColumnsShowFocus(false);
    setMouseTracking(true);
    setSelectionMode(ExtendedSelection);

    setContextMenuPolicy(Qt::CustomContextMenu);

    m_toolTip = new Smb4KToolTip(this);

    QStringList header_labels;
    header_labels.append(i18n("Network"));
    header_labels.append(i18n("Type"));
    header_labels.append(i18n("IP Address"));
    header_labels.append(i18n("Comment"));
    setHeaderLabels(header_labels);

    header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    //
    // Connections
    //
    connect(this, &Smb4KNetworkBrowser::itemActivated, this, &Smb4KNetworkBrowser::slotItemActivated);
    connect(this, &Smb4KNetworkBrowser::itemSelectionChanged, this, &Smb4KNetworkBrowser::slotItemSelectionChanged);
}

Smb4KNetworkBrowser::~Smb4KNetworkBrowser()
{
}

Smb4KToolTip *Smb4KNetworkBrowser::toolTip()
{
    return m_toolTip;
}

bool Smb4KNetworkBrowser::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::ToolTip: {
        QPoint pos = viewport()->mapFromGlobal(cursor().pos());
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(itemAt(pos));

        if (item) {
            if (Smb4KSettings::showNetworkItemToolTip()) {
                int ind = 0;

                switch (item->type()) {
                case Host: {
                    ind = 2;
                    break;
                }
                case Share: {
                    ind = 3;
                    break;
                }
                default: {
                    ind = 1;
                    break;
                }
                }

                if (pos.x() > ind * indentation()) {
                    m_toolTip->setupToolTip(Smb4KToolTip::NetworkItem, item->networkItem());
                    m_toolTip->show(cursor().pos(), nativeParentWidget()->windowHandle());
                }
            }
        }

        break;
    }
    default: {
        break;
    }
    }

    return QTreeWidget::event(e);
}

void Smb4KNetworkBrowser::mousePressEvent(QMouseEvent *e)
{
    if (m_toolTip->isVisible()) {
        m_toolTip->hide();
    }

    QTreeWidgetItem *item = itemAt(e->pos());

    if (!item && currentItem()) {
        currentItem()->setSelected(false);
        setCurrentItem(nullptr);
    }

    QTreeWidget::mousePressEvent(e);
}

void Smb4KNetworkBrowser::mouseMoveEvent(QMouseEvent *e)
{
    if (m_toolTip->isVisible()) {
        m_toolTip->hide();
    }

    QTreeWidget::mouseMoveEvent(e);
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KNetworkBrowser::slotItemActivated(QTreeWidgetItem *item, int /*column*/)
{
    // Only do something if there are no keyboard modifiers pressed
    // and there is only one item selected.
    if (QApplication::keyboardModifiers() == Qt::NoModifier && selectedItems().size() == 1) {
        if (item) {
            switch (item->type()) {
            case Workgroup:
            case Host: {
                if (!item->isExpanded()) {
                    expandItem(item);
                } else {
                    collapseItem(item);
                }

                break;
            }
            default: {
                break;
            }
            }
        }
    }
}

void Smb4KNetworkBrowser::slotItemSelectionChanged()
{
    if (selectedItems().size() > 1) {
        // If multiple items are selected, only allow shares
        // to stay selected.
        for (int i = 0; i < selectedItems().size(); ++i) {
            Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(selectedItems()[i]);

            if (item) {
                switch (item->networkItem()->type()) {
                case Workgroup:
                case Host: {
                    item->setSelected(false);
                    break;
                }
                case Share: {
                    if (item->shareItem()->isPrinter()) {
                        item->setSelected(false);
                    }
                    break;
                }
                default: {
                    break;
                }
                }
            }
        }
    }
}

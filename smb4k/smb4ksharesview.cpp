/*
    This is the shares view of Smb4K.

    SPDX-FileCopyrightText: 2006-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4ksharesview.h"
#include "core/smb4ksettings.h"
#include "core/smb4kshare.h"
#include "smb4ksharesviewitem.h"
#include "smb4ktooltip.h"

// Qt includes
#include <QDrag>
#include <QMouseEvent>
#include <QStorageInfo>
#include <QWheelEvent>

// KDE includes
#include <KIconLoader>

Smb4KSharesView::Smb4KSharesView(QWidget *parent)
    : QListWidget(parent)
{
    setMouseTracking(true);
    setSelectionMode(ExtendedSelection);
    setResizeMode(Adjust);
    setSortingEnabled(true);
    setWordWrap(true);
    setAcceptDrops(true);
    setDragEnabled(true);
    setDropIndicatorShown(true);
    setUniformItemSizes(true);
    setWrapping(true);
    setTextElideMode(Qt::ElideNone);

    m_toolTip = new Smb4KToolTip(this);

    setContextMenuPolicy(Qt::CustomContextMenu);
}

Smb4KSharesView::~Smb4KSharesView()
{
}

void Smb4KSharesView::setViewMode(QListView::ViewMode mode, int iconSize)
{
    //
    // Set the view mode
    //
    QListWidget::setViewMode(mode);

    //
    // Make adjustments
    //
    switch (mode) {
    case IconMode: {
        setUniformItemSizes(true);
        setIconSize(QSize(iconSize, iconSize));
        setSpacing(5);
        break;
    }
    case ListMode: {
        setUniformItemSizes(false);
        setIconSize(QSize(iconSize, iconSize));
        setSpacing(0);
        break;
    }
    default: {
        break;
    }
    }

    //
    // Align the items
    //
    for (int i = 0; i < count(); ++i) {
        Smb4KSharesViewItem *viewItem = static_cast<Smb4KSharesViewItem *>(item(i));
        viewItem->setItemAlignment(mode);
    }
}

Smb4KToolTip *Smb4KSharesView::toolTip()
{
    return m_toolTip;
}

bool Smb4KSharesView::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::ToolTip: {
        // Intercept the tool tip event and show our own tool tip.
        QPoint pos = viewport()->mapFromGlobal(cursor().pos());
        Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(itemAt(pos));

        if (item) {
            if (Smb4KSettings::showShareToolTip()) {
                //
                // Set up the tooltip
                //
                m_toolTip->setupToolTip(Smb4KToolTip::MountedShare, item->shareItem());

                //
                // Show the tooltip
                //
                m_toolTip->show(cursor().pos(), nativeParentWidget()->windowHandle());
            }
        }

        break;
    }
    default: {
        break;
    }
    }

    return QListWidget::event(e);
}

void Smb4KSharesView::mousePressEvent(QMouseEvent *e)
{
    //
    // Hide the tooltip
    //
    if (m_toolTip->isVisible()) {
        m_toolTip->hide();
    }

    //
    // Get the item that is under the mouse. If there is no
    // item, unselect the current item.
    //
    QListWidgetItem *item = itemAt(e->position().toPoint());

    if (!item && !selectedItems().isEmpty()) {
        clearSelection();
        setCurrentItem(nullptr);
        Q_EMIT itemPressed(currentItem());
    }

    QListWidget::mousePressEvent(e);
}

void Smb4KSharesView::mouseMoveEvent(QMouseEvent *e)
{
    //
    // Hide the tooltip
    //
    if (m_toolTip->isVisible()) {
        m_toolTip->hide();
    }

    QListWidget::mouseMoveEvent(e);
}

void Smb4KSharesView::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->accept();
    } else {
        e->ignore();
    }
}

void Smb4KSharesView::dragMoveEvent(QDragMoveEvent *e)
{
    QAbstractItemView::dragMoveEvent(e);

    if (e->proposedAction() == Qt::CopyAction || e->proposedAction() == Qt::MoveAction) {
        Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(itemAt(e->position().toPoint()));

        if (item) {
            QStorageInfo storageInfo(item->shareItem()->canonicalPath());

            if (!storageInfo.isReadOnly() && !item->shareItem()->isInaccessible() && (item->flags() & Qt::ItemIsDropEnabled)) {
                QUrl url = QUrl::fromLocalFile(item->shareItem()->path());

                if (e->source() == this && e->mimeData()->urls().first() == url) {
                    e->ignore();
                } else {
                    e->accept();
                }
            } else {
                e->ignore();
            }
        } else {
            e->ignore();
        }
    } else {
        e->ignore();
    }
}

void Smb4KSharesView::dropEvent(QDropEvent *e)
{
    if (e->proposedAction() == Qt::CopyAction || e->proposedAction() == Qt::MoveAction) {
        Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(itemAt(e->position().toPoint()));

        if (item) {
            QStorageInfo storageInfo(item->shareItem()->canonicalPath());

            if (!storageInfo.isReadOnly() && !item->shareItem()->isInaccessible() && (item->flags() & Qt::ItemIsDropEnabled)
                && (e->proposedAction() == Qt::CopyAction || e->proposedAction() == Qt::MoveAction)) {
                QUrl url = QUrl::fromLocalFile(item->shareItem()->path());

                if (e->source() == this && e->mimeData()->urls().first() == url) {
                    e->ignore();
                } else {
                    e->acceptProposedAction();
                    Q_EMIT acceptedDropEvent(item, e);
                    // e->accept();
                }
            } else {
                e->ignore();
            }
        } else {
            e->ignore();
        }
    } else {
        e->ignore();
    }
}

Qt::DropActions Smb4KSharesView::supportedDropActions() const
{
    return (Qt::CopyAction | Qt::LinkAction);
}

QMimeData *Smb4KSharesView::mimeData(const QList<QListWidgetItem *> &list) const
{
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;

    for (int i = 0; i < list.count(); ++i) {
        Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(list.at(i));
        urls << QUrl::fromLocalFile(item->shareItem()->path());
    }

    mimeData->setUrls(urls);

    return mimeData;
}

void Smb4KSharesView::startDrag(Qt::DropActions supported)
{
    QList<QListWidgetItem *> list = selectedItems();

    if (!list.isEmpty()) {
        QMimeData *data = mimeData(list);

        if (!data) {
            return;
        }

        QDrag *drag = new QDrag(this);

        QPixmap pixmap;

        if (list.count() == 1) {
            Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(list.first());
            pixmap = item->icon().pixmap(KIconLoader::SizeMedium);
        } else {
            pixmap = KDE::icon(QStringLiteral("document-multiple")).pixmap(KIconLoader::SizeMedium);
        }

        drag->setPixmap(pixmap);
        drag->setMimeData(data);

        drag->exec(supported, Qt::IgnoreAction);
    }
}

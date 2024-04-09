/*
 *  Preview dialog
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SMB4KPREVIEWDIALOG_H
#define SMB4KPREVIEWDIALOG_H

// application specific includes
#include "core/smb4kglobal.h"
#include "smb4kdialogs_export.h"

// Qt includes
#include <QAction>
#include <QDialog>
#include <QListWidget>
#include <QPushButton>

// KDE includes
#include <KDualAction>
#include <KUrlComboBox>

class SMB4KDIALOGS_EXPORT Smb4KPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KPreviewDialog(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    virtual ~Smb4KPreviewDialog();

    /**
     * Set the share that is to be previewed.
     *
     * @param share     The share item
     *
     * @returns TRUE if the share was set successfully.
     */
    bool setShare(SharePtr share);

public Q_SLOTS:
    void loadPreview(const NetworkItemPtr &networkItem);

protected Q_SLOTS:
    void slotCloseButtonClicked();
    void slotItemActivated(QListWidgetItem *item);
    void slotPreviewResults(const QList<FilePtr> &files);
    void slotReloadActionTriggered(bool checked);
    void slotUpActionTriggered();
    void slotUrlActivated(const QUrl &url);
    void slotAdjustReloadAction(const NetworkItemPtr &item, int type);

private:
    QListWidget *m_listWidget;
    QPushButton *m_closeButton;
    SharePtr m_share;
    NetworkItemPtr m_currentItem;
    KDualAction *m_reloadAction;
    QAction *m_upAction;
    KUrlComboBox *m_urlComboBox;
};

#endif

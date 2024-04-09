/*
 *  Bookmark editor
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SMB4KBOOKMARKEDITOR_H
#define SMB4KBOOKMARKEDITOR_H

// application specific includes
#include "smb4kconfigpagebookmarks.h"
#include "smb4kdialogs_export.h"

// Qt includes
#include <QDialog>
#include <QPushButton>

class SMB4KDIALOGS_EXPORT Smb4KBookmarkEditor : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KBookmarkEditor(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    virtual ~Smb4KBookmarkEditor();

protected Q_SLOTS:
    /**
     * Invoked when the "Save" button should be enabled
     */
    void slotEnabledButtons();

    /**
     * Invoked when the dialog is accepted and the bookmarks
     * are to be saved.
     */
    void slotSaveBookmarks();

private:
    Smb4KConfigPageBookmarks *m_mainWidget;
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
};

#endif

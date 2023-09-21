/*
 *  Bookmark dialog
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SMB4KBOOKMARKDIALOG_H
#define SMB4KBOOKMARKDIALOG_H

// application specific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QDialog>
#include <QListWidget>
#include <QPushButton>

// KDE includes
#include <KComboBox>
#include <KLineEdit>

class Q_DECL_EXPORT Smb4KBookmarkDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    Smb4KBookmarkDialog(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~Smb4KBookmarkDialog();

    /**
     * Set the shares that are to be bookmarked. If all shares have already been
     * bookmarked, this function returns FALSE otherwise TRUE.
     *
     * This function should be run before showing the dialog.
     *
     * @param shares        The list of shares that are to be bookmarked
     *
     * @returns TRUE if at least one of the shares has not been bookmarked yet.
     */
    bool setBookmarks(const QList<SharePtr> &shares);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

protected Q_SLOTS:
    void slotItemDoubleClicked(QListWidgetItem *item);
    void slotItemSelectionChanged();
    void slotLabelEdited();
    void slotCategoryEdited();
    void slotAccepted();
    void slotRejected();

private:
    QListWidget *m_listWidget;
    QWidget *m_editorWidget;
    KLineEdit *m_labelEdit;
    KComboBox *m_categoryEdit;
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
};

#endif

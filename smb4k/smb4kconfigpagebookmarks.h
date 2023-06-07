/*
    The configuration page for bookmarks

    SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGPAGEBOOKMARKS_H
#define SMB4KCONFIGPAGEBOOKMARKS_H

// application specific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QTreeWidget>
#include <QLabel>

// KDE includes
#include <KLineEdit>
#include <KComboBox>
#include <QPushButton>

/**
 * This configuration page contains the bookmarks
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 3.3.0
 */

class Smb4KConfigPageBookmarks : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KConfigPageBookmarks(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    virtual ~Smb4KConfigPageBookmarks();

    /**
     * Returns TRUE if the bookmarks have possibly changed
     * and FALSE otherwise.
     */
    bool bookmarksChanged() const;

protected:
    /**
     * Reimplemented from QObject
     */
    bool eventFilter(QObject *obj, QEvent *e) override;

Q_SIGNALS:
    /**
     * Emitted when the bookmarks may have been modified.
     */
    void bookmarksModified();

public Q_SLOTS:
    /**
     * Load the bookmarks
     */
    void loadBookmarks();

    /**
     * Save the bookmarks
     */
    void saveBookmarks();

protected Q_SLOTS:
    void slotResetButtonClicked(bool checked);
    void slotEditButtonClicked(bool checked);
    void slotAddCategoryButtonClicked(bool checked);
    void slotRemoveButtonClicked(bool checked);
    void slotClearButtonClicked(bool checked);
    void slotCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void slotItemSelectionChanged();
    void slotItemDoubleClicked(QTreeWidgetItem *item, int column);
    void slotLabelEdited();
    void slotCategoryEdited();
    void slotUserNameEdited();
    void slotWorkgroupEdited();
    void slotIpAddressEdited();
    void slotEnableButtons();

private:
    enum Role {
        TypeRole = Qt::UserRole,
        DataRole = Qt::UserRole + 1
    };
    enum Type {
        CategoryType = Qt::UserRole + 100,
        BookmarkType = Qt::UserRole + 101
    };
    QTreeWidgetItem *addCategoryItem(const QString &text);
    void startEditingCategoryItem(QTreeWidgetItem *item);
    void endEditingCategoryItem(QTreeWidgetItem *item);
    QList<BookmarkPtr> m_bookmarks;
    QTreeWidget *m_treeWidget;
    QWidget *m_editorWidget;
    QLabel *m_labelLabel;
    KLineEdit *m_labelEdit;
    QLabel *m_categoryLabel;
    KComboBox *m_categoryEdit;
    QLabel *m_userNameLabel;
    KLineEdit *m_userNameEdit;
    QLabel *m_workgroupLabel;
    KLineEdit *m_workgroupEdit;
    QLabel *m_ipAddressLabel;
    KLineEdit *m_ipAddressEdit;
    QPushButton *m_resetButton;
    QPushButton *m_editButton;
    QPushButton *m_addCategoryButton;
    QPushButton *m_removeButton;
    QPushButton *m_clearButton;
    bool m_bookmarksChanged;
    bool m_savingBookmarks;
};

#endif

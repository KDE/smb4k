/*
    This file contains private helper classes for the Smb4KMounter class.
    -------------------
    begin                : Do Jul 19 2007
    SPDX-FileCopyrightText: 2007-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KMOUNTER_P_H
#define SMB4KMOUNTER_P_H

// application specific includes
#include "smb4kglobal.h"
#include "smb4kmounter.h"

// Qt includes
#include <QCheckBox>
#include <QDebug>
#include <QDialog>
#include <QPointer>
#include <QPushButton>
#include <QString>

class Smb4KMountDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * The constructor.
     *
     * @param share       A pointer to an empty share
     *
     * @param bookmark    A pointer to an empty bookmark
     *
     * @param parent      The parent widget
     */
    explicit Smb4KMountDialog(const SharePtr &share, const BookmarkPtr &bookmark, QWidget *parent = 0);

    /**
     * The destructor.
     */
    ~Smb4KMountDialog();

    /**
     * Returns if the share should be bookmarked or not.
     *
     * @returns TRUE if the share should be bookmarked
     */
    bool bookmarkShare();

    /**
     * Returns if the user input is valid or not.
     *
     * @returns TRUE if the user input is valid.
     */
    bool validUserInput();

protected Q_SLOTS:
    /**
     * This slot is activated when the OK button has been clicked.
     */
    void slotOkClicked();

    /**
     * This slot is activated when the Cancel button has been clicked.
     */
    void slotCancelClicked();

    /**
     * This slot is activated when the 'Add Bookmark' button has been clicked.
     */
    void slotBookmarkButtonClicked();

    /**
     * This slot is being enabled if there is input text.
     *
     * @param text        The input text.
     */
    void slotChangeInputValue(const QString &text);

    /**
     * This slot is used for making text completion for the share edit line
     * work.
     */
    void slotShareNameEntered();

    /**
     * This slot is used for making text completion for the IP edit line
     * work.
     */
    void slotIpEntered();

    /**
     * This slot is used for making text completion for the workgroup edit
     * line work.
     */
    void slotWorkgroupEntered();

    /**
     * This slot is used for making text completion for the label edit line
     * work.
     */
    void slotLabelEntered();

    /**
     * This slot is used for making text completion for the category name
     * edit line work.
     */
    void slotCategoryEntered();

    /**
     * This slot is used for enabling / disabling the bookmark editor widgets
     */
    void slotAddBookmarkClicked(bool on);

private:
    /**
     * This function sets up the view.
     */
    void setupView();

    /**
     * Check that the user input is valid.
     */
    bool validUserInput(const QString &input);

    /**
     * Adjust the size
     */
    void adjustDialogSize();

    /**
     * The share that is passed to the mounter.
     */
    SharePtr m_share;

    /**
     * The bookmark that was passed to the dialog
     */
    BookmarkPtr m_bookmark;

    /**
     * Valid user input?
     */
    bool m_valid;

    /**
     * List of bookmark categories
     */
    QStringList m_categories;
};

class Smb4KMounterPrivate
{
public:
    int remountTimeout;
    int remountAttempts;
    int timerId;
    int checkTimeout;
    int newlyMounted;
    int newlyUnmounted;
    QPointer<Smb4KMountDialog> dialog;
    QList<SharePtr> importedShares;
    QList<SharePtr> retries;
    QList<SharePtr> remounts;
    QString activeProfile;
    bool detectAllShares;
    bool firstImportDone;
    bool longActionRunning;
};

class Smb4KMounterStatic
{
public:
    Smb4KMounter instance;
};

#endif

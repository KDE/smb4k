/*
    The configuration page for the authentication settings of Smb4K

    SPDX-FileCopyrightText: 2003-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGPAGEAUTHENTICATION_H
#define SMB4KCONFIGPAGEAUTHENTICATION_H

// Application specific includes
#include "core/smb4kauthinfo.h"

// Qt includes
#include <QList>
#include <QListWidget>
#include <QPushButton>
#include <QWidget>

// Forward declarations
class QListWidgetItem;

/**
 * This is the configuration tab for the authentication settings
 * of Smb4K.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KConfigPageAuthentication : public QWidget
{
    Q_OBJECT

public:
    /**
     * The constructor.
     *
     * @param parent          The parent widget
     */
    explicit Smb4KConfigPageAuthentication(QWidget *parent = nullptr);

    /**
     * The destructor.
     */
    virtual ~Smb4KConfigPageAuthentication();

    /**
     * Load the login credentials.
     */
    void loadLoginCredentials();

    /**
     * Save the login credentials.
     */
    void saveLoginCredentials();

    /**
     * Returns TRUE if the wallet entries are loaded and FALSE otherwise.
     *
     * @returns TRUE if the wallet entries are loaded
     */
    bool loginCredentialsLoaded();

    /**
     * This functions checks the entries in this widget against the ones in
     * the wallet and returns the result. If the login credentials were not
     * loaded, this function will always return FALSE.
     *
     * @returns TRUE if the wallet entries have changed and FALSE otherwise.
     */
    bool loginCredentialsChanged();

Q_SIGNALS:
    /**
     * This signal is emitted every time the wallet entries potentially were
     * modified by the user. When this signal is emitted, it does not necessarily
     * mean that any wallet entry indeed changed. It only means that the user
     * edited one.
     */
    void walletEntriesModified();

protected:
    /**
     * Reimplemented.
     */
    bool eventFilter(QObject *object, QEvent *event) override;

protected Q_SLOTS:
    /**
     * This slot is called when the "Use wallet" check box is toggled.
     *
     * @param checked       TRUE if the check box is checked and otherwise
     *                      FALSE.
     */
    void slotKWalletButtonToggled(bool checked);

    /**
     * This slot is invoked when the "Default login" check box is toggled.
     *
     * @param checked       TRUE if the check box is checked and otherwise
     *                      FALSE.
     */
    void slotDefaultLoginToggled(bool checked);

    /**
     * This slot is called when the wallet entries are to be loaded.
     *
     * @param checked         TRUE if the button is checked and FALSE otherwise.
     */
    void slotLoadButtonClicked(bool checked);

    /**
     * This slot is called when the list of all wallet entries is to be saved.
     *
     * @param checked         TRUE if the button is checked and FALSE otherwise.
     */
    void slotSaveButtonClicked(bool checked);

    /**
     * This slot is called when a wallet entry is to be edited.
     *
     * @param checked         TRUE if the button is checked and FALSE otherwise.
     */
    void slotEditButtonClicked(bool checked);

    /**
     * This slot is called when a wallet entry is to be removed.
     *
     * @param checked         TRUE if the button is checked and FALSE otherwise.
     */
    void slotRemoveButtonClicked(bool checked);

    /**
     * This slot is called when the list of all wallet entries is to be cleared.
     *
     * @param checked         TRUE if the button is checked and FALSE otherwise.
     */
    void slotClearButtonClicked(bool checked);

    /**
     * This slot is called when the actions performed on the custom options
     * are to be reset.
     *
     * @param checked         TRUE if the button is checked and FALSE otherwise.
     */
    void slotResetButtonClicked(bool checked);

    /**
     * This slot is called when the reset button is to be enabled/disabled.
     */
    void slotEnableResetButton();

    /**
     * This slot is called when an wallet item in the list view is double clicked.
     *
     * @param item          The item that was double clicked.
     */
    void slotWalletItemDoubleClicked(QListWidgetItem *item);

private:
    QWidget *m_walletEntriesEditor;
    QListWidget *m_walletEntriesWidget;
    QPushButton *m_loadButton;
    QPushButton *m_saveButton;
    QPushButton *m_editButton;
    QPushButton *m_removeButton;
    QPushButton *m_clearButton;
    QPushButton *m_resetButton;
    QList<Smb4KAuthInfo *> m_entriesList;
    bool m_entries_loaded;
};

#endif

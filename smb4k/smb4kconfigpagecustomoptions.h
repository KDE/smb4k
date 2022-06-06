/*
    The configuration page for the custom options

    SPDX-FileCopyrightText: 2013-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGPAGECUSTOMOPTIONS_H
#define SMB4KCONFIGPAGECUSTOMOPTIONS_H

// application specific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QListWidget>
#include <QWidget>

/**
 * This configuration page contains the custom options
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.1.0
 */

class Smb4KConfigPageCustomOptions : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KConfigPageCustomOptions(QWidget *parent = 0);

    /**
     * Destructor
     */
    virtual ~Smb4KConfigPageCustomOptions();

    /**
     * This function inserts a list of custom option items into the list widget.
     */
    void insertCustomOptions();

    /**
     * Returns TRUE if there may be changed custom settings. You must check if
     * this is indeed the case in you code.
     *
     * @returns TRUE if custom settings may have changed.
     */
    bool customSettingsMaybeChanged();

protected:
    /**
     * Reimplemented from QObject
     */
    bool eventFilter(QObject *obj, QEvent *e) override;

Q_SIGNALS:
    /**
     * This signal is emitted every time the custom settings potentially were
     * modified by the user. When this signal is emitted, it does not necessarily
     * mean that any custom setting changed. It only means that the user edited
     * one option.
     */
    void customSettingsModified();

protected Q_SLOTS:
    /**
     * This slot is invoked when an item is double clicked. It is used
     * to edit the item the user double clicked.
     *
     * @param item            The item that was double clicked.
     */
    void slotEditCustomItem(QListWidgetItem *item);

    /**
     * This slot is invoked when the custom context menu for the custom
     * options widget is requested.
     *
     * @param pos             The position where the context menu was requested.
     */
    void slotCustomContextMenuRequested(const QPoint &pos);

    /**
     * This slot is called when a custom options is to be edited.
     *
     * @param checked         TRUE if the action is checked and FALSE otherwise.
     */
    void slotEditActionTriggered(bool);

    /**
     * This slot is called when a custom option is to be removed.
     *
     * @param checked         TRUE if the action is checked and FALSE otherwise.
     */
    void slotRemoveActionTriggered(bool);

    /**
     * This slot is called when the list of custom options is to be cleared.
     *
     * @param checked         TRUE if the action is checked and FALSE otherwise.
     */
    void slotClearActionTriggered(bool);

    /**
     * This slot is called when the actions performed on the custom options
     * are to be reset.
     *
     * @param checked         TRUE if the action is checked and FALSE otherwise.
     */
    void slotResetActionTriggered(bool);

    /**
     * This slot is called when the reset button is to be enabled/disabled.
     */
    void slotEnableResetButton();

private:
    bool m_maybe_changed;
};

#endif

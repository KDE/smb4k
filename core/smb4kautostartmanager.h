/*
 *  Handles autostart capabilities for Smb4K
 *
 *  SPDX-FileCopyrightText: 2026 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SMB4KAUTOSTARTMANAGER_H
#define SMB4KAUTOSTARTMANAGER_H

// application specific includes
#include "smb4kcore_export.h"

// Qt includes
#include <QObject>
#include <QScopedPointer>

/**
 * This class adds or removes an autostart desktop file according to the
 * user's choice.
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 4.1.0
 */

class Smb4KAutoStartManagerPrivate;

class SMB4KCORE_EXPORT Smb4KAutoStartManager : public QObject
{
    Q_OBJECT

    friend class Smb4KAutoStartManagerPrivate;

public:
    /**
     * Constructor
     */
    explicit Smb4KAutoStartManager(QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~Smb4KAutoStartManager();

    /**
     * The static pointer to this class.
     *
     * @returns a static pointer to this class
     */
    static Smb4KAutoStartManager *self();

    /**
     * Initializes the autostart manager.
     */
    void init();

    /**
     * Enables or disables autostarting by adding a desktop file to the
     * autostart folder or by removing it.
     *
     * @param on        TRUE if autostart is to be enabled and FALSE otherwise
     */
    void enableAutoStart(bool on);

protected Q_SLOTS:
    /**
     * This slot is called when the autostart desktop file changed.
     *
     * @param path      The path of the file
     */
    void slotAutoStartFileChanged(const QString &path);

private:
    QScopedPointer<Smb4KAutoStartManagerPrivate> d;
};

#endif

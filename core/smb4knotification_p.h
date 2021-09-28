/*
    These are the private helper classes of the Smb4KNotification
    namespace.
    -------------------
    begin                : So Jun 22 2014
    SPDX-FileCopyrightText: 2014-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KNOTIFICATION_P_H
#define SMB4KNOTIFICATION_P_H

// Qt includes
#include <QObject>
#include <QUrl>

// KDE includes
#include <KNotifications/KNotification>

class Smb4KNotifier : public KNotification
{
    Q_OBJECT

public:
    Smb4KNotifier(const QString &event);
    ~Smb4KNotifier();
    void setMountpoint(const QUrl &mountpoint);
    QUrl mountpoint() const;

public Q_SLOTS:
    void slotOpenShare();

private:
    QUrl m_mountpoint;
};

#endif

/*
    smb4ktooltip  -  Provides tooltips for Smb4K

    SPDX-FileCopyrightText: 2020-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4ktooltip.h"
#include "smb4kbasicnetworkitem.h"
#include "smb4kglobal.h"
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4kworkgroup.h"

// Qt includes
#include <QApplication>
#include <QFormLayout>
#include <QLabel>
#include <QScreen>

// KDE includes
#include <KIconLoader>
#include <KLocalizedString>
#include <KSeparator>

using namespace Smb4KGlobal;

Smb4KToolTip::Smb4KToolTip(QWidget *parent)
    : KToolTipWidget(parent)
{
    m_contentsWidget = new QWidget(parent);
    m_type = Unknown;
}

Smb4KToolTip::~Smb4KToolTip()
{
}

void Smb4KToolTip::setupToolTip(Smb4KToolTip::Type type, NetworkItemPtr item)
{
    m_type = type;
    m_item = item;

    qDeleteAll(m_contentsWidget->children());

    m_mainLayout = new QHBoxLayout(m_contentsWidget);

    switch (m_type) {
    case NetworkItem: {
        setupNetworkItemContents();
        break;
    }
    case MountedShare: {
        setupMountedShareContents();
        break;
    }
    default: {
        break;
    }
    }
}

void Smb4KToolTip::update()
{
    if (!m_item.isNull() || m_type == Unknown) {
        return;
    }

    switch (m_type) {
    case NetworkItem: {
        setupNetworkItemContents();
        break;
    }
    case MountedShare: {
        setupMountedShareContents();
        break;
    }
    default: {
        break;
    }
    }
}

void Smb4KToolTip::show(const QPoint &pos, QWindow *transientParent)
{
    QPoint tooltipPos = pos;

    int testWidth = m_contentsWidget->width() + cursor().pos().x() + layout()->contentsMargins().left() + layout()->contentsMargins().right() + 5;
    int testHeight = m_contentsWidget->height() + cursor().pos().y() + layout()->contentsMargins().top() + layout()->contentsMargins().bottom() + 5;

    if (QApplication::screenAt(pos)->virtualSize().width() < testWidth) {
        tooltipPos.setX(pos.x() - m_contentsWidget->width() - layout()->contentsMargins().left() - layout()->contentsMargins().right() - 5);
    } else {
        tooltipPos.setX(pos.x() + 5);
    }

    if (QApplication::screenAt(pos)->virtualSize().height() < testHeight) {
        tooltipPos.setY(pos.y() - m_contentsWidget->height() - layout()->contentsMargins().top() - layout()->contentsMargins().bottom() - 5);
    } else {
        tooltipPos.setY(pos.y() + 5);
    }

    showAt(tooltipPos, m_contentsWidget, transientParent);
}

void Smb4KToolTip::setupNetworkItemContents()
{
    if (!m_contentsWidget->layout()->isEmpty()) {
        switch (m_item->type()) {
        case Workgroup: {
            WorkgroupPtr workgroup = m_item.staticCast<Smb4KWorkgroup>();
            QLabel *masterBrowserName = m_contentsWidget->findChild<QLabel *>(QStringLiteral("MasterBrowserName"));

            if (workgroup->hasMasterBrowser()) {
                if (workgroup->hasMasterBrowserIpAddress()) {
                    masterBrowserName->setText(workgroup->masterBrowserName() + QStringLiteral(" (") + workgroup->masterBrowserIpAddress()
                                               + QStringLiteral(")"));
                } else {
                    masterBrowserName->setText(workgroup->masterBrowserName());
                }
            } else {
                masterBrowserName->setText(QStringLiteral("-"));
            }
            break;
        }
        case Host: {
            HostPtr host = m_item.staticCast<Smb4KHost>();
            m_contentsWidget->findChild<QLabel *>(QStringLiteral("CommentString"))->setText(!host->comment().isEmpty() ? host->comment() : QStringLiteral("-"));
            m_contentsWidget->findChild<QLabel *>(QStringLiteral("IPAddressString"))->setText(host->hasIpAddress() ? host->ipAddress() : QStringLiteral("-"));
            break;
        }
        case Share: {
            SharePtr share = m_item.staticCast<Smb4KShare>();

            m_contentsWidget->findChild<QLabel *>(QStringLiteral("CommentString"))
                ->setText(!share->comment().isEmpty() ? share->comment() : QStringLiteral("-"));
            m_contentsWidget->findChild<QLabel *>(QStringLiteral("IPAddressString"))
                ->setText(share->hasHostIpAddress() ? share->hostIpAddress() : QStringLiteral("-"));

            QLabel *mountedState = m_contentsWidget->findChild<QLabel *>(QStringLiteral("MountedState"));

            if (!share->isPrinter()) {
                mountedState->setText(share->isMounted() ? i18n("yes") : i18n("no"));
            } else {
                mountedState->setText(QStringLiteral("-"));
            }
            break;
        }
        default: {
            break;
        }
        }

        return;
    }

    QLabel *iconLabel = new QLabel(m_contentsWidget);
    iconLabel->setPixmap(m_item->icon().pixmap(KIconLoader::SizeEnormous));
    m_mainLayout->addWidget(iconLabel, Qt::AlignHCenter);

    QFormLayout *descriptionLayout = new QFormLayout();
    m_mainLayout->addLayout(descriptionLayout);

    QFont captionFont = font();
    captionFont.setBold(true);

    QLabel *caption = new QLabel(m_contentsWidget);
    caption->setAlignment(Qt::AlignCenter);
    caption->setFont(captionFont);
    descriptionLayout->addRow(caption);

    KSeparator *separator = new KSeparator(Qt::Horizontal, m_contentsWidget);
    descriptionLayout->addRow(separator);

    QLabel *typeName = new QLabel(m_contentsWidget);
    descriptionLayout->addRow(i18n("Type:"), typeName);

    switch (m_item->type()) {
    case Workgroup: {
        WorkgroupPtr workgroup = m_item.staticCast<Smb4KWorkgroup>();

        caption->setText(workgroup->workgroupName());
        typeName->setText(i18n("Workgroup"));

        QLabel *masterBrowserName = new QLabel(m_contentsWidget);
        masterBrowserName->setObjectName(QStringLiteral("MasterBrowserName"));
        descriptionLayout->addRow(i18n("Master Browser:"), masterBrowserName);

        if (workgroup->hasMasterBrowser()) {
            if (workgroup->hasMasterBrowserIpAddress()) {
                masterBrowserName->setText(workgroup->masterBrowserName() + QStringLiteral(" (") + workgroup->masterBrowserIpAddress() + QStringLiteral(")"));
            } else {
                masterBrowserName->setText(workgroup->masterBrowserName());
            }
        } else {
            masterBrowserName->setText(QStringLiteral("-"));
        }

        break;
    }
    case Host: {
        HostPtr host = m_item.staticCast<Smb4KHost>();
        caption->setText(host->hostName());
        typeName->setText(i18n("Host"));

        QLabel *commentString = new QLabel(!host->comment().isEmpty() ? host->comment() : QStringLiteral("-"), m_contentsWidget);
        commentString->setObjectName(QStringLiteral("CommentString"));
        descriptionLayout->addRow(i18n("Comment:"), commentString);

        QLabel *ipAddress = new QLabel(host->hasIpAddress() ? host->ipAddress() : QStringLiteral("-"), m_contentsWidget);
        ipAddress->setObjectName(QStringLiteral("IPAddressString"));
        descriptionLayout->addRow(i18n("IP Address:"), ipAddress);

        QLabel *workgroupName = new QLabel(host->workgroupName(), m_contentsWidget);
        descriptionLayout->addRow(i18n("Workgroup:"), workgroupName);

        break;
    }
    case Share: {
        SharePtr share = m_item.staticCast<Smb4KShare>();
        caption->setText(share->shareName());
        typeName->setText(i18n("Share (%1)", share->shareTypeString()));

        QLabel *commentString = new QLabel(!share->comment().isEmpty() ? share->comment() : QStringLiteral("-"), m_contentsWidget);
        commentString->setObjectName(QStringLiteral("CommentString"));
        descriptionLayout->addRow(i18n("Comment:"), commentString);

        QLabel *mountedState = new QLabel(m_contentsWidget);
        mountedState->setObjectName(QStringLiteral("MountedState"));
        descriptionLayout->addRow(i18n("Mounted:"), mountedState);

        if (!share->isPrinter()) {
            mountedState->setText(share->isMounted() ? i18n("yes") : i18n("no"));
        } else {
            mountedState->setText(QStringLiteral("-"));
        }

        QLabel *hostName = new QLabel(share->hostName(), m_contentsWidget);
        descriptionLayout->addRow(i18n("Host:"), hostName);

        QLabel *ipAddressString = new QLabel(share->hasHostIpAddress() ? share->hostIpAddress() : QStringLiteral("-"), m_contentsWidget);
        ipAddressString->setObjectName(QStringLiteral("IPAddressString"));
        descriptionLayout->addRow(i18n("IP Address:"), ipAddressString);

        QLabel *locationString = new QLabel(share->displayString(), m_contentsWidget);
        descriptionLayout->addRow(i18n("Location:"), locationString);

        break;
    }
    default: {
        break;
    }
    }

    m_contentsWidget->adjustSize();
    m_contentsWidget->ensurePolished();
}

void Smb4KToolTip::setupMountedShareContents()
{
    SharePtr share = m_item.staticCast<Smb4KShare>();

    if (!m_contentsWidget->layout()->isEmpty()) {
        m_contentsWidget->findChild<QLabel *>(QStringLiteral("IconLabel"))->setPixmap(share->icon().pixmap(KIconLoader::SizeEnormous));
        m_contentsWidget->findChild<QLabel *>(QStringLiteral("LoginString"))->setText(!share->userName().isEmpty() ? share->userName() : i18n("unknown"));

        QString sizeIndication;

        if (share->totalDiskSpace() != 0 && share->freeDiskSpace() != 0) {
            sizeIndication = i18n("%1 free of %2 (%3 used)", share->freeDiskSpaceString(), share->totalDiskSpaceString(), share->diskUsageString());
        } else {
            sizeIndication = i18n("unknown");
        }

        m_contentsWidget->findChild<QLabel *>(QStringLiteral("SizeString"))->setText(sizeIndication);

        return;
    }

    QLabel *iconLabel = new QLabel(m_contentsWidget);
    iconLabel->setPixmap(share->icon().pixmap(KIconLoader::SizeEnormous));
    iconLabel->setObjectName(QStringLiteral("IconLabel"));
    m_mainLayout->addWidget(iconLabel, Qt::AlignHCenter);

    QFormLayout *descriptionLayout = new QFormLayout();
    m_mainLayout->addLayout(descriptionLayout);

    QFont captionFont = font();
    captionFont.setBold(true);

    QLabel *caption = new QLabel(share->shareName(), m_contentsWidget);
    caption->setAlignment(Qt::AlignCenter);
    caption->setFont(captionFont);
    descriptionLayout->addRow(caption);

    KSeparator *separator = new KSeparator(Qt::Horizontal, m_contentsWidget);
    descriptionLayout->addRow(separator);

    QLabel *locationString = new QLabel(share->displayString(), m_contentsWidget);
    descriptionLayout->addRow(i18n("Location:"), locationString);

    QLabel *mountpointString = new QLabel(share->path(), m_contentsWidget);
    descriptionLayout->addRow(i18n("Mountpoint:"), mountpointString);

    QLabel *loginString = new QLabel(!share->userName().isEmpty() ? share->userName() : i18n("unknown"), m_contentsWidget);
    loginString->setObjectName(QStringLiteral("LoginString"));
    descriptionLayout->addRow(i18n("Username:"), loginString);

    QString owner(!share->user().loginName().isEmpty() ? share->user().loginName() : i18n("unknown"));
    QString group(!share->group().name().isEmpty() ? share->group().name() : i18n("unknown"));

    QLabel *ownerString = new QLabel(owner + QStringLiteral(" - ") + group, m_contentsWidget);
    descriptionLayout->addRow(i18n("Owner:"), ownerString);

    QLabel *fileSystemString = new QLabel(share->fileSystemString(), m_contentsWidget);
    descriptionLayout->addRow(i18n("File system:"), fileSystemString);

    QString sizeIndication;

    if (share->totalDiskSpace() != 0 && share->freeDiskSpace() != 0) {
        sizeIndication = i18n("%1 free of %2 (%3 used)", share->freeDiskSpaceString(), share->totalDiskSpaceString(), share->diskUsageString());
    } else {
        sizeIndication = i18n("unknown");
    }

    QLabel *sizeString = new QLabel(sizeIndication, m_contentsWidget);
    sizeString->setObjectName(QStringLiteral("SizeString"));
    descriptionLayout->addRow(i18n("Size:"), sizeString);

    m_contentsWidget->adjustSize();
    m_contentsWidget->ensurePolished();
}

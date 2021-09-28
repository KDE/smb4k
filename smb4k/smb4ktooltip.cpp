/*
    smb4ktooltip  -  Provides tooltips for Smb4K
    -------------------
    begin                : Mi Mai 2020
    SPDX-FileCopyrightText: 2020-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
*/

/***************************************************************************
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

// application specific includes
#include "smb4ktooltip.h"
#include "smb4kbasicnetworkitem.h"
#include "smb4kglobal.h"
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4kworkgroup.h"

// Qt includes
#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QScreen>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>
#include <KWidgetsAddons/KSeparator>

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
    //
    // Copy the type
    //
    m_type = type;

    //
    // Copy the item
    //
    m_item = item;

    //
    // Clear the contents widget
    //
    qDeleteAll(m_contentsWidget->children());

    //
    // Set the layout
    //
    m_mainLayout = new QHBoxLayout(m_contentsWidget);

    //
    // Setup the contents widget
    //
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
    //
    // Return if there is no item
    //
    if (!m_item.isNull() || m_type == Unknown) {
        return;
    }

    //
    // Update the contents widget
    //
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
    //
    // Update the contents, if possible
    //
    if (!m_contentsWidget->layout()->isEmpty()) {
        switch (m_item->type()) {
        case Workgroup: {
            WorkgroupPtr workgroup = m_item.staticCast<Smb4KWorkgroup>();
            QLabel *masterBrowserName = m_contentsWidget->findChild<QLabel *>("MasterBrowserName");

            if (workgroup->hasMasterBrowser()) {
                if (workgroup->hasMasterBrowserIpAddress()) {
                    masterBrowserName->setText(workgroup->masterBrowserName() + " (" + workgroup->masterBrowserIpAddress() + ')');
                } else {
                    masterBrowserName->setText(workgroup->masterBrowserName());
                }
            } else {
                masterBrowserName->setText("-");
            }
            break;
        }
        case Host: {
            HostPtr host = m_item.staticCast<Smb4KHost>();
            m_contentsWidget->findChild<QLabel *>("CommentString")->setText(!host->comment().isEmpty() ? host->comment() : "-");
            m_contentsWidget->findChild<QLabel *>("IPAddressString")->setText(host->hasIpAddress() ? host->ipAddress() : "-");
            break;
        }
        case Share: {
            SharePtr share = m_item.staticCast<Smb4KShare>();

            m_contentsWidget->findChild<QLabel *>("CommentString")->setText(!share->comment().isEmpty() ? share->comment() : "-");
            m_contentsWidget->findChild<QLabel *>("IPAddressString")->setText(share->hasHostIpAddress() ? share->hostIpAddress() : "-");

            QLabel *mountedState = m_contentsWidget->findChild<QLabel *>("MountedState");

            if (!share->isPrinter()) {
                mountedState->setText(share->isMounted() ? i18n("yes") : i18n("no"));
            } else {
                mountedState->setText("-");
            }
            break;
        }
        default: {
            break;
        }
        }

        return;
    }

    //
    // Set up the widget
    //

    // Icon
    QLabel *iconLabel = new QLabel(m_contentsWidget);
    iconLabel->setPixmap(m_item->icon().pixmap(KIconLoader::SizeEnormous));
    m_mainLayout->addWidget(iconLabel, Qt::AlignHCenter);

    // Header
    QGridLayout *descriptionLayout = new QGridLayout();
    m_mainLayout->addLayout(descriptionLayout);

    QLabel *caption = new QLabel(m_contentsWidget);
    caption->setForegroundRole(QPalette::ToolTipText);
    caption->setBackgroundRole(QPalette::AlternateBase);

    QFont captionFont = caption->font();
    captionFont.setBold(true);
    caption->setFont(captionFont);

    descriptionLayout->addWidget(caption, 0, 0, 1, 2, Qt::AlignHCenter);

    KSeparator *separator = new KSeparator(Qt::Horizontal, m_contentsWidget);
    separator->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(separator, 1, 0, 1, 2);

    // Type
    QLabel *typeCaption = new QLabel(i18n("Type:"), m_contentsWidget);
    typeCaption->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(typeCaption, 2, 0, Qt::AlignRight);

    QLabel *typeName = new QLabel(m_contentsWidget);
    typeName->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(typeName, 2, 1);

    switch (m_item->type()) {
    case Workgroup: {
        WorkgroupPtr workgroup = m_item.staticCast<Smb4KWorkgroup>();

        caption->setText(workgroup->workgroupName());
        typeName->setText(i18n("Workgroup"));

        // Master browser
        QLabel *masterBrowserLabel = new QLabel(i18n("Master Browser:"), m_contentsWidget);
        masterBrowserLabel->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(masterBrowserLabel, 3, 0, Qt::AlignRight);

        QLabel *masterBrowserName = new QLabel(m_contentsWidget);
        masterBrowserName->setObjectName("MasterBrowserName");
        masterBrowserName->setForegroundRole(QPalette::ToolTipText);

        if (workgroup->hasMasterBrowser()) {
            if (workgroup->hasMasterBrowserIpAddress()) {
                masterBrowserName->setText(workgroup->masterBrowserName() + " (" + workgroup->masterBrowserIpAddress() + ')');
            } else {
                masterBrowserName->setText(workgroup->masterBrowserName());
            }
        } else {
            masterBrowserName->setText("-");
        }

        descriptionLayout->addWidget(masterBrowserName, 3, 1);

        descriptionLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 4, 0, 2, 1);

        break;
    }
    case Host: {
        HostPtr host = m_item.staticCast<Smb4KHost>();
        caption->setText(host->hostName());
        typeName->setText(i18n("Host"));

        // Comment
        QLabel *commentLabel = new QLabel(i18n("Comment:"), m_contentsWidget);
        commentLabel->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(commentLabel, 3, 0, Qt::AlignRight);

        QLabel *commentString = new QLabel(!host->comment().isEmpty() ? host->comment() : "-", m_contentsWidget);
        commentString->setObjectName("CommentString");
        commentString->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(commentString, 3, 1);

        // IP address
        QLabel *ipAddressLabel = new QLabel(i18n("IP Address:"), m_contentsWidget);
        ipAddressLabel->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(ipAddressLabel, 4, 0, Qt::AlignRight);

        QLabel *ipAddress = new QLabel(host->hasIpAddress() ? host->ipAddress() : "-", m_contentsWidget);
        ipAddress->setObjectName("IPAddressString");
        ipAddress->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(ipAddress, 4, 1);

        // Workgroup
        QLabel *workgroupLabel = new QLabel(i18n("Workgroup:"), m_contentsWidget);
        workgroupLabel->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(workgroupLabel, 5, 0, Qt::AlignRight);

        QLabel *workgroupName = new QLabel(host->workgroupName(), m_contentsWidget);
        workgroupName->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(workgroupName, 5, 1);

        descriptionLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 6, 0, 2, 1);

        break;
    }
    case Share: {
        SharePtr share = m_item.staticCast<Smb4KShare>();
        caption->setText(share->shareName());
        typeName->setText(i18n("Share (%1)", share->shareTypeString()));

        // Comment
        QLabel *commentLabel = new QLabel(i18n("Comment:"), m_contentsWidget);
        commentLabel->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(commentLabel, 3, 0, Qt::AlignRight);

        QLabel *commentString = new QLabel(!share->comment().isEmpty() ? share->comment() : "-", m_contentsWidget);
        commentString->setObjectName("CommentString");
        commentString->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(commentString, 3, 1);

        // State (mounted/not mounted)
        QLabel *mountedLabel = new QLabel(i18n("Mounted:"), m_contentsWidget);
        mountedLabel->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(mountedLabel, 4, 0, Qt::AlignRight);

        QLabel *mountedState = nullptr;

        if (!share->isPrinter()) {
            mountedState = new QLabel(share->isMounted() ? i18n("yes") : i18n("no"), m_contentsWidget);
        } else {
            mountedState = new QLabel("-", m_contentsWidget);
        }

        mountedState->setObjectName("MountedState");
        mountedState->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(mountedState, 4, 1);

        // Host
        QLabel *hostLabel = new QLabel(i18n("Host:"), m_contentsWidget);
        hostLabel->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(hostLabel, 5, 0, Qt::AlignRight);

        QLabel *hostName = new QLabel(share->hostName(), m_contentsWidget);
        hostName->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(hostName, 5, 1);

        // IP address
        QLabel *ipAddressLabel = new QLabel(i18n("IP Address:"), m_contentsWidget);
        ipAddressLabel->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(ipAddressLabel, 6, 0, Qt::AlignRight);

        QLabel *ipAddressString = new QLabel(share->hasHostIpAddress() ? share->hostIpAddress() : "-", m_contentsWidget);
        ipAddressString->setObjectName("IPAddressString");
        ipAddressString->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(ipAddressString, 6, 1);

        // Location
        QLabel *locationLabel = new QLabel(i18n("Location:"), m_contentsWidget);
        locationLabel->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(locationLabel, 7, 0, Qt::AlignRight);

        QLabel *locationString = new QLabel(share->displayString(), m_contentsWidget);
        locationString->setForegroundRole(QPalette::ToolTipText);

        descriptionLayout->addWidget(locationString, 7, 1);

        descriptionLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 8, 0, 2, 1);

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
    //
    // Cast the item
    //
    SharePtr share = m_item.staticCast<Smb4KShare>();

    //
    // Update the contents, if possible
    //
    if (!m_contentsWidget->layout()->isEmpty()) {
        m_contentsWidget->findChild<QLabel *>("IconLabel")->setPixmap(share->icon().pixmap(KIconLoader::SizeEnormous));
        m_contentsWidget->findChild<QLabel *>("LoginString")->setText(!share->login().isEmpty() ? share->login() : i18n("unknown"));

        QString sizeIndication;

        if (share->totalDiskSpace() != 0 && share->freeDiskSpace() != 0) {
            sizeIndication = i18n("%1 free of %2 (%3 used)", share->freeDiskSpaceString(), share->totalDiskSpaceString(), share->diskUsageString());
        } else {
            sizeIndication = i18n("unknown");
        }

        m_contentsWidget->findChild<QLabel *>("SizeString")->setText(sizeIndication);

        return;
    }

    //
    // Set up the widget
    //

    // Icon
    QLabel *iconLabel = new QLabel(m_contentsWidget);
    iconLabel->setPixmap(share->icon().pixmap(KIconLoader::SizeEnormous));
    iconLabel->setObjectName("IconLabel");
    m_mainLayout->addWidget(iconLabel, Qt::AlignHCenter);

    // Header
    QGridLayout *descriptionLayout = new QGridLayout();
    m_mainLayout->addLayout(descriptionLayout);

    QLabel *caption = new QLabel(share->shareName(), m_contentsWidget);
    caption->setForegroundRole(QPalette::ToolTipText);
    caption->setBackgroundRole(QPalette::AlternateBase);

    QFont captionFont = caption->font();
    captionFont.setBold(true);
    caption->setFont(captionFont);

    descriptionLayout->addWidget(caption, 0, 0, 1, 2, Qt::AlignHCenter);

    KSeparator *separator = new KSeparator(Qt::Horizontal, m_contentsWidget);
    separator->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(separator, 1, 0, 1, 2);

    // Location
    QLabel *locationLabel = new QLabel(i18n("Location:"), m_contentsWidget);
    locationLabel->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(locationLabel, 2, 0, Qt::AlignRight);

    QLabel *locationString = new QLabel(share->displayString(), m_contentsWidget);
    locationString->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(locationString, 2, 1);

    // Mount point
    QLabel *mountpointLabel = new QLabel(i18n("Mountpoint:"), m_contentsWidget);
    mountpointLabel->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(mountpointLabel, 3, 0, Qt::AlignRight);

    QLabel *mountpointString = new QLabel(share->path(), m_contentsWidget);
    mountpointString->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(mountpointString, 3, 1);

    // Login
    QLabel *loginLabel = new QLabel(i18n("Login:"), m_contentsWidget);
    loginLabel->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(loginLabel, 4, 0, Qt::AlignRight);

    QLabel *loginString = new QLabel(!share->login().isEmpty() ? share->login() : i18n("unknown"), m_contentsWidget);
    loginString->setObjectName("LoginString");
    loginString->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(loginString, 4, 1);

    // Owner
    QLabel *ownerLabel = new QLabel(i18n("Owner:"), m_contentsWidget);
    ownerLabel->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(ownerLabel, 5, 0, Qt::AlignRight);

    QString owner(!share->user().loginName().isEmpty() ? share->user().loginName() : i18n("unknown"));
    QString group(!share->group().name().isEmpty() ? share->group().name() : i18n("unknown"));

    QLabel *ownerString = new QLabel(QString("%1 - %2").arg(owner, group), m_contentsWidget);
    ownerString->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(ownerString, 5, 1);

    // File system
    QLabel *fileSystemLabel = new QLabel(i18n("File system:"), m_contentsWidget);
    fileSystemLabel->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(fileSystemLabel, 6, 0, Qt::AlignRight);

    QLabel *fileSystemString = new QLabel(share->fileSystemString(), m_contentsWidget);
    fileSystemString->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(fileSystemString, 6, 1);

    // Size
    QLabel *sizeLabel = new QLabel(i18n("Size:"), m_contentsWidget);
    sizeLabel->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(sizeLabel, 7, 0, Qt::AlignRight);

    QString sizeIndication;

    if (share->totalDiskSpace() != 0 && share->freeDiskSpace() != 0) {
        sizeIndication = i18n("%1 free of %2 (%3 used)", share->freeDiskSpaceString(), share->totalDiskSpaceString(), share->diskUsageString());
    } else {
        sizeIndication = i18n("unknown");
    }

    QLabel *sizeString = new QLabel(sizeIndication, m_contentsWidget);
    sizeString->setObjectName("SizeString");
    sizeString->setForegroundRole(QPalette::ToolTipText);

    descriptionLayout->addWidget(sizeString, 7, 1);

    descriptionLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum), 8, 0, 2, 1);

    m_contentsWidget->adjustSize();
    m_contentsWidget->ensurePolished();
}

/*
 *  Preview dialog
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

// application specific includes
#include "smb4kpreviewdialog.h"
#include "core/smb4kclient.h"
#include "core/smb4khomesshareshandler.h"
#include "core/smb4ksettings.h"

// Qt includes
#include <QDialogButtonBox>
#include <QMap>
#include <QVBoxLayout>

// KDE includes
#include <KConfigGroup>
#include <KLocalizedString>
#include <KWindowConfig>
#include <QToolBar>
#include <QWindow>
// #include <KIO/OpenUrlJob>

Smb4KPreviewDialog::Smb4KPreviewDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Preview Dialog"));
    setAttribute(Qt::WA_DeleteOnClose, true);

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_listWidget = new QListWidget(this);
    m_listWidget->setSelectionMode(QListWidget::SingleSelection);
    connect(m_listWidget, &QListWidget::itemActivated, this, &Smb4KPreviewDialog::slotItemActivated);

    layout->addWidget(m_listWidget);

    QToolBar *toolBar = new QToolBar(this);
    toolBar->setToolButtonStyle(Qt::ToolButtonFollowStyle);
    toolBar->setProperty("otherToolbar", true);

    m_reloadAction = new KDualAction(toolBar);
    m_reloadAction->setObjectName(QStringLiteral("reload_action"));
    m_reloadAction->setInactiveText(i18n("Reload"));
    m_reloadAction->setInactiveIcon(KDE::icon(QStringLiteral("view-refresh")));
    m_reloadAction->setActiveText(i18n("Abort"));
    m_reloadAction->setActiveIcon(KDE::icon(QStringLiteral("process-stop")));
    m_reloadAction->setActive(false);
    m_reloadAction->setAutoToggle(false);
    connect(m_reloadAction, &KDualAction::triggered, this, &Smb4KPreviewDialog::slotReloadActionTriggered);

    toolBar->addAction(m_reloadAction);

    m_upAction = toolBar->addAction(KDE::icon(QStringLiteral("go-up")), i18n("Up"), this, &Smb4KPreviewDialog::slotUpActionTriggered);
    m_upAction->setObjectName(QStringLiteral("up_action"));
    m_upAction->setEnabled(false);

    toolBar->addSeparator();

    m_urlComboBox = new KUrlComboBox(KUrlComboBox::Directories, toolBar);
    m_urlComboBox->setEditable(false);
    connect(m_urlComboBox, &KUrlComboBox::urlActivated, this, &Smb4KPreviewDialog::slotUrlActivated);

    toolBar->addWidget(m_urlComboBox);

    layout->addWidget(toolBar);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    m_closeButton = buttonBox->addButton(QDialogButtonBox::Close);
    m_closeButton->setShortcut(QKeySequence::Close);
    m_closeButton->setDefault(true);
    connect(m_closeButton, &QPushButton::clicked, this, &Smb4KPreviewDialog::slotCloseButtonClicked);

    layout->addWidget(buttonBox);

    setMinimumWidth(sizeHint().width() > 350 ? sizeHint().width() : 350);

    create();

    KConfigGroup dialogGroup(Smb4KSettings::self()->config(), QStringLiteral("PreviewDialog"));
    QSize dialogSize;

    if (dialogGroup.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), dialogGroup);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584

    connect(Smb4KClient::self(), &Smb4KClient::files, this, &Smb4KPreviewDialog::slotPreviewResults);
    connect(Smb4KClient::self(), &Smb4KClient::aboutToStart, this, &Smb4KPreviewDialog::slotAdjustReloadAction);
    connect(Smb4KClient::self(), &Smb4KClient::finished, this, &Smb4KPreviewDialog::slotAdjustReloadAction);
}

Smb4KPreviewDialog::~Smb4KPreviewDialog()
{
}

bool Smb4KPreviewDialog::setShare(SharePtr share)
{
    if (share->isPrinter()) {
        return false;
    }

    if (share->isHomesShare()) {
        if (!Smb4KHomesSharesHandler::self()->specifyUser(share, true)) {
            return false;
        }
    }

    m_share = share;

    setWindowTitle(i18n("Preview of %1", m_share->displayString()));

    loadPreview(m_share);

    return true;
}

void Smb4KPreviewDialog::loadPreview(const NetworkItemPtr &networkItem)
{
    QStringList urls = m_urlComboBox->urls();

    if (!urls.contains(networkItem->url().toDisplayString())) {
        urls << networkItem->url().toDisplayString();
    }

    m_urlComboBox->setUrls(urls);
    m_urlComboBox->setUrl(networkItem->url());

    m_currentItem = networkItem;

    Smb4KClient::self()->lookupFiles(networkItem);
}

void Smb4KPreviewDialog::slotCloseButtonClicked()
{
    KConfigGroup dialogGroup(Smb4KSettings::self()->config(), QStringLiteral("PreviewDialog"));
    KWindowConfig::saveWindowSize(windowHandle(), dialogGroup);

    accept();
}

void Smb4KPreviewDialog::slotItemActivated(QListWidgetItem *item)
{
    Smb4KFile file = item->data(Qt::UserRole).value<Smb4KFile>();

    if (file.isDirectory()) {
        FilePtr fileItem = FilePtr(new Smb4KFile(file));
        loadPreview(fileItem);
    } else {
        // KIO::OpenUrlJob *job = new KIO::OpenUrlJob(file.url());
        // job->setFollowRedirections(false);
        // job->setAutoDelete(true);
        // job->start();
    }
}

void Smb4KPreviewDialog::slotPreviewResults(const QList<FilePtr> &files)
{
    if (!files.first()->url().toString().startsWith(m_share->url().toString())) {
        return;
    }

    if (m_listWidget->count() != 0) {
        m_listWidget->clear();
    }

    QMap<QString, QListWidgetItem *> sortingMap;

    for (const FilePtr &file : files) {
        QVariant variant = QVariant::fromValue(*file.data());

        QListWidgetItem *item = new QListWidgetItem();
        item->setText(file->name());
        item->setIcon(file->icon());
        item->setData(Qt::UserRole, variant);

        if (file->isDirectory()) {
            sortingMap[QStringLiteral("00_") + file->name()] = item;
        } else {
            sortingMap[QStringLiteral("01_") + file->name()] = item;
        }
    }

    QMapIterator<QString, QListWidgetItem *> it(sortingMap);

    while (it.hasNext()) {
        it.next();
        m_listWidget->addItem(it.value());
    }

    m_upAction->setEnabled(!m_currentItem->url().matches(m_share->url(), QUrl::StripTrailingSlash));
}

void Smb4KPreviewDialog::slotReloadActionTriggered(bool checked)
{
    Q_UNUSED(checked);

    if (!m_reloadAction->isActive()) {
        FilePtr file = FilePtr(new Smb4KFile(QUrl(m_urlComboBox->currentText())));
        file->setUserName(m_share->userName());
        file->setPassword(m_share->password());
        file->setDirectory(true);

        loadPreview(file);
    } else {
        Smb4KClient::self()->abort();
    }
}

void Smb4KPreviewDialog::slotUpActionTriggered()
{
    if (!m_currentItem->url().matches(m_share->url(), QUrl::StripTrailingSlash)) {
        QUrl url = m_currentItem->url().adjusted(QUrl::StripTrailingSlash); // Do not merge with the line below (See code for KIO::upUrl())
        url = url.adjusted(QUrl::RemoveFilename);

        FilePtr file = FilePtr(new Smb4KFile(url));
        file->setUserName(m_share->userName());
        file->setPassword(m_share->password());
        file->setDirectory(true);

        loadPreview(file);
    }
}

void Smb4KPreviewDialog::slotUrlActivated(const QUrl &url)
{
    Q_UNUSED(url);

    NetworkItemPtr networkItem;

    if (url.matches(m_share->url(), QUrl::RemoveUserInfo | QUrl::StripTrailingSlash)) {
        networkItem = m_share;
    } else {
        FilePtr file = FilePtr(new Smb4KFile(url));
        file->setUserName(m_share->userName());
        file->setPassword(m_share->password());
        file->setDirectory(true);

        networkItem = file;
    }

    loadPreview(networkItem);
}

void Smb4KPreviewDialog::slotAdjustReloadAction(const NetworkItemPtr &item, int type)
{
    if (m_currentItem->url().matches(item->url(), QUrl::StripTrailingSlash) && type == LookupFiles) {
        m_reloadAction->setActive(!m_reloadAction->isActive());
    }
}

/*
    This file contains private helper classes for the Smb4KSynchronizer
    class.

    SPDX-FileCopyrightText: 2008-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4ksynchronizer_p.h"
#include "smb4kglobal.h"
#include "smb4knotification.h"
#include "smb4ksettings.h"
#include "smb4kshare.h"

// Qt includes
#include <QApplication>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLocale>
#include <QPointer>
#include <QStandardPaths>
#include <QTimer>
#include <QWindow>

// KDE includes
#include <KCompletion/KLineEdit>
#include <KConfigGui/KWindowConfig>
#include <KI18n/KLocalizedString>
#include <KIOWidgets/KUrlCompletion>
#include <KIconThemes/KIconLoader>

using namespace Smb4KGlobal;

Smb4KSyncJob::Smb4KSyncJob(QObject *parent)
    : KJob(parent)
    , m_share(nullptr)
    , m_process(nullptr)
{
    setCapabilities(KJob::Killable);

    m_terminated = false;
    m_job_tracker = new KUiServerJobTracker(this);
}

Smb4KSyncJob::~Smb4KSyncJob()
{
}

void Smb4KSyncJob::start()
{
    QTimer::singleShot(0, this, SLOT(slotStartSynchronization()));
}

void Smb4KSyncJob::setupSynchronization(const SharePtr &share)
{
    if (share) {
        m_share = share;
    }
}

bool Smb4KSyncJob::doKill()
{
    if (m_process && m_process->state() != KProcess::NotRunning) {
        m_process->terminate();
        m_terminated = true;
    }

    return KJob::doKill();
}

void Smb4KSyncJob::slotStartSynchronization()
{
    //
    // Find the shell command
    //
    QString rsync = QStandardPaths::findExecutable(QStringLiteral("rsync"));

    if (rsync.isEmpty()) {
        Smb4KNotification::commandNotFound(QStringLiteral("rsync"));
        emitResult();
        return;
    } else {
        // Go ahead
    }

    //
    // The synchronization dialog
    //
    if (m_share) {
        // Show the user an URL input dialog.
        QPointer<Smb4KSynchronizationDialog> dlg = new Smb4KSynchronizationDialog(m_share, QApplication::activeWindow());

        if (dlg->exec() == QDialog::Accepted) {
            // Create the destination directory if it does not already exits.
            QDir syncDir(dlg->destination().path());

            if (!syncDir.exists()) {
                if (!QDir().mkpath(syncDir.path())) {
                    Smb4KNotification::mkdirFailed(syncDir);
                    emitResult();
                    return;
                }
            }

            // Make sure that we have got the trailing slash present.
            // rsync is very picky regarding it.
            m_sourceUrl = dlg->source();
            m_destinationUrl = dlg->destination();

            delete dlg;
        } else {
            delete dlg;
            emitResult();
            return;
        }
    } else {
        emitResult();
        return;
    }

    //
    // The command
    //
    QStringList command;
    command << rsync;
    command << QStringLiteral("--progress");
    command << QStringLiteral("--info=progress2");

    //
    // Basic settings
    //
    if (Smb4KSettings::archiveMode()) {
        command << QStringLiteral("--archive");
    }

    if (Smb4KSettings::recurseIntoDirectories()) {
        command << QStringLiteral("--recursive");
    }

    if (Smb4KSettings::relativePathNames()) {
        command << QStringLiteral("--relative");
    }

    if (Smb4KSettings::noImpliedDirectories()) {
        command << QStringLiteral("--no-implied-dirs");
    }

    if (Smb4KSettings::transferDirectories()) {
        command << QStringLiteral("--dirs");
    }

    if (Smb4KSettings::makeBackups()) {
        command << QStringLiteral("--backup");

        if (Smb4KSettings::useBackupDirectory()) {
            command << QStringLiteral("--backup-dir=") + Smb4KSettings::backupDirectory().path();
        }

        if (Smb4KSettings::useBackupSuffix()) {
            command << QStringLiteral("--suffix=") + Smb4KSettings::backupSuffix();
        }
    }

    //
    // File handling
    //
    if (Smb4KSettings::updateTarget()) {
        command << QStringLiteral("--update");
    }

    if (Smb4KSettings::updateInPlace()) {
        command << QStringLiteral("--inplace");
    }

    if (Smb4KSettings::efficientSparseFileHandling()) {
        command << QStringLiteral("--sparse");
    }

    if (Smb4KSettings::copyFilesWhole()) {
        command << QStringLiteral("--whole-file");
    }

    if (Smb4KSettings::updateExisting()) {
        command << QStringLiteral("--existing");
    }

    if (Smb4KSettings::ignoreExisting()) {
        command << QStringLiteral("--ignore-existing");
    }

    if (Smb4KSettings::preserveSymlinks()) {
        command << QStringLiteral("--links");
    }

    if (Smb4KSettings::transformSymlinks()) {
        command << QStringLiteral("--copy-links");
    }

    if (Smb4KSettings::transformUnsafeSymlinks()) {
        command << QStringLiteral("--copy-unsafe-links");
    }

    if (Smb4KSettings::ignoreUnsafeSymlinks()) {
        command << QStringLiteral("--safe-links");
    }

    if (Smb4KSettings::mungeSymlinks()) {
        command << QStringLiteral("--munge-links");
    }

    if (Smb4KSettings::preserveHardLinks()) {
        command << QStringLiteral("--hard-links");
    }

    if (Smb4KSettings::copyDirectorySymlinks()) {
        command << QStringLiteral("--copy-dirlinks");
    }

    if (Smb4KSettings::keepDirectorySymlinks()) {
        command << QStringLiteral("--keep-dirlinks");
    }

    if (Smb4KSettings::preservePermissions()) {
        command << QStringLiteral("--perms");
    }

    if (Smb4KSettings::preserveACLs()) {
        command << QStringLiteral("--acls");
    }

    if (Smb4KSettings::preserveExtendedAttributes()) {
        command << QStringLiteral("--xattrs");
    }

    if (Smb4KSettings::preserveAccessTimes()) {
        command << QStringLiteral("--atimes");
    }

    if (Smb4KSettings::preserveCreateTimes()) {
        command << QStringLiteral("--crtimes");
    }

    if (Smb4KSettings::preserveOwner()) {
        command << QStringLiteral("--owner");
    }

    if (Smb4KSettings::preserveGroup()) {
        command << QStringLiteral("--group");
    }

    if (Smb4KSettings::preserveDevicesAndSpecials()) {
        // Alias -D
        command << QStringLiteral("--devices");
        command << QStringLiteral("--specials");
    }

    if (Smb4KSettings::preserveTimes()) {
        command << QStringLiteral("--times");
    }

    if (Smb4KSettings::omitDirectoryTimes()) {
        command << QStringLiteral("--omit-dir-times");
    }

    //
    // File transfer
    //
    if (Smb4KSettings::compressData()) {
        command << QStringLiteral("--compress");

        if (Smb4KSettings::useCompressionLevel()) {
            command << QStringLiteral("--compress-level=") + QString::number(Smb4KSettings::compressionLevel());
        }

        if (Smb4KSettings::useSkipCompression()) {
            command << QStringLiteral("--skip-compress=") + Smb4KSettings::skipCompression();
        }
    }

    if (Smb4KSettings::self()->useMaximalTransferSize()) {
        command << QStringLiteral("--max-size=") + QString::number(Smb4KSettings::self()->maximalTransferSize()) + QStringLiteral("kB");
    }

    if (Smb4KSettings::self()->useMinimalTransferSize()) {
        command << QStringLiteral("--min-size=") + QString::number(Smb4KSettings::self()->minimalTransferSize()) + QStringLiteral("kB");
    }

    if (Smb4KSettings::keepPartial()) {
        command << QStringLiteral(" --partial");

        if (Smb4KSettings::usePartialDirectory()) {
            command << QStringLiteral("--partial-dir=") + Smb4KSettings::partialDirectory().path();
        }
    }

    if (Smb4KSettings::useBandwidthLimit()) {
        command << QStringLiteral("--bwlimit=") + QString::number(Smb4KSettings::bandwidthLimit()) + QStringLiteral("kB");
    }

    //
    // File deletion
    //
    if (Smb4KSettings::removeSourceFiles()) {
        command << QStringLiteral("--remove-source-files");
    }

    if (Smb4KSettings::deleteExtraneous()) {
        command << QStringLiteral("--delete");
    }

    if (Smb4KSettings::deleteBefore()) {
        command << QStringLiteral("--delete-before");
    }

    if (Smb4KSettings::deleteDuring()) {
        command << QStringLiteral("--delete-during");
    }

    if (Smb4KSettings::deleteAfter()) {
        command << QStringLiteral("--delete-after");
    }

    if (Smb4KSettings::deleteExcluded()) {
        command << QStringLiteral("--delete-excluded");
    }

    if (Smb4KSettings::ignoreErrors()) {
        command << QStringLiteral("--ignore-errors");
    }

    if (Smb4KSettings::forceDirectoryDeletion()) {
        command << QStringLiteral("--force");
    }

    if (Smb4KSettings::useMaximumDelete()) {
        command << QStringLiteral("--max-delete=") + QString::number(Smb4KSettings::maximumDeleteValue());
    }

    //
    // Filtering
    //
    if (Smb4KSettings::useCVSExclude()) {
        command << QStringLiteral("--cvs-exclude");
    }

    if (Smb4KSettings::useExcludePattern()) {
        command << QStringLiteral("--exclude=") + Smb4KSettings::excludePattern();
    }

    if (Smb4KSettings::useExcludeFrom()) {
        command << QStringLiteral("--exclude-from=") + Smb4KSettings::excludeFrom().path();
    }

    if (Smb4KSettings::useIncludePattern()) {
        command << QStringLiteral("--include=") + Smb4KSettings::includePattern();
    }

    if (Smb4KSettings::useIncludeFrom()) {
        command << QStringLiteral("--include-from=") + Smb4KSettings::includeFrom().path();
    }

    if (Smb4KSettings::useCustomFilteringRules()) {
        if (!Smb4KSettings::customFilteringRules().isEmpty()) {
            qDebug() << "Do we need to spilt the filtering rules into a list?";
            command << Smb4KSettings::customFilteringRules();
        }
    }

    if (Smb4KSettings::useFFilterRule()) {
        command << QStringLiteral("-F");
    }

    if (Smb4KSettings::useFFFilterRule()) {
        command << QStringLiteral("-F");
        command << QStringLiteral("-F");
    }

    //
    // Miscellaneous
    //
    if (Smb4KSettings::useBlockSize()) {
        command << QStringLiteral("--block-size=") + QString::number(Smb4KSettings::blockSize());
    }

    if (Smb4KSettings::useChecksumSeed()) {
        command << QStringLiteral("--checksum-seed=") + QString::number(Smb4KSettings::checksumSeed());
    }

    if (Smb4KSettings::useChecksum()) {
        command << QStringLiteral("--checksum");
    }

    if (Smb4KSettings::oneFileSystem()) {
        command << QStringLiteral("--one-file-system");
    }

    if (Smb4KSettings::delayUpdates()) {
        command << QStringLiteral("--delay-updates");
    }

    // Make sure that the trailing slash is present. rsync is very
    // picky regarding it.
    QString source = m_sourceUrl.path() + (!m_sourceUrl.path().endsWith(QStringLiteral("/")) ? QStringLiteral("/") : QStringLiteral(""));
    QString destination = m_destinationUrl.path() + (!m_destinationUrl.path().endsWith(QStringLiteral("/")) ? QStringLiteral("/") : QStringLiteral(""));

    command << source;
    command << destination;

    //
    // The job tracker
    //
    m_job_tracker->registerJob(this);
    connect(this, SIGNAL(result(KJob *)), m_job_tracker, SLOT(unregisterJob(KJob *)));

    //
    // The process
    //
    m_process = new KProcess(this);
    m_process->setOutputChannelMode(KProcess::SeparateChannels);
    m_process->setProgram(command);

    connect(m_process, SIGNAL(readyReadStandardOutput()), SLOT(slotReadStandardOutput()));
    connect(m_process, SIGNAL(readyReadStandardError()), SLOT(slotReadStandardError()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(slotProcessFinished(int, QProcess::ExitStatus)));

    // Start the synchronization process
    Q_EMIT aboutToStart(m_destinationUrl.path());

    // Send description to the GUI
    Q_EMIT description(this, i18n("Synchronizing"), qMakePair(i18n("Source"), source), qMakePair(i18n("Destination"), destination));

    // Dummy to show 0 %
    emitPercent(0, 100);

    m_terminated = false;
    m_process->start();
}

void Smb4KSyncJob::slotReadStandardOutput()
{
    QStringList stdOut = QString::fromUtf8(m_process->readAllStandardOutput()).split(QStringLiteral("\r"), Qt::SkipEmptyParts);

    for (const QString &line : stdOut) {
        if (line.contains(QStringLiteral("%"))) {
            bool success = true;
            QString transferInfo = line.trimmed().simplified();

            // Overall progress
            QString progressString = transferInfo.section(QStringLiteral(" "), 1, 1).replace(QStringLiteral("%"), QStringLiteral(""));

            if (!progressString.isEmpty()) {
                qulonglong progress = progressString.toLongLong(&success);

                if (success) {
                    setPercent(progress);
                }
            }

            // Speed
            QString speedString = transferInfo.section(QStringLiteral(" "), 2, 2).section(QRegExp(QStringLiteral("../s")), 0, 0);

            if (!speedString.isEmpty()) {
                QLocale locale;
                double speed = locale.toDouble(speedString, &success);

                if (success) {
                    // MB == 1000000 B and kB == 1000 B per definition!
                    if (transferInfo.contains(QStringLiteral("MB/s"))) {
                        speed *= 1e6;
                    } else if (transferInfo.contains(QStringLiteral("kB/s"))) {
                        speed *= 1e3;
                    }

                    emitSpeed((ulong)speed);
                }
            }

            // Transfered files
            QString transferedFilesString = transferInfo.section(QStringLiteral("xfr#"), 1, 1).section(QStringLiteral(","), 0, 0);

            if (!transferedFilesString.isEmpty()) {
                qulonglong transferedFiles = transferedFilesString.toULongLong(&success);

                if (success) {
                    setProcessedAmount(KJob::Files, transferedFiles);
                }
            }

            // Total amount of files
            QString totalFilesString = transferInfo.section(QStringLiteral("/"), -1, -1).section(QStringLiteral(")"), 0, 0);

            if (!totalFilesString.isEmpty()) {
                qulonglong totalFiles = totalFilesString.toULongLong(&success);

                if (success) {
                    setTotalAmount(KJob::Files, totalFiles);
                }
            }

        } else if (!line.contains(QStringLiteral("sending incremental file list"))) {
            QString file = line.trimmed().remove(QStringLiteral("\n")).section(QStringLiteral("/"), -1, -1);

            QUrl sourceUrl = m_sourceUrl;
            sourceUrl.setPath(QDir::cleanPath(sourceUrl.path() + QStringLiteral("/") + file));

            QUrl destinationUrl = m_destinationUrl;
            destinationUrl.setPath(QDir::cleanPath(destinationUrl.path() + QStringLiteral("/") + file));

            // Send description to the GUI
            Q_EMIT description(this, i18n("Synchronizing"), qMakePair(i18n("Source"), sourceUrl.path()), qMakePair(i18n("Destination"), destinationUrl.path()));
        }
    }
}

void Smb4KSyncJob::slotReadStandardError()
{
    if (!m_terminated) {
        QString stdErr = QString::fromUtf8(m_process->readAllStandardError()).trimmed();
        Smb4KNotification::synchronizationFailed(m_sourceUrl, m_destinationUrl, stdErr);
    }
}

void Smb4KSyncJob::slotProcessFinished(int, QProcess::ExitStatus status)
{
    // Dummy to show 100 %
    emitPercent(100, 100);

    // Handle error.
    switch (status) {
    case QProcess::CrashExit: {
        Smb4KNotification::processError(m_process->error());
        break;
    }
    default: {
        break;
    }
    }

    // Finish job
    emitResult();
    Q_EMIT finished(m_destinationUrl.path());
}

Smb4KSynchronizationDialog::Smb4KSynchronizationDialog(const SharePtr &share, QWidget *parent)
    : QDialog(parent)
    , m_share(share)
{
    setWindowTitle(i18n("Synchronization"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    m_swap_button = buttonBox->addButton(i18n("Swap Paths"), QDialogButtonBox::ActionRole);
    m_swap_button->setToolTip(i18n("Swap source and destination"));
    m_synchronize_button = buttonBox->addButton(i18n("Synchronize"), QDialogButtonBox::ActionRole);
    m_synchronize_button->setToolTip(i18n("Synchronize the destination with the source"));
    m_cancel_button = buttonBox->addButton(QDialogButtonBox::Cancel);

    m_cancel_button->setShortcut(Qt::Key_Escape);

    m_synchronize_button->setDefault(true);

    QGridLayout *layout = new QGridLayout(this);

    QLabel *pixmap = new QLabel(this);
    QPixmap sync_pix = KDE::icon(QStringLiteral("folder-sync")).pixmap(KIconLoader::SizeHuge);
    pixmap->setPixmap(sync_pix);
    pixmap->setAlignment(Qt::AlignBottom);

    QLabel *description = new QLabel(i18n("Please provide the source and destination "
                                          "directory for the synchronization."),
                                     this);
    description->setWordWrap(true);
    description->setAlignment(Qt::AlignBottom);

    QUrl sourceUrl = QUrl(QDir::cleanPath(m_share->path()));
    QUrl destinationUrl =
        QUrl(QDir::cleanPath(Smb4KSettings::rsyncPrefix().path() + QDir::separator() + m_share->hostName() + QDir::separator() + m_share->shareName()));

    QLabel *source_label = new QLabel(i18n("Source:"), this);
    m_source = new KUrlRequester(this);
    m_source->setUrl(sourceUrl);
    m_source->setMode(KFile::Directory | KFile::LocalOnly);
    m_source->lineEdit()->setSqueezedTextEnabled(true);
    m_source->completionObject()->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_source->completionObject()->setMode(KUrlCompletion::FileCompletion);
    m_source->setWhatsThis(
        i18n("This is the source directory. The data that it contains is to be written "
             "to the destination directory."));

    QLabel *destination_label = new QLabel(i18n("Destination:"), this);
    m_destination = new KUrlRequester(this);
    m_destination->setUrl(destinationUrl);
    m_destination->setMode(KFile::Directory | KFile::LocalOnly);
    m_destination->lineEdit()->setSqueezedTextEnabled(true);
    m_destination->completionObject()->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_destination->completionObject()->setMode(KUrlCompletion::FileCompletion);
    m_destination->setWhatsThis(
        i18n("This is the destination directory. It will be updated with the data "
             "from the source directory."));

    layout->addWidget(pixmap, 0, 0);
    layout->addWidget(description, 0, 1, Qt::AlignBottom);
    layout->addWidget(source_label, 1, 0);
    layout->addWidget(m_source, 1, 1);
    layout->addWidget(destination_label, 2, 0);
    layout->addWidget(m_destination, 2, 1);
    layout->addWidget(buttonBox, 3, 0, 1, 2);

    //
    // Connections
    //
    connect(m_cancel_button, SIGNAL(clicked()), SLOT(slotCancelClicked()));
    connect(m_synchronize_button, SIGNAL(clicked()), SLOT(slotSynchronizeClicked()));
    connect(m_swap_button, SIGNAL(clicked()), SLOT(slotSwapPathsClicked()));

    //
    // Set the dialog size
    //
    create();

    KConfigGroup group(Smb4KSettings::self()->config(), "SynchronizationDialog");
    QSize dialogSize;

    if (group.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), group);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584
}

Smb4KSynchronizationDialog::~Smb4KSynchronizationDialog()
{
}

const QUrl Smb4KSynchronizationDialog::source()
{
    return m_source->url();
}

const QUrl Smb4KSynchronizationDialog::destination()
{
    return m_destination->url();
}

/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSynchronizationDialog::slotCancelClicked()
{
    reject();
}

void Smb4KSynchronizationDialog::slotSynchronizeClicked()
{
    KConfigGroup group(Smb4KSettings::self()->config(), "SynchronizationDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);
    accept();
}

void Smb4KSynchronizationDialog::slotSwapPathsClicked()
{
    // Swap URLs.
    QString sourceURL = m_source->url().path();
    QString destinationURL = m_destination->url().path();

    m_source->setUrl(QUrl(destinationURL));
    m_destination->setUrl(QUrl(sourceURL));
}

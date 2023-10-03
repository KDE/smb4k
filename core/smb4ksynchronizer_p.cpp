/*
    This file contains private helper classes for the Smb4KSynchronizer
    class.

    SPDX-FileCopyrightText: 2008-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4ksynchronizer_p.h"
#include "smb4kglobal.h"
#include "smb4knotification.h"
#include "smb4ksettings.h"

// Qt includes
#include <QLocale>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTimer>

// KDE includes
#include <KLocalizedString>

using namespace Smb4KGlobal;

Smb4KSyncJob::Smb4KSyncJob(QObject *parent)
    : KJob(parent)
    , m_process(nullptr)
{
    setCapabilities(KJob::Killable);

    m_terminated = false;
    m_jobTracker = new KUiServerJobTracker(this);
}

Smb4KSyncJob::~Smb4KSyncJob()
{
}

void Smb4KSyncJob::start()
{
    QTimer::singleShot(0, this, SLOT(slotStartSynchronization()));
}

void Smb4KSyncJob::setupSynchronization(const QUrl &sourceUrl, const QUrl &destinationUrl)
{
    if (sourceUrl.isValid() && !sourceUrl.isEmpty() && destinationUrl.isValid() && !destinationUrl.isEmpty()) {
        m_sourceUrl = sourceUrl;
        m_destinationUrl = destinationUrl;
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
    if (m_sourceUrl.isEmpty() || m_destinationUrl.isEmpty()) {
        emitResult();
        return;
    }

    QString rsync = QStandardPaths::findExecutable(QStringLiteral("rsync"));

    if (rsync.isEmpty()) {
        Smb4KNotification::commandNotFound(QStringLiteral("rsync"));
        emitResult();
        return;
    }

    QDir destinationDirectory(m_destinationUrl.path());

    if (!destinationDirectory.exists()) {
        if (!QDir().mkpath(destinationDirectory.path())) {
            Smb4KNotification::mkdirFailed(destinationDirectory);
            emitResult();
            return;
        }
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
    m_jobTracker->registerJob(this);
    connect(this, &Smb4KSyncJob::result, m_jobTracker, &KUiServerJobTracker::unregisterJob);

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
            QRegularExpression expression(QStringLiteral("../s"));
            QString speedString = transferInfo.section(QStringLiteral(" "), 2, 2).section(expression, 0, 0);

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
            QString relativePath = line.trimmed().simplified();

            QUrl sourceUrl = m_sourceUrl;
            sourceUrl.setPath(QDir::cleanPath(sourceUrl.path() + QStringLiteral("/") + relativePath));

            QUrl destinationUrl = m_destinationUrl;
            destinationUrl.setPath(QDir::cleanPath(destinationUrl.path() + QStringLiteral("/") + relativePath));

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

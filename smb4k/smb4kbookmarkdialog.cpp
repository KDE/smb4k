/*
 *  Bookmark dialog
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

// application specific includes
#include "smb4kbookmarkdialog.h"
#include "core/smb4kbookmark.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4ksettings.h"
#include "smb4khomesuserdialog.h"

// Qt includes
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QWindow>

// KDE includes
#include <KConfigGroup>
#include <KIconLoader>
#include <KLocalizedString>
#include <KWindowConfig>

Smb4KBookmarkDialog::Smb4KBookmarkDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Bookmark Shares"));
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QWidget *descriptionWidget = new QWidget(this);
    QHBoxLayout *descriptionWidgetLayout = new QHBoxLayout(descriptionWidget);
    descriptionWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *descriptionPixmap = new QLabel(descriptionWidget);
    descriptionPixmap->setPixmap(KDE::icon(QStringLiteral("bookmark-new")).pixmap(KIconLoader::SizeHuge));
    descriptionPixmap->setAlignment(Qt::AlignBottom);
    descriptionPixmap->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    descriptionWidgetLayout->addWidget(descriptionPixmap);

    QLabel *descriptionText = new QLabel(this);
    descriptionText->setText(i18n("All listed shares will be bookmarked. To add a label or category, double-click the respective bookmark entry."));
    descriptionText->setWordWrap(true);
    descriptionText->setAlignment(Qt::AlignBottom);
    descriptionText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    descriptionWidgetLayout->addWidget(descriptionText);

    layout->addWidget(descriptionWidget);
    layout->addSpacing(layout->spacing());

    m_listWidget = new QListWidget(this);
    m_listWidget->setSelectionMode(QListWidget::SingleSelection);
    m_listWidget->viewport()->installEventFilter(this);

    connect(m_listWidget, &QListWidget::itemDoubleClicked, this, &Smb4KBookmarkDialog::slotItemDoubleClicked);
    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, &Smb4KBookmarkDialog::slotItemSelectionChanged);

    layout->addWidget(m_listWidget);

    m_editorWidget = new QWidget(this);
    m_editorWidget->setVisible(false);

    QGridLayout *editorWidgetLayout = new QGridLayout(m_editorWidget);
    editorWidgetLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *labelLabel = new QLabel(i18n("Label:"), m_editorWidget);
    m_labelEdit = new KLineEdit(m_editorWidget);
    m_labelEdit->setClearButtonEnabled(true);
    m_labelEdit->setCompletionMode(KCompletion::CompletionPopupAuto);

    connect(m_labelEdit, &KLineEdit::editingFinished, this, &Smb4KBookmarkDialog::slotLabelEdited);

    QLabel *categoryLabel = new QLabel(i18n("Category:"), m_editorWidget);
    m_categoryEdit = new KComboBox(m_editorWidget);
    m_categoryEdit->setEditable(true);
    m_categoryEdit->lineEdit()->setClearButtonEnabled(true);
    m_categoryEdit->setCompletionMode(KCompletion::CompletionPopupAuto);

    QStringList categories = Smb4KBookmarkHandler::self()->categoryList();

    if (!categories.contains(QStringLiteral(""))) {
        categories << QStringLiteral("");
    }

    m_categoryEdit->addItems(categories);
    m_categoryEdit->setCurrentText(QStringLiteral(""));

    connect(m_categoryEdit->lineEdit(), &KLineEdit::editingFinished, this, &Smb4KBookmarkDialog::slotCategoryEdited);

    editorWidgetLayout->addWidget(labelLabel, 0, 0);
    editorWidgetLayout->addWidget(m_labelEdit, 0, 1);
    editorWidgetLayout->addWidget(categoryLabel, 1, 0);
    editorWidgetLayout->addWidget(m_categoryEdit, 1, 1);

    layout->addWidget(m_editorWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    m_saveButton = buttonBox->addButton(QDialogButtonBox::Save);
    m_saveButton->setShortcut(QKeySequence::Save);
    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    m_cancelButton->setShortcut(QKeySequence::Cancel);
    m_cancelButton->setDefault(true);

    connect(m_saveButton, &QPushButton::clicked, this, &Smb4KBookmarkDialog::slotSaveBookmarks);
    connect(m_cancelButton, &QPushButton::clicked, this, &Smb4KBookmarkDialog::reject);

    layout->addWidget(buttonBox);

    setMinimumWidth(sizeHint().width() > 350 ? sizeHint().width() : 350);

    create();

    KConfigGroup dialogGroup(Smb4KSettings::self()->config(), "BookmarkDialog");
    QSize dialogSize;

    if (dialogGroup.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), dialogGroup);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584

    KConfigGroup completionGroup(Smb4KSettings::self()->config(), "CompletionItems");

    if (completionGroup.exists()) {
        m_labelEdit->completionObject()->setItems(completionGroup.readEntry("LabelCompletion", QStringList()));
        m_categoryEdit->completionObject()->setItems(completionGroup.readEntry("CategoryCompletion", Smb4KBookmarkHandler::self()->categoryList()));
    }
}

Smb4KBookmarkDialog::~Smb4KBookmarkDialog()
{
}

bool Smb4KBookmarkDialog::setShares(const QList<SharePtr> &shares)
{
    bool bookmarksSet = false;

    for (const SharePtr &share : qAsConst(shares)) {
        if (share->isHomesShare()) {
            QPointer<Smb4KHomesUserDialog> homesUserDialog = new Smb4KHomesUserDialog(this);

            if (homesUserDialog->setShare(share)) {
                // We want to get a return value here, so we use exec()
                if (homesUserDialog->exec() != QDialog::Accepted) {
                    delete homesUserDialog;
                    continue;
                } else {
                    delete homesUserDialog;
                }
            } else {
                delete homesUserDialog;
                continue;
            }
        }

        if (Smb4KBookmarkHandler::self()->isBookmarked(share)) {
            continue;
        }

        Smb4KBookmark bookmark;
        bookmark.setShare(share.data());

        QVariant variant = QVariant::fromValue(bookmark);

        QListWidgetItem *bookmarkItem = new QListWidgetItem(m_listWidget);
        bookmarkItem->setText(bookmark.displayString());
        bookmarkItem->setIcon(bookmark.icon());
        bookmarkItem->setData(Qt::UserRole, variant);

        bookmarksSet = true;
    }

    return bookmarksSet;
}

bool Smb4KBookmarkDialog::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_listWidget->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QPointF pos = m_listWidget->viewport()->mapFromGlobal(mouseEvent->globalPosition());
            QListWidgetItem *item = m_listWidget->itemAt(pos.toPoint());
#else
            QPoint pos = m_listWidget->viewport()->mapFromGlobal(mouseEvent->globalPos());
            QListWidgetItem *item = m_listWidget->itemAt(pos);
#endif
            if (!item) {
                m_listWidget->clearSelection();
            }
        }
    }

    return QObject::eventFilter(object, event);
}

void Smb4KBookmarkDialog::slotItemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);

    Smb4KBookmark bookmark = item->data(Qt::UserRole).value<Smb4KBookmark>();

    m_labelEdit->setText(bookmark.label());
    m_categoryEdit->setCurrentText(bookmark.categoryName());

    m_editorWidget->setVisible(true);
}

void Smb4KBookmarkDialog::slotItemSelectionChanged()
{
    if (m_editorWidget->isVisible()) {
        m_editorWidget->setVisible(false);
        m_labelEdit->clear();
        m_categoryEdit->lineEdit()->clear();
    }
}

void Smb4KBookmarkDialog::slotLabelEdited()
{
    if (m_listWidget->currentItem() && m_editorWidget->isVisible()) {
        Smb4KBookmark bookmark = m_listWidget->currentItem()->data(Qt::UserRole).value<Smb4KBookmark>();
        bookmark.setLabel(m_labelEdit->text());

        QVariant variant = QVariant::fromValue(bookmark);
        m_listWidget->currentItem()->setData(Qt::UserRole, variant);

        m_labelEdit->completionObject()->addItem(m_labelEdit->text());
    }
}

void Smb4KBookmarkDialog::slotCategoryEdited()
{
    if (m_listWidget->currentItem() && m_editorWidget->isVisible()) {
        Smb4KBookmark bookmark = m_listWidget->currentItem()->data(Qt::UserRole).value<Smb4KBookmark>();
        bookmark.setCategoryName(m_categoryEdit->currentText());

        QVariant variant = QVariant::fromValue(bookmark);
        m_listWidget->currentItem()->setData(Qt::UserRole, variant);

        if (!m_categoryEdit->contains(m_categoryEdit->currentText())) {
            m_categoryEdit->addItem(m_categoryEdit->currentText());
        }

        m_categoryEdit->completionObject()->addItem(m_categoryEdit->currentText());
    }
}

void Smb4KBookmarkDialog::slotSaveBookmarks()
{
    if (m_editorWidget->isVisible()) {
        Smb4KBookmark bookmark = m_listWidget->currentItem()->data(Qt::UserRole).value<Smb4KBookmark>();

        bookmark.setLabel(m_labelEdit->text());
        bookmark.setCategoryName(m_categoryEdit->currentText());
    }

    QList<BookmarkPtr> bookmarks;

    for (int i = 0; i < m_listWidget->count(); ++i) {
        Smb4KBookmark bookmark = m_listWidget->item(i)->data(Qt::UserRole).value<Smb4KBookmark>();
        bookmarks << BookmarkPtr(new Smb4KBookmark(bookmark));
    }

    Smb4KBookmarkHandler::self()->addBookmarks(bookmarks);

    KConfigGroup dialogGroup(Smb4KSettings::self()->config(), "BookmarkEditor");
    KWindowConfig::saveWindowSize(windowHandle(), dialogGroup);

    KConfigGroup completionGroup(Smb4KSettings::self()->config(), "CompletionItems");
    completionGroup.writeEntry("LabelCompletion", m_labelEdit->completionObject()->items());
    completionGroup.writeEntry("CategoryCompletion", m_categoryEdit->completionObject()->items());

    accept();
}

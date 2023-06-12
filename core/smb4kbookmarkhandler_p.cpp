/*
    Private classes for the bookmark handler

    SPDX-FileCopyrightText: 2011-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kbookmarkhandler_p.h"
#include "smb4kbookmark.h"
#include "smb4ksettings.h"

// Qt includes
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWindow>

// KDE includes
#include <KComboBox>
#include <KIconLoader>
#include <KLineEdit>
#include <KLocalizedString>
#include <KWindowConfig>

Smb4KBookmarkDialog::Smb4KBookmarkDialog(const QList<BookmarkPtr> &bookmarks, const QStringList &categories, QWidget *parent)
    : QDialog(parent)
{
    //
    // Set the window title
    //
    setWindowTitle(i18n("Add Bookmarks"));

    //
    // Setup the view
    //
    setupView();

    //
    // Load the list of bookmarks and categories
    //
    loadLists(bookmarks, categories);

    //
    // Set the dialog size
    //
    create();

    KConfigGroup group(Smb4KSettings::self()->config(), "BookmarkDialog");
    QSize dialogSize;

    if (group.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), group);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584

    //
    // Fill the completion objects
    //
    KComboBox *categoryCombo = findChild<KComboBox *>(QStringLiteral("CategoryCombo"));

    if (group.hasKey("GroupCompletion")) {
        // For backward compatibility (since Smb4K 3.0.72).
        categoryCombo->completionObject()->setItems(group.readEntry("GroupCompletion", m_categories));
        group.deleteEntry("GroupCompletion");
    } else {
        categoryCombo->completionObject()->setItems(group.readEntry("CategoryCompletion", m_categories));
    }

    KLineEdit *labelEdit = findChild<KLineEdit *>(QStringLiteral("LabelEdit"));
    labelEdit->completionObject()->setItems(group.readEntry("LabelCompletion", QStringList()));

    //
    // Connections
    //
    connect(KIconLoader::global(), SIGNAL(iconChanged(int)), SLOT(slotIconSizeChanged(int)));
}

Smb4KBookmarkDialog::~Smb4KBookmarkDialog()
{
    while (!m_bookmarks.isEmpty()) {
        m_bookmarks.takeFirst().clear();
    }
}

const QList<BookmarkPtr> &Smb4KBookmarkDialog::bookmarks()
{
    return m_bookmarks;
}

void Smb4KBookmarkDialog::setupView()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QWidget *description = new QWidget(this);
    QHBoxLayout *descriptionLayout = new QHBoxLayout(description);
    descriptionLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *pixmap = new QLabel(description);
    QPixmap sync_pix = KDE::icon(QStringLiteral("bookmark-new")).pixmap(KIconLoader::SizeHuge);
    pixmap->setPixmap(sync_pix);
    pixmap->setAlignment(Qt::AlignBottom);

    QLabel *label = new QLabel(i18n("All listed shares will be bookmarked. To edit the label "
                                    "or group, click the respective bookmark entry."),
                               description);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignBottom);

    descriptionLayout->addWidget(pixmap, 0);
    descriptionLayout->addWidget(label, Qt::AlignBottom);

    QListWidget *listWidget = new QListWidget(this);
    listWidget->setObjectName(QStringLiteral("BookmarksListWidget"));
    listWidget->setSortingEnabled(true);
    listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    int iconSize = KIconLoader::global()->currentSize(KIconLoader::Small);
    listWidget->setIconSize(QSize(iconSize, iconSize));

    QWidget *editorWidgets = new QWidget(this);
    editorWidgets->setObjectName(QStringLiteral("EditorWidgets"));
    editorWidgets->setEnabled(false);

    QGridLayout *editorWidgetsLayout = new QGridLayout(editorWidgets);
    editorWidgetsLayout->setContentsMargins(0, 0, 0, 0);

    //
    // The label editor
    //
    QLabel *labelLabel = new QLabel(i18n("Label:"), editorWidgets);
    KLineEdit *labelEdit = new KLineEdit(editorWidgets);
    labelEdit->setObjectName(QStringLiteral("LabelEdit"));
    labelEdit->setClearButtonEnabled(true);

    //
    // The category editor
    //
    QLabel *categoryLabel = new QLabel(i18n("Category:"), editorWidgets);
    KComboBox *categoryCombo = new KComboBox(true, editorWidgets);
    categoryCombo->setObjectName(QStringLiteral("CategoryCombo"));

    editorWidgetsLayout->addWidget(labelLabel, 0, 0);
    editorWidgetsLayout->addWidget(labelEdit, 0, 1);
    editorWidgetsLayout->addWidget(categoryLabel, 1, 0);
    editorWidgetsLayout->addWidget(categoryCombo, 1, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    QPushButton *okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    QPushButton *cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);

    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    cancelButton->setShortcut(Qt::Key_Escape);

    okButton->setDefault(true);

    layout->addWidget(description, 0);
    layout->addWidget(listWidget, 0);
    layout->addWidget(editorWidgets, 0);
    layout->addWidget(buttonBox, 0);

    //
    // Connections
    //
    connect(listWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(slotBookmarkClicked(QListWidgetItem *)));
    connect(labelEdit, SIGNAL(editingFinished()), this, SLOT(slotLabelEdited()));
    connect(categoryCombo->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotCategoryEdited()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(slotDialogAccepted()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void Smb4KBookmarkDialog::loadLists(const QList<BookmarkPtr> &bookmarks, const QStringList &categories)
{
    //
    // Get the category combo box
    //
    KComboBox *categoryCombo = findChild<KComboBox *>(QStringLiteral("CategoryCombo"));
    QListWidget *listWidget = findChild<QListWidget *>(QStringLiteral("BookmarksListWidget"));

    //
    // Copy the bookmarks to the internal list and add them to
    // the list widget afterwards.
    //
    for (const BookmarkPtr &b : bookmarks) {
        QListWidgetItem *item = new QListWidgetItem(b->icon(), b->displayString(), listWidget);
        item->setData(Qt::UserRole, static_cast<QUrl>(b->url()));

        m_bookmarks << b;
    }

    //
    // Copy the categories
    //
    m_categories = categories;

    //
    // Add the categories to the combo box
    //
    categoryCombo->addItems(m_categories);
}

BookmarkPtr Smb4KBookmarkDialog::findBookmark(const QUrl &url)
{
    BookmarkPtr bookmark;

    for (const BookmarkPtr &b : qAsConst(m_bookmarks)) {
        if (b->url() == url) {
            bookmark = b;
            break;
        } else {
            continue;
        }
    }

    return bookmark;
}

void Smb4KBookmarkDialog::slotBookmarkClicked(QListWidgetItem *bookmarkItem)
{
    //
    // Get the widgets
    //
    KComboBox *categoryCombo = findChild<KComboBox *>(QStringLiteral("CategoryCombo"));
    KLineEdit *labelEdit = findChild<KLineEdit *>(QStringLiteral("LabelEdit"));
    QWidget *editorWidgets = findChild<QWidget *>(QStringLiteral("EditorWidgets"));

    //
    // Modify the widgets
    //
    if (bookmarkItem) {
        // Enable the editor widgets if necessary
        if (!editorWidgets->isEnabled()) {
            editorWidgets->setEnabled(true);
        }

        QUrl url = bookmarkItem->data(Qt::UserRole).toUrl();
        BookmarkPtr bookmark = findBookmark(url);

        if (bookmark) {
            labelEdit->setText(bookmark->label());
            categoryCombo->setCurrentItem(bookmark->categoryName());
        } else {
            labelEdit->clear();
            categoryCombo->clearEditText();
            editorWidgets->setEnabled(false);
        }
    } else {
        labelEdit->clear();
        categoryCombo->clearEditText();
        editorWidgets->setEnabled(false);
    }
}

void Smb4KBookmarkDialog::slotLabelEdited()
{
    //
    // Get the label line edit
    //
    KLineEdit *labelEdit = findChild<KLineEdit *>(QStringLiteral("LabelEdit"));
    QListWidget *listWidget = findChild<QListWidget *>(QStringLiteral("BookmarksListWidget"));

    //
    // Set the label
    //
    QUrl url = listWidget->currentItem()->data(Qt::UserRole).toUrl();
    BookmarkPtr bookmark = findBookmark(url);

    if (bookmark) {
        bookmark->setLabel(labelEdit->userText());
    }

    //
    // Add label to completion object
    //
    KCompletion *completion = labelEdit->completionObject();

    if (!labelEdit->userText().isEmpty()) {
        completion->addItem(labelEdit->userText());
    }
}

void Smb4KBookmarkDialog::slotCategoryEdited()
{
    //
    // Get the category combo box
    //
    KComboBox *categoryCombo = findChild<KComboBox *>(QStringLiteral("CategoryCombo"));
    QListWidget *listWidget = findChild<QListWidget *>(QStringLiteral("BookmarksListWidget"));

    //
    // Get the bookmark
    //
    QUrl url = listWidget->currentItem()->data(Qt::UserRole).toUrl();
    BookmarkPtr bookmark = findBookmark(url);

    //
    // Set the category name
    //
    if (bookmark) {
        bookmark->setCategoryName(categoryCombo->currentText());
    }

    //
    // Add the category name to the combo box
    //
    if (categoryCombo->findText(categoryCombo->currentText()) == -1) {
        categoryCombo->addItem(categoryCombo->currentText());
    }

    // Add group to completion object
    KCompletion *completion = categoryCombo->completionObject();

    if (!categoryCombo->currentText().isEmpty()) {
        completion->addItem(categoryCombo->currentText());
    }
}

void Smb4KBookmarkDialog::slotDialogAccepted()
{
    //
    // Get the widgets
    //
    KComboBox *categoryCombo = findChild<KComboBox *>(QStringLiteral("CategoryCombo"));
    KLineEdit *labelEdit = findChild<KLineEdit *>(QStringLiteral("LabelEdit"));

    KConfigGroup group(Smb4KSettings::self()->config(), "BookmarkDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);
    group.writeEntry("LabelCompletion", labelEdit->completionObject()->items());
    group.writeEntry("CategoryCompletion", categoryCombo->completionObject()->items());

    accept();
}

void Smb4KBookmarkDialog::slotIconSizeChanged(int group)
{
    //
    // Get the list widget
    //
    QListWidget *listWidget = findChild<QListWidget *>(QStringLiteral("BookmarksListWidget"));

    //
    // Change the icon size
    //
    switch (group) {
    case KIconLoader::Small: {
        int iconSize = KIconLoader::global()->currentSize(KIconLoader::Small);
        listWidget->setIconSize(QSize(iconSize, iconSize));
        break;
    }
    default: {
        break;
    }
    }
}

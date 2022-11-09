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
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTimer>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>
#include <QWindow>

// KDE includes
#include <KCompletion/KComboBox>
#include <KCompletion/KLineEdit>
#include <KConfigGui/KWindowConfig>
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>

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

Smb4KBookmarkEditor::Smb4KBookmarkEditor(const QList<BookmarkPtr> &bookmarks, QWidget *parent)
    : QDialog(parent)
    , m_bookmarks(bookmarks)
{
    //
    // Set the window title
    //
    setWindowTitle(i18n("Edit Bookmarks"));

    //
    // Setup the view
    //
    setupView();

    //
    // Load the bookmarks into the editor
    //
    loadBookmarks();

    //
    // Set the dialog size
    //
    create();

    KConfigGroup group(Smb4KSettings::self()->config(), "BookmarkEditor");
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
    KLineEdit *labelEdit = findChild<KLineEdit *>(QStringLiteral("LabelEdit"));
    KLineEdit *ipEdit = findChild<KLineEdit *>(QStringLiteral("IpEdit"));
    KLineEdit *loginEdit = findChild<KLineEdit *>(QStringLiteral("LoginEdit"));
    KLineEdit *workgroupEdit = findChild<KLineEdit *>(QStringLiteral("WorkgroupEdit"));

    if (group.hasKey("GroupCompletion")) {
        // For backward compatibility (since Smb4K 3.0.72).
        categoryCombo->completionObject()->setItems(group.readEntry("GroupCompletion", m_categories));
        group.deleteEntry("GroupCompletion");
    } else {
        categoryCombo->completionObject()->setItems(group.readEntry("CategoryCompletion", m_categories));
    }

    labelEdit->completionObject()->setItems(group.readEntry("LabelCompletion", QStringList()));
    ipEdit->completionObject()->setItems(group.readEntry("IPCompletion", QStringList()));
    loginEdit->completionObject()->setItems(group.readEntry("LoginCompletion", QStringList()));
    workgroupEdit->completionObject()->setItems(group.readEntry("WorkgroupCompletion", QStringList()));

    //
    // Connections
    //
    connect(KIconLoader::global(), SIGNAL(iconChanged(int)), SLOT(slotIconSizeChanged(int)));
}

Smb4KBookmarkEditor::~Smb4KBookmarkEditor()
{
    while (!m_bookmarks.isEmpty()) {
        m_bookmarks.takeFirst().clear();
    }
}

bool Smb4KBookmarkEditor::eventFilter(QObject *obj, QEvent *e)
{
    //
    // Get the widget
    //
    QTreeWidget *treeWidget = findChild<QTreeWidget *>(QStringLiteral("BookmarksTreeWidget"));

    if (obj == treeWidget->viewport()) {
        switch (e->type()) {
        case QEvent::DragEnter: {
            QDragEnterEvent *ev = static_cast<QDragEnterEvent *>(e);

            if (ev->source() == treeWidget->viewport()) {
                e->accept();
            } else {
                e->ignore();
            }
            break;
        }
        case QEvent::DragLeave: {
            e->ignore();
            break;
        }
        case QEvent::Drop: {
            QTimer::singleShot(50, this, SLOT(slotAdjust()));
            break;
        }
        default: {
            break;
        }
        }
    }

    return QDialog::eventFilter(obj, e);
}

void Smb4KBookmarkEditor::setupView()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QTreeWidget *treeWidget = new QTreeWidget(this);
    treeWidget->setObjectName(QStringLiteral("BookmarksTreeWidget"));
    treeWidget->setColumnCount(2);
    treeWidget->hideColumn((treeWidget->columnCount() - 1)); // for sorting purposes
    treeWidget->headerItem()->setHidden(true);
    treeWidget->setRootIsDecorated(true);
    treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    treeWidget->setDragDropMode(QTreeWidget::InternalMove);
    int iconSize = KIconLoader::global()->currentSize(KIconLoader::Small);
    treeWidget->setIconSize(QSize(iconSize, iconSize));
    treeWidget->viewport()->installEventFilter(this);

    QAction *addCategoryAction = new QAction(KDE::icon(QStringLiteral("bookmark-add-folder")), i18n("Add Category"), treeWidget);
    QAction *deleteAction = new QAction(KDE::icon(QStringLiteral("edit-delete")), i18n("Remove"), treeWidget);
    deleteAction->setObjectName(QStringLiteral("DeleteAction"));
    QAction *clearAction = new QAction(KDE::icon(QStringLiteral("edit-clear")), i18n("Clear"), treeWidget);

    KActionMenu *actionMenu = new KActionMenu(treeWidget);
    actionMenu->setObjectName(QStringLiteral("ActionMenu"));
    actionMenu->addAction(addCategoryAction);
    actionMenu->addAction(deleteAction);
    actionMenu->addAction(clearAction);

    //
    // The editor widgets
    //
    QWidget *editorWidgets = new QWidget(this);
    editorWidgets->setObjectName(QStringLiteral("EditorWidgets"));
    editorWidgets->setEnabled(false);

    QGridLayout *editorsLayout = new QGridLayout(editorWidgets);
    editorsLayout->setContentsMargins(0, 0, 0, 0);

    //
    // The label line edit
    //
    QLabel *labelLabel = new QLabel(i18n("Label:"), editorWidgets);
    KLineEdit *labelEdit = new KLineEdit(editorWidgets);
    labelEdit->setObjectName(QStringLiteral("LabelEdit"));
    labelEdit->setClearButtonEnabled(true);

    QLabel *loginLabel = new QLabel(i18n("Username:"), editorWidgets);
    KLineEdit *loginEdit = new KLineEdit(editorWidgets);
    loginEdit->setObjectName(QStringLiteral("LoginEdit"));
    loginEdit->setClearButtonEnabled(true);

    //
    // The workgroup/domain edit line
    //
    QLabel *workgroupLabel = new QLabel(i18n("Workgroup:"), editorWidgets);
    KLineEdit *workgroupEdit = new KLineEdit(editorWidgets);
    workgroupEdit->setObjectName(QStringLiteral("WorkgroupEdit"));
    workgroupEdit->setClearButtonEnabled(true);

    //
    // The IP address line edit
    //
    QLabel *ipLabel = new QLabel(i18n("IP Address:"), editorWidgets);
    KLineEdit *ipEdit = new KLineEdit(editorWidgets);
    ipEdit->setObjectName(QStringLiteral("IpEdit"));
    ipEdit->setClearButtonEnabled(true);

    //
    // The category combo box
    //
    QLabel *categoryLabel = new QLabel(i18n("Category:"), editorWidgets);
    KComboBox *categoryCombo = new KComboBox(true, editorWidgets);
    categoryCombo->setObjectName(QStringLiteral("CategoryCombo"));
    categoryCombo->setDuplicatesEnabled(false);

    editorsLayout->addWidget(labelLabel, 0, 0);
    editorsLayout->addWidget(labelEdit, 0, 1);
    editorsLayout->addWidget(loginLabel, 1, 0);
    editorsLayout->addWidget(loginEdit, 1, 1);
    editorsLayout->addWidget(workgroupLabel, 2, 0);
    editorsLayout->addWidget(workgroupEdit, 2, 1);
    editorsLayout->addWidget(ipLabel, 3, 0);
    editorsLayout->addWidget(ipEdit, 3, 1);
    editorsLayout->addWidget(categoryLabel, 4, 0);
    editorsLayout->addWidget(categoryCombo, 4, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    QPushButton *okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    QPushButton *cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);

    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    cancelButton->setShortcut(Qt::Key_Escape);

    okButton->setDefault(true);

    layout->addWidget(treeWidget);
    layout->addWidget(editorWidgets);
    layout->addWidget(buttonBox);

    //
    // Connections
    //
    connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(slotItemClicked(QTreeWidgetItem *, int)));
    connect(treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenuRequested(QPoint)));
    connect(labelEdit, SIGNAL(editingFinished()), this, SLOT(slotLabelEdited()));
    connect(ipEdit, SIGNAL(editingFinished()), this, SLOT(slotIpEdited()));
    connect(workgroupEdit, SIGNAL(editingFinished()), this, SLOT(slotWorkgroupNameEdited()));
    connect(loginEdit, SIGNAL(editingFinished()), this, SLOT(slotLoginEdited()));
    connect(categoryCombo->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotCategoryEdited()));
    connect(addCategoryAction, SIGNAL(triggered(bool)), this, SLOT(slotAddCategoryTriggered(bool)));
    connect(deleteAction, SIGNAL(triggered(bool)), this, SLOT(slotDeleteTriggered(bool)));
    connect(clearAction, SIGNAL(triggered(bool)), this, SLOT(slotClearTriggered(bool)));
    connect(okButton, SIGNAL(clicked()), this, SLOT(slotDialogAccepted()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(slotDialogRejected()));
}

void Smb4KBookmarkEditor::loadBookmarks()
{
    //
    // Get the widgets
    //
    KComboBox *categoryCombo = findChild<KComboBox *>(QStringLiteral("CategoryCombo"));
    QTreeWidget *treeWidget = findChild<QTreeWidget *>(QStringLiteral("BookmarksTreeWidget"));

    //
    // Clear the tree widget and the group combo box
    //
    treeWidget->clear();
    categoryCombo->clear();

    //
    // Copy the groups into the internal list
    //
    m_categories.clear();

    for (const BookmarkPtr &bookmark : qAsConst(m_bookmarks)) {
        if (!m_categories.contains(bookmark->categoryName())) {
            m_categories << bookmark->categoryName();
        }
    }

    //
    // Insert the groups into the tree widget
    //
    for (const QString &category : qAsConst(m_categories)) {
        if (!category.isEmpty()) {
            QTreeWidgetItem *categoryItem = new QTreeWidgetItem(QTreeWidgetItem::UserType);
            categoryItem->setIcon(0, KDE::icon(QStringLiteral("folder-bookmark")));
            categoryItem->setText(0, category);
            categoryItem->setText((treeWidget->columnCount() - 1), QStringLiteral("00_") + category);
            categoryItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
            treeWidget->addTopLevelItem(categoryItem);
        }
    }

    //
    // Insert the bookmarks info the tree widget
    //
    for (const BookmarkPtr &bookmark : qAsConst(m_bookmarks)) {
        QTreeWidgetItem *bookmarkItem = new QTreeWidgetItem(QTreeWidgetItem::UserType);
        bookmarkItem->setData(0, QTreeWidgetItem::UserType, static_cast<QUrl>(bookmark->url()));
        bookmarkItem->setIcon(0, bookmark->icon());
        bookmarkItem->setText(0, bookmark->displayString());
        bookmarkItem->setText((treeWidget->columnCount() - 1), QStringLiteral("01_") + bookmark->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort));
        bookmarkItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);

        if (!bookmark->categoryName().isEmpty()) {
            QList<QTreeWidgetItem *> items = treeWidget->findItems(bookmark->categoryName(), Qt::MatchFixedString | Qt::MatchCaseSensitive, 0);

            if (!items.isEmpty()) {
                items.first()->addChild(bookmarkItem);
                items.first()->setExpanded(true);
            }
        } else {
            treeWidget->addTopLevelItem(bookmarkItem);
        }
    }

    //
    // Sort
    //
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
        treeWidget->topLevelItem(i)->sortChildren((treeWidget->columnCount() - 1), Qt::AscendingOrder);
    }

    treeWidget->sortItems((treeWidget->columnCount() - 1), Qt::AscendingOrder);

    //
    // Check that an empty group entry is also present. If it is not there,
    // add it now and insert the groups to the group combo box afterwards.
    //
    if (!m_categories.contains(QStringLiteral("")) && !m_categories.contains(QString())) {
        m_categories << QStringLiteral("");
    }

    categoryCombo->addItems(m_categories);
    categoryCombo->setCurrentItem(QStringLiteral(""));
}

QList<BookmarkPtr> Smb4KBookmarkEditor::editedBookmarks()
{
    return m_bookmarks;
}

BookmarkPtr Smb4KBookmarkEditor::findBookmark(const QUrl &url)
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

void Smb4KBookmarkEditor::slotItemClicked(QTreeWidgetItem *item, int /*col*/)
{
    //
    // Get the widgets
    //
    KComboBox *categoryCombo = findChild<KComboBox *>(QStringLiteral("CategoryCombo"));
    QTreeWidget *treeWidget = findChild<QTreeWidget *>(QStringLiteral("BookmarksTreeWidget"));
    QWidget *editorWidgets = findChild<QWidget *>(QStringLiteral("EditorWidgets"));
    KLineEdit *labelEdit = findChild<KLineEdit *>(QStringLiteral("LabelEdit"));
    KLineEdit *ipEdit = findChild<KLineEdit *>(QStringLiteral("IpEdit"));
    KLineEdit *loginEdit = findChild<KLineEdit *>(QStringLiteral("LoginEdit"));
    KLineEdit *workgroupEdit = findChild<KLineEdit *>(QStringLiteral("WorkgroupEdit"));

    //
    // Process the item
    //
    if (item) {
        if (treeWidget->indexOfTopLevelItem(item) != -1) {
            // This is a top-level item, i.e. it is either a bookmark without
            // group or a group entry.
            // Bookmarks have an URL stored, group folders not.
            if (!item->data(0, QTreeWidgetItem::UserType).toUrl().isEmpty()) {
                BookmarkPtr bookmark = findBookmark(item->data(0, QTreeWidgetItem::UserType).toUrl());

                if (bookmark) {
                    labelEdit->setText(bookmark->label());
                    loginEdit->setText(bookmark->userName());
                    ipEdit->setText(bookmark->hostIpAddress());
                    workgroupEdit->setText(bookmark->workgroupName());
                    categoryCombo->setCurrentItem(bookmark->categoryName());
                    editorWidgets->setEnabled(true);
                } else {
                    labelEdit->clear();
                    loginEdit->clear();
                    ipEdit->clear();
                    workgroupEdit->clear();
                    categoryCombo->clearEditText();
                    editorWidgets->setEnabled(false);
                }
            } else {
                labelEdit->clear();
                loginEdit->clear();
                ipEdit->clear();
                workgroupEdit->clear();
                categoryCombo->clearEditText();
                editorWidgets->setEnabled(false);
            }
        } else {
            // This can only be a bookmark.
            BookmarkPtr bookmark = findBookmark(item->data(0, QTreeWidgetItem::UserType).toUrl());

            if (bookmark) {
                labelEdit->setText(bookmark->label());
                loginEdit->setText(bookmark->userName());
                ipEdit->setText(bookmark->hostIpAddress());
                workgroupEdit->setText(bookmark->workgroupName());
                categoryCombo->setCurrentItem(bookmark->categoryName());
                editorWidgets->setEnabled(true);
            } else {
                labelEdit->clear();
                loginEdit->clear();
                ipEdit->clear();
                workgroupEdit->clear();
                categoryCombo->clearEditText();
                editorWidgets->setEnabled(false);
            }
        }
    } else {
        labelEdit->clear();
        loginEdit->clear();
        ipEdit->clear();
        workgroupEdit->clear();
        categoryCombo->clearEditText();
        editorWidgets->setEnabled(false);
    }
}

void Smb4KBookmarkEditor::slotContextMenuRequested(const QPoint &pos)
{
    //
    // Get the widgets
    //
    QTreeWidget *treeWidget = findChild<QTreeWidget *>(QStringLiteral("BookmarksTreeWidget"));
    QAction *deleteAction = findChild<QAction *>(QStringLiteral("DeleteAction"));
    KActionMenu *actionMenu = findChild<KActionMenu *>(QStringLiteral("ActionMenu"));

    //
    // Open the context menu
    //
    QTreeWidgetItem *item = treeWidget->itemAt(pos);
    deleteAction->setEnabled((item));
    actionMenu->menu()->popup(treeWidget->viewport()->mapToGlobal(pos));
}

void Smb4KBookmarkEditor::slotLabelEdited()
{
    //
    // Get the widgets
    //
    QTreeWidget *treeWidget = findChild<QTreeWidget *>(QStringLiteral("BookmarksTreeWidget"));
    KLineEdit *labelEdit = findChild<KLineEdit *>(QStringLiteral("LabelEdit"));

    //
    // Find the bookmark
    //
    QUrl url = treeWidget->currentItem()->data(0, QTreeWidgetItem::UserType).toUrl();

    BookmarkPtr bookmark = findBookmark(url);

    //
    // Set the label
    //
    if (bookmark) {
        bookmark->setLabel(labelEdit->userText());
    }

    //
    // Add the label to the completion object
    //
    KCompletion *completion = labelEdit->completionObject();

    if (!labelEdit->userText().isEmpty()) {
        completion->addItem(labelEdit->userText());
    }
}

void Smb4KBookmarkEditor::slotLoginEdited()
{
    //
    // Get the widgets
    //
    QTreeWidget *treeWidget = findChild<QTreeWidget *>(QStringLiteral("BookmarksTreeWidget"));
    KLineEdit *loginEdit = findChild<KLineEdit *>(QStringLiteral("LoginEdit"));

    //
    // Find the bookmark
    //
    QUrl url = treeWidget->currentItem()->data(0, QTreeWidgetItem::UserType).toUrl();

    BookmarkPtr bookmark = findBookmark(url);

    //
    // Set the login
    //
    if (bookmark) {
        bookmark->setUserName(loginEdit->userText());
    }

    //
    // Add the login to the completion object
    //
    KCompletion *completion = loginEdit->completionObject();

    if (!loginEdit->userText().isEmpty()) {
        completion->addItem(loginEdit->userText());
    }
}

void Smb4KBookmarkEditor::slotIpEdited()
{
    //
    // Get the widgets
    //
    QTreeWidget *treeWidget = findChild<QTreeWidget *>(QStringLiteral("BookmarksTreeWidget"));
    KLineEdit *ipEdit = findChild<KLineEdit *>(QStringLiteral("IpEdit"));

    //
    // Find the bookmark
    //
    QUrl url = treeWidget->currentItem()->data(0, QTreeWidgetItem::UserType).toUrl();

    BookmarkPtr bookmark = findBookmark(url);

    //
    // Set the IP address
    //
    if (bookmark) {
        bookmark->setHostIpAddress(ipEdit->userText());
    }

    //
    // Add the IP address to the completion object
    //
    KCompletion *completion = ipEdit->completionObject();

    if (!ipEdit->userText().isEmpty()) {
        completion->addItem(ipEdit->userText());
    }
}

void Smb4KBookmarkEditor::slotWorkgroupNameEdited()
{
    //
    // Get the widgets
    //
    QTreeWidget *treeWidget = findChild<QTreeWidget *>(QStringLiteral("BookmarksTreeWidget"));
    KLineEdit *workgroupEdit = findChild<KLineEdit *>(QStringLiteral("WorkgroupEdit"));

    //
    // Find the bookmark
    //
    QUrl url = treeWidget->currentItem()->data(0, QTreeWidgetItem::UserType).toUrl();

    BookmarkPtr bookmark = findBookmark(url);

    //
    // Set the workgroup name
    //
    if (bookmark) {
        bookmark->setWorkgroupName(workgroupEdit->userText());
    }

    //
    // Add the workgroup name to the completion object
    //
    KCompletion *completion = workgroupEdit->completionObject();

    if (!workgroupEdit->userText().isEmpty()) {
        completion->addItem(workgroupEdit->userText());
    }
}

void Smb4KBookmarkEditor::slotCategoryEdited()
{
    //
    // Get the widgets
    //
    KComboBox *categoryCombo = findChild<KComboBox *>(QStringLiteral("CategoryCombo"));
    QTreeWidget *treeWidget = findChild<QTreeWidget *>(QStringLiteral("BookmarksTreeWidget"));

    //
    // Get the URL of the current item
    //
    QUrl url = treeWidget->currentItem()->data(0, QTreeWidgetItem::UserType).toUrl();

    //
    // Return here, if the current item is a group
    //
    if (url.isEmpty()) {
        return;
    }

    //
    // Find the bookmark
    //
    BookmarkPtr bookmark = findBookmark(url);

    //
    // Set the category
    //
    if (bookmark) {
        bookmark->setCategoryName(categoryCombo->currentText());
    }

    //
    // Reload the bookmarks (The current item is cleared by this!)
    //
    loadBookmarks();

    //
    // Reset the current item
    //
    QTreeWidgetItemIterator it(treeWidget);

    while (*it) {
        if ((*it)->data(0, QTreeWidgetItem::UserType).toUrl() == url) {
            treeWidget->setCurrentItem(*it);
            slotItemClicked(*it, 0);
            break;
        }

        ++it;
    }

    //
    // Add the category to the completion object
    //
    KCompletion *completion = categoryCombo->completionObject();

    if (!categoryCombo->currentText().isEmpty()) {
        completion->addItem(categoryCombo->currentText());
    }
}

void Smb4KBookmarkEditor::slotAddCategoryTriggered(bool /*checked*/)
{
    //
    // Get the widgets
    //
    KComboBox *categoryCombo = findChild<KComboBox *>(QStringLiteral("CategoryCombo"));
    QTreeWidget *treeWidget = findChild<QTreeWidget *>(QStringLiteral("BookmarksTreeWidget"));

    //
    // Process the category
    //
    bool ok = false;

    QString categoryName = QInputDialog::getText(this, i18n("Add Category"), i18n("Category name:"), QLineEdit::Normal, QString(), &ok);

    if (ok && !categoryName.isEmpty() && treeWidget->findItems(categoryName, Qt::MatchFixedString | Qt::MatchCaseSensitive, 0).isEmpty()) {
        // Create a new category item and add it to the widget
        QTreeWidgetItem *categoryItem = new QTreeWidgetItem(QTreeWidgetItem::UserType);
        categoryItem->setIcon(0, KDE::icon(QStringLiteral("folder-bookmark")));
        categoryItem->setText(0, categoryName);
        categoryItem->setText((treeWidget->columnCount() - 1), QStringLiteral("00_") + categoryName);
        categoryItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
        treeWidget->addTopLevelItem(categoryItem);
        treeWidget->sortItems((treeWidget->columnCount() - 1), Qt::AscendingOrder);

        // Add the group to the combo box
        categoryCombo->addItem(categoryName);
        categoryCombo->completionObject()->addItem(categoryName);
    }
}

void Smb4KBookmarkEditor::slotDeleteTriggered(bool /*checked*/)
{
    //
    // Get the widget
    //
    QTreeWidget *treeWidget = findChild<QTreeWidget *>(QStringLiteral("BookmarksTreeWidget"));

    //
    // Remove the bookmarks from the view and the internal list
    //
    QList<QTreeWidgetItem *> selected = treeWidget->selectedItems();

    while (!selected.isEmpty()) {
        QTreeWidgetItem *item = selected.takeFirst();
        QUrl url = item->data(0, QTreeWidgetItem::UserType).toUrl();

        QMutableListIterator<BookmarkPtr> it(m_bookmarks);

        while (it.hasNext()) {
            BookmarkPtr bookmark = it.next();

            if (bookmark->url() == url) {
                it.remove();
                break;
            }
        }

        delete item;
    }
}

void Smb4KBookmarkEditor::slotClearTriggered(bool /*checked*/)
{
    //
    // Get the widget
    //
    QTreeWidget *treeWidget = findChild<QTreeWidget *>(QStringLiteral("BookmarksTreeWidget"));

    //
    // Clear the widget and the lists
    //
    treeWidget->clear();
    m_bookmarks.clear();
    m_categories.clear();
}

void Smb4KBookmarkEditor::slotDialogAccepted()
{
    //
    // Get the widgets
    //
    KComboBox *categoryCombo = findChild<KComboBox *>(QStringLiteral("CategoryCombo"));
    KLineEdit *labelEdit = findChild<KLineEdit *>(QStringLiteral("LabelEdit"));
    KLineEdit *ipEdit = findChild<KLineEdit *>(QStringLiteral("IpEdit"));
    KLineEdit *loginEdit = findChild<KLineEdit *>(QStringLiteral("LoginEdit"));
    KLineEdit *workgroupEdit = findChild<KLineEdit *>(QStringLiteral("WorkgroupEdit"));

    //
    // Write the dialog properties to the config file
    //
    KConfigGroup group(Smb4KSettings::self()->config(), "BookmarkEditor");
    KWindowConfig::saveWindowSize(windowHandle(), group);
    group.writeEntry("LabelCompletion", labelEdit->completionObject()->items());
    group.writeEntry("LoginCompletion", loginEdit->completionObject()->items());
    group.writeEntry("IPCompletion", ipEdit->completionObject()->items());
    group.writeEntry("CategoryCompletion", categoryCombo->completionObject()->items());
    group.writeEntry("WorkgroupCompletion", workgroupEdit->completionObject()->items());

    //
    // Accept the dialog
    //
    accept();
}

void Smb4KBookmarkEditor::slotDialogRejected()
{
    //
    // Reject the dialog
    //
    reject();
}

void Smb4KBookmarkEditor::slotIconSizeChanged(int group)
{
    //
    // Get the widget
    //
    QTreeWidget *treeWidget = findChild<QTreeWidget *>(QStringLiteral("BookmarksTreeWidget"));

    //
    // Change the icon size
    //
    switch (group) {
    case KIconLoader::Small: {
        int iconSize = KIconLoader::global()->currentSize(KIconLoader::Small);
        treeWidget->setIconSize(QSize(iconSize, iconSize));
        break;
    }
    default: {
        break;
    }
    }
}

void Smb4KBookmarkEditor::slotAdjust()
{
    //
    // Get the widget
    //
    QTreeWidget *treeWidget = findChild<QTreeWidget *>(QStringLiteral("BookmarksTreeWidget"));

    //
    // Do the necessary adjustments
    //
    QTreeWidgetItemIterator it(treeWidget);

    while (*it) {
        if (!(*it)->parent()) {
            if ((*it)->data(0, QTreeWidgetItem::UserType).toUrl().isEmpty()) {
                if ((*it)->childCount() == 0) {
                    delete *it;
                }
            } else {
                BookmarkPtr bookmark = findBookmark((*it)->data(0, QTreeWidgetItem::UserType).toUrl());

                if (bookmark) {
                    bookmark->setCategoryName(QStringLiteral(""));
                }
            }
        } else {
            BookmarkPtr bookmark = findBookmark((*it)->data(0, QTreeWidgetItem::UserType).toUrl());

            if (bookmark) {
                bookmark->setCategoryName((*it)->parent()->text(0));
            }
        }
        ++it;
    }
}

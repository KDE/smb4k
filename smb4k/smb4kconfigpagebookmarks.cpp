/*
    The configuration page for bookmarks

    SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kconfigpagebookmarks.h"
#include "core/smb4kbookmarkhandler.h"

// Qt includes
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

// KF includes
#include <KIconLoader>
#include <KLocalizedString>

Smb4KConfigPageBookmarks::Smb4KConfigPageBookmarks(QWidget *parent)
    : QWidget(parent)
{
    m_bookmarksChanged = false;
    m_savingBookmarks = false;

    QHBoxLayout *layout = new QHBoxLayout(this);
    QVBoxLayout *leftLayout = new QVBoxLayout();

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setObjectName(QStringLiteral("TreeWidget"));
    m_treeWidget->headerItem()->setHidden(true);
    m_treeWidget->setColumnCount(1);
    m_treeWidget->setRootIsDecorated(false);
    m_treeWidget->setSelectionMode(QTreeWidget::SingleSelection);
    m_treeWidget->setItemsExpandable(false);
    m_treeWidget->setDragEnabled(true);
    m_treeWidget->setDragDropMode(QTreeWidget::InternalMove);
    m_treeWidget->setDropIndicatorShown(true);
    m_treeWidget->installEventFilter(this);
    m_treeWidget->viewport()->installEventFilter(this);

    connect(m_treeWidget, &QTreeWidget::currentItemChanged, this, &Smb4KConfigPageBookmarks::slotCurrentItemChanged);
    connect(m_treeWidget, &QTreeWidget::itemSelectionChanged, this, &Smb4KConfigPageBookmarks::slotItemSelectionChanged);
    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &Smb4KConfigPageBookmarks::slotItemDoubleClicked);

    m_editorWidget = new QWidget(this);
    m_editorWidget->setObjectName(QStringLiteral("EditorWidget"));
    m_editorWidget->setVisible(false);

    QGridLayout *editorWidgetLayout = new QGridLayout(m_editorWidget);
    editorWidgetLayout->setContentsMargins(0, 0, 0, 0);

    m_labelLabel = new QLabel(i18n("Label:"), m_editorWidget);
    m_labelEdit = new KLineEdit(m_editorWidget);
    m_labelEdit->setClearButtonEnabled(true);
    m_labelEdit->setCompletionMode(KCompletion::CompletionNone);

    connect(m_labelEdit, &KLineEdit::textChanged, this, &Smb4KConfigPageBookmarks::slotLabelChanged);
    connect(m_labelEdit, &KLineEdit::editingFinished, this, &Smb4KConfigPageBookmarks::slotLabelEdited);

    m_categoryLabel = new QLabel(i18n("Category:"), m_editorWidget);
    m_categoryEdit = new KComboBox(m_editorWidget);
    m_categoryEdit->setEditable(true);
    m_categoryEdit->lineEdit()->setClearButtonEnabled(true);
    m_categoryEdit->setCompletionMode(KCompletion::CompletionNone);

    connect(m_categoryEdit->lineEdit(), &KLineEdit::textChanged, this, &Smb4KConfigPageBookmarks::slotCategoryChanged);
    connect(m_categoryEdit->lineEdit(), &QLineEdit::editingFinished, this, &Smb4KConfigPageBookmarks::slotCategoryEdited);

    m_userNameLabel = new QLabel(i18n("Username:"), m_editorWidget);
    m_userNameEdit = new KLineEdit(m_editorWidget);
    m_userNameEdit->setObjectName(QStringLiteral("UserNameEdit"));
    m_userNameEdit->setClearButtonEnabled(true);
    m_userNameEdit->setCompletionMode(KCompletion::CompletionNone);

    connect(m_userNameEdit, &KLineEdit::textChanged, this, &Smb4KConfigPageBookmarks::slotUserNameChanged);
    connect(m_userNameEdit, &KLineEdit::editingFinished, this, &Smb4KConfigPageBookmarks::slotUserNameEdited);

    m_workgroupLabel = new QLabel(i18n("Workgroup:"), m_editorWidget);
    m_workgroupEdit = new KLineEdit(m_editorWidget);
    m_workgroupEdit->setClearButtonEnabled(true);
    m_workgroupEdit->setCompletionMode(KCompletion::CompletionNone);

    connect(m_workgroupEdit, &KLineEdit::textChanged, this, &Smb4KConfigPageBookmarks::slotWorkgroupChanged);
    connect(m_workgroupEdit, &KLineEdit::editingFinished, this, &Smb4KConfigPageBookmarks::slotWorkgroupEdited);

    m_ipAddressLabel = new QLabel(i18n("IP Address:"), m_editorWidget);
    m_ipAddressEdit = new KLineEdit(m_editorWidget);
    m_ipAddressEdit->setClearButtonEnabled(true);
    m_ipAddressEdit->setCompletionMode(KCompletion::CompletionNone);
    // FIXME: Do we need an input mask?

    connect(m_ipAddressEdit, &KLineEdit::textChanged, this, &Smb4KConfigPageBookmarks::slotIpAddressChanged);
    connect(m_ipAddressEdit, &KLineEdit::editingFinished, this, &Smb4KConfigPageBookmarks::slotIpAddressEdited);

    editorWidgetLayout->addWidget(m_labelLabel, 0, 0);
    editorWidgetLayout->addWidget(m_labelEdit, 0, 1);
    editorWidgetLayout->addWidget(m_categoryLabel, 1, 0);
    editorWidgetLayout->addWidget(m_categoryEdit, 1, 1);
    editorWidgetLayout->addWidget(m_userNameLabel, 2, 0);
    editorWidgetLayout->addWidget(m_userNameEdit, 2, 1);
    editorWidgetLayout->addWidget(m_workgroupLabel, 3, 0);
    editorWidgetLayout->addWidget(m_workgroupEdit, 3, 1);
    editorWidgetLayout->addWidget(m_ipAddressLabel, 4, 0);
    editorWidgetLayout->addWidget(m_ipAddressEdit, 4, 1);

    leftLayout->addWidget(m_treeWidget);
    leftLayout->addWidget(m_editorWidget);

    layout->addLayout(leftLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Vertical, this);

    m_resetButton = buttonBox->addButton(QDialogButtonBox::Reset);
    m_resetButton->setEnabled(false);
    m_resetButton->setObjectName(QStringLiteral("ResetButton"));

    connect(m_resetButton, &QPushButton::clicked, this, &Smb4KConfigPageBookmarks::slotResetButtonClicked);

    m_editButton = buttonBox->addButton(i18n("Edit"), QDialogButtonBox::ActionRole);
    m_editButton->setIcon(KDE::icon(QStringLiteral("bookmark-edit")));
    m_editButton->setEnabled(false);

    connect(m_editButton, &QPushButton::clicked, this, &Smb4KConfigPageBookmarks::slotEditButtonClicked);

    m_addCategoryButton = buttonBox->addButton(i18n("Add Category"), QDialogButtonBox::ActionRole);
    m_addCategoryButton->setIcon(KDE::icon(QStringLiteral("bookmark-add-folder")));
    m_addCategoryButton->setEnabled(!Smb4KBookmarkHandler::self()->bookmarkList().isEmpty());

    connect(m_addCategoryButton, &QPushButton::clicked, this, &Smb4KConfigPageBookmarks::slotAddCategoryButtonClicked);

    m_removeButton = buttonBox->addButton(i18n("Remove"), QDialogButtonBox::ActionRole);
    m_removeButton->setIcon(KDE::icon(QStringLiteral("bookmark-remove")));
    m_removeButton->setEnabled(false);

    connect(m_removeButton, &QPushButton::clicked, this, &Smb4KConfigPageBookmarks::slotRemoveButtonClicked);

    m_clearButton = buttonBox->addButton(i18n("Clear List"), QDialogButtonBox::ActionRole);
    m_clearButton->setIcon(KDE::icon(QStringLiteral("edit-clear")));
    m_clearButton->setEnabled(!Smb4KBookmarkHandler::self()->bookmarkList().isEmpty());

    connect(m_clearButton, &QPushButton::clicked, this, &Smb4KConfigPageBookmarks::slotClearButtonClicked);

    layout->addWidget(buttonBox);

    loadBookmarks();

    connect(this, &Smb4KConfigPageBookmarks::bookmarksModified, this, &Smb4KConfigPageBookmarks::slotEnableButtons);
    connect(Smb4KBookmarkHandler::self(), &Smb4KBookmarkHandler::updated, this, &Smb4KConfigPageBookmarks::loadBookmarks);
    connect(KIconLoader::global(), &KIconLoader::iconChanged, this, &Smb4KConfigPageBookmarks::slotIconSizeChanged);
}

Smb4KConfigPageBookmarks::~Smb4KConfigPageBookmarks()
{
}

void Smb4KConfigPageBookmarks::loadBookmarks()
{
    if (m_savingBookmarks) {
        return;
    }

    if (m_treeWidget->topLevelItemCount() != 0) {
        m_treeWidget->clear();
    }

    QStringList categories = Smb4KBookmarkHandler::self()->categoryList();
    m_categoryEdit->addItems(categories);

    if (!m_categoryEdit->contains(QStringLiteral(""))) {
        m_categoryEdit->addItem(QStringLiteral(""));
    }

    for (const QString &category : qAsConst(categories)) {
        QList<BookmarkPtr> bookmarks = Smb4KBookmarkHandler::self()->bookmarkList(category);
        QTreeWidgetItem *categoryItem = nullptr;

        if (!category.isEmpty() && !bookmarks.isEmpty()) {
            categoryItem = addCategoryItem(category);
        }

        if (!bookmarks.isEmpty()) {
            for (const BookmarkPtr &bookmark : qAsConst(bookmarks)) {
                QVariant variant = QVariant::fromValue(*bookmark.data());
                QTreeWidgetItem *bookmarkItem = nullptr;

                if (categoryItem) {
                    bookmarkItem = new QTreeWidgetItem(categoryItem, 0);
                } else {
                    bookmarkItem = new QTreeWidgetItem(m_treeWidget);
                }

                bookmarkItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
                bookmarkItem->setText(0, bookmark->displayString());
                bookmarkItem->setIcon(0, bookmark->icon());
                bookmarkItem->setData(0, TypeRole, BookmarkType);
                bookmarkItem->setData(0, DataRole, variant);
            }
        }

        if (categoryItem) {
            categoryItem->sortChildren(0, Qt::AscendingOrder);
        }
    }

    m_treeWidget->sortItems(0, Qt::AscendingOrder);

    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_treeWidget->topLevelItem(i);

        if (item->childCount() > 0) {
            item->setExpanded(true);
        }
    }

    m_bookmarksChanged = false;
    Q_EMIT bookmarksModified();
}

void Smb4KConfigPageBookmarks::saveBookmarks()
{
    if (m_bookmarksChanged) {
        QTreeWidgetItemIterator it(m_treeWidget, QTreeWidgetItemIterator::NoChildren);
        QList<BookmarkPtr> bookmarksList;

        while (*it) {
            if ((*it)->data(0, TypeRole).toInt() == BookmarkType) {
                BookmarkPtr bookmarkPtr = BookmarkPtr(new Smb4KBookmark((*it)->data(0, DataRole).value<Smb4KBookmark>()));

                if (bookmarkPtr) {
                    bookmarksList << bookmarkPtr;
                }
            }
            ++it;
        }

        m_savingBookmarks = true;
        Smb4KBookmarkHandler::self()->addBookmarks(bookmarksList, true);
        m_savingBookmarks = false;

        m_bookmarksChanged = false;
        Q_EMIT bookmarksModified();
    }
}

void Smb4KConfigPageBookmarks::setCompletionItems(const QMap<QString, QStringList> &items)
{
    m_labelEdit->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_labelEdit->completionObject()->setItems(items[QStringLiteral("LabelCompletion")]);

    m_categoryEdit->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_categoryEdit->completionObject()->setItems(items[QStringLiteral("CategoryCompletion")]);

    m_userNameEdit->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_userNameEdit->completionObject()->setItems(items[QStringLiteral("LoginCompletion")]);

    m_ipAddressEdit->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_ipAddressEdit->completionObject()->setItems(items[QStringLiteral("IpAddressCompletion")]);

    m_workgroupEdit->setCompletionMode(KCompletion::CompletionPopupAuto);
    m_workgroupEdit->completionObject()->setItems(items[QStringLiteral("WorkgroupCompletion")]);
}

QMap<QString, QStringList> Smb4KConfigPageBookmarks::getCompletionItems() const
{
    QMap<QString, QStringList> items;

    if (m_labelEdit->completionMode() != KCompletion::CompletionNone) {
        items[QStringLiteral("LabelCompletion")] = m_labelEdit->completionObject()->items();
    }

    if (m_categoryEdit->completionMode() != KCompletion::CompletionNone) {
        items[QStringLiteral("CategoryCompletion")] = m_categoryEdit->completionObject()->items();
    }

    if (m_userNameEdit->completionMode() != KCompletion::CompletionNone) {
        items[QStringLiteral("LoginCompletion")] = m_userNameEdit->completionObject()->items();
    }

    if (m_ipAddressEdit->completionMode() != KCompletion::CompletionNone) {
        items[QStringLiteral("IpAddressCompletion")] = m_ipAddressEdit->completionObject()->items();
    }

    if (m_workgroupEdit->completionMode() != KCompletion::CompletionNone) {
        items[QStringLiteral("WorkgroupCompletion")] = m_workgroupEdit->completionObject()->items();
    }

    return items;
}

bool Smb4KConfigPageBookmarks::bookmarksChanged() const
{
    return m_bookmarksChanged;
}

QTreeWidgetItem *Smb4KConfigPageBookmarks::addCategoryItem(const QString &text)
{
    QTreeWidgetItem *categoryItem = new QTreeWidgetItem(m_treeWidget, QTreeWidgetItem::UserType);
    categoryItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
    categoryItem->setText(0, text);
    categoryItem->setIcon(0, KDE::icon(QStringLiteral("folder-favorites")));
    categoryItem->setData(0, TypeRole, CategoryType);
    categoryItem->setData(0, DataRole, text);
    QFont font = categoryItem->font(0);
    font.setBold(true);
    categoryItem->setFont(0, font);
    categoryItem->setExpanded(true);

    return categoryItem;
}

void Smb4KConfigPageBookmarks::startEditingCategoryItem(QTreeWidgetItem *item)
{
    m_treeWidget->setCurrentItem(item);
    m_treeWidget->openPersistentEditor(item, 0);
    m_treeWidget->setFocus();
}

void Smb4KConfigPageBookmarks::endEditingCategoryItem(QTreeWidgetItem *item)
{
    if (m_treeWidget->isPersistentEditorOpen(item, 0)) {
        m_treeWidget->closePersistentEditor(item, 0);
        item->setData(0, DataRole, item->text(0));

        if (item->childCount() != 0) {
            for (int i = 0; i < item->childCount(); ++i) {
                Smb4KBookmark bookmark = item->child(i)->data(0, DataRole).value<Smb4KBookmark>();
                bookmark.setCategoryName(item->text(0));

                QVariant variant = QVariant::fromValue(bookmark);
                item->child(i)->setData(0, DataRole, variant);
            }
        }

        m_categoryEdit->addItem(item->text(0));
        m_categoryEdit->completionObject()->addItem(item->text(0));
    }
}

bool Smb4KConfigPageBookmarks::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == m_treeWidget->viewport()) {
        if (e->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QPointF pos = m_treeWidget->viewport()->mapFromGlobal(mouseEvent->globalPosition());
            QTreeWidgetItem *item = m_treeWidget->itemAt(pos.toPoint());
#else
            QPoint pos = m_treeWidget->viewport()->mapFromGlobal(mouseEvent->globalPos());
            QTreeWidgetItem *item = m_treeWidget->itemAt(pos);
#endif
            m_editButton->setEnabled(item != nullptr);
            m_removeButton->setEnabled(item != nullptr);

            if (!item) {
                m_treeWidget->clearSelection();
            }

            if (m_treeWidget->isPersistentEditorOpen(m_treeWidget->currentItem())) {
                endEditingCategoryItem(m_treeWidget->currentItem());
            }
        }
    } else if (obj == m_treeWidget) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);

            if (keyEvent->key() == Qt::Key_Return) {
                if (m_treeWidget->isPersistentEditorOpen(m_treeWidget->currentItem())) {
                    endEditingCategoryItem(m_treeWidget->currentItem());
                    return true;
                }
            }
        } else if (e->type() == QEvent::ChildRemoved) {
            // NOTE: Occurs after a drop event on the viewport if an item was moved.
            // We use it to get the categories right.
            QTreeWidgetItemIterator it(m_treeWidget);

            while (*it) {
                if ((*it)->data(0, TypeRole).toInt() == BookmarkType) {
                    Smb4KBookmark bookmark = (*it)->data(0, DataRole).value<Smb4KBookmark>();

                    if ((*it)->parent()) {
                        bookmark.setCategoryName((*it)->parent()->data(0, DataRole).toString());
                    } else {
                        bookmark.setCategoryName(QStringLiteral(""));
                    }

                    QVariant variant = QVariant::fromValue(bookmark);
                    (*it)->setData(0, DataRole, variant);
                }

                ++it;
            }
        }
    }

    return QObject::eventFilter(obj, e);
}

void Smb4KConfigPageBookmarks::slotResetButtonClicked(bool checked)
{
    Q_UNUSED(checked);
    m_treeWidget->clear();
    loadBookmarks();
}

void Smb4KConfigPageBookmarks::slotEditButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    if (m_treeWidget->currentItem()) {
        slotItemDoubleClicked(m_treeWidget->currentItem(), 0);
    }
}

void Smb4KConfigPageBookmarks::slotAddCategoryButtonClicked(bool checked)
{
    Q_UNUSED(checked)
    QTreeWidgetItem *item = addCategoryItem(i18n("New Category"));
    startEditingCategoryItem(item);
}

void Smb4KConfigPageBookmarks::slotRemoveButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    if (m_treeWidget->currentItem()) {
        QTreeWidgetItem *parentItem = nullptr;

        if (m_treeWidget->currentItem()->data(0, TypeRole).toInt() == BookmarkType) {
            parentItem = m_treeWidget->currentItem()->parent();
        }

        delete m_treeWidget->currentItem();
        m_treeWidget->setCurrentItem(nullptr);

        if (parentItem && parentItem->childCount() == 0) {
            delete parentItem;
        }

        m_bookmarksChanged = true;
        Q_EMIT bookmarksModified();
    }
}

void Smb4KConfigPageBookmarks::slotClearButtonClicked(bool checked)
{
    Q_UNUSED(checked);
    m_treeWidget->clear();
    m_bookmarksChanged = true;
    Q_EMIT bookmarksModified();
}

void Smb4KConfigPageBookmarks::slotCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(current);

    if (m_treeWidget->isPersistentEditorOpen(previous)) {
        endEditingCategoryItem(previous);
    }
}

void Smb4KConfigPageBookmarks::slotItemSelectionChanged()
{
    bool enableButtons = !m_treeWidget->selectedItems().isEmpty();

    m_editButton->setEnabled(enableButtons);
    m_removeButton->setEnabled(enableButtons);

    if (m_editorWidget->isVisible()) {
        m_editorWidget->setVisible(false);
        m_labelEdit->clear();
        m_categoryEdit->clear();
        m_userNameEdit->clear();
        m_workgroupEdit->clear();
        m_ipAddressEdit->clear();
    }
}

void Smb4KConfigPageBookmarks::slotItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if (item->data(0, TypeRole).toInt() == BookmarkType) {
        QString label = item->data(0, DataRole).value<Smb4KBookmark>().label();
        m_labelEdit->setText(label);

        if (!m_labelEdit->completionObject()->items().contains(label)) {
            m_labelEdit->completionObject()->addItem(label);
        }

        QString categoryName = item->data(0, DataRole).value<Smb4KBookmark>().categoryName();
        m_categoryEdit->setCurrentItem(categoryName, true);

        if (!m_categoryEdit->completionObject()->items().contains(categoryName)) {
            m_categoryEdit->completionObject()->addItem(categoryName);
        }

        QString userName = item->data(0, DataRole).value<Smb4KBookmark>().userName();
        m_userNameEdit->setText(userName);

        if (!m_userNameEdit->completionObject()->items().contains(userName)) {
            m_userNameEdit->completionObject()->addItem(userName);
        }

        QString workgroupName = item->data(0, DataRole).value<Smb4KBookmark>().workgroupName();
        m_workgroupEdit->setText(workgroupName);

        if (!m_workgroupEdit->completionObject()->items().contains(workgroupName)) {
            m_workgroupEdit->completionObject()->addItem(workgroupName);
        }

        QString hostIpAddress = item->data(0, DataRole).value<Smb4KBookmark>().hostIpAddress();
        m_ipAddressEdit->setText(hostIpAddress);

        if (!m_ipAddressEdit->completionObject()->items().contains(hostIpAddress)) {
            qDebug() << "Add completion item";
            m_ipAddressEdit->completionObject()->addItem(hostIpAddress);
        }

        m_editorWidget->setVisible(true);
    } else {
        startEditingCategoryItem(item);
    }
}

void Smb4KConfigPageBookmarks::slotLabelChanged(const QString &text)
{
    if (m_treeWidget->currentItem() && m_editorWidget->isVisible()) {
        Smb4KBookmark bookmark = m_treeWidget->currentItem()->data(0, DataRole).value<Smb4KBookmark>();
        m_bookmarksChanged = (bookmark.label() != text);
        Q_EMIT bookmarksModified();
    }
}

void Smb4KConfigPageBookmarks::slotLabelEdited()
{
    if (m_treeWidget->currentItem() && m_editorWidget->isVisible()) {
        if (m_treeWidget->currentItem()->data(0, TypeRole).toInt() == BookmarkType) {
            Smb4KBookmark bookmark = m_treeWidget->currentItem()->data(0, DataRole).value<Smb4KBookmark>();
            bookmark.setLabel(m_labelEdit->text());

            QVariant variant = QVariant::fromValue(bookmark);
            m_treeWidget->currentItem()->setData(0, DataRole, variant);

            if (m_labelEdit->completionMode() != KCompletion::CompletionNone) {
                m_labelEdit->completionObject()->addItem(m_labelEdit->text());
            }
        }
    }
}

void Smb4KConfigPageBookmarks::slotCategoryChanged(const QString &text)
{
    if (m_treeWidget->currentItem() && m_editorWidget->isVisible()) {
        if (m_treeWidget->currentItem()->data(0, TypeRole).toInt() == BookmarkType) {
            Smb4KBookmark bookmark = m_treeWidget->currentItem()->data(0, DataRole).value<Smb4KBookmark>();
            m_bookmarksChanged = (bookmark.categoryName() != text);
            Q_EMIT bookmarksModified();
        }
    }
}

void Smb4KConfigPageBookmarks::slotCategoryEdited()
{
    if (m_treeWidget->currentItem() && m_editorWidget->isVisible()) {
        if (m_treeWidget->currentItem()->data(0, TypeRole).toInt() == BookmarkType) {
            Smb4KBookmark bookmark = m_treeWidget->currentItem()->data(0, DataRole).value<Smb4KBookmark>();
            bookmark.setCategoryName(m_categoryEdit->currentText());

            QVariant variant = QVariant::fromValue(bookmark);
            m_treeWidget->currentItem()->setData(0, DataRole, variant);

            QTreeWidgetItem *categoryItem = nullptr;

            for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
                QTreeWidgetItem *item = m_treeWidget->topLevelItem(i);

                if (item->data(0, TypeRole).toInt() == CategoryType) {
                    if (item->data(0, DataRole).toString() == bookmark.categoryName()) {
                        categoryItem = item;
                        break;
                    }
                }
            }

            if (!categoryItem) {
                categoryItem = addCategoryItem(bookmark.categoryName());
            }

            if (categoryItem != m_treeWidget->currentItem()->parent()) {
                QTreeWidgetItem *itemToMove = m_treeWidget->currentItem();
                QTreeWidgetItem *parentItem = m_treeWidget->currentItem()->parent();

                parentItem->removeChild(itemToMove);

                categoryItem->addChild(itemToMove);
                categoryItem->sortChildren(0, Qt::AscendingOrder);
            }

            if (m_categoryEdit->completionMode() != KCompletion::CompletionNone) {
                m_categoryEdit->completionObject()->addItem(m_categoryEdit->currentText());
            }
        }
    }
}

void Smb4KConfigPageBookmarks::slotUserNameChanged(const QString &text)
{
    if (m_treeWidget->currentItem() && m_editorWidget->isVisible()) {
        Smb4KBookmark bookmark = m_treeWidget->currentItem()->data(0, DataRole).value<Smb4KBookmark>();
        m_bookmarksChanged = (bookmark.userName() != text);
        Q_EMIT bookmarksModified();
    }
}

void Smb4KConfigPageBookmarks::slotUserNameEdited()
{
    if (m_treeWidget->currentItem() && m_editorWidget->isVisible()) {
        if (m_treeWidget->currentItem()->data(0, TypeRole).toInt() == BookmarkType) {
            Smb4KBookmark bookmark = m_treeWidget->currentItem()->data(0, DataRole).value<Smb4KBookmark>();
            bookmark.setUserName(m_userNameEdit->text());

            QVariant variant = QVariant::fromValue(bookmark);
            m_treeWidget->currentItem()->setData(0, DataRole, variant);

            if (m_userNameEdit->completionMode() != KCompletion::CompletionNone) {
                m_userNameEdit->completionObject()->addItem(m_userNameEdit->text());
            }
        }
    }
}

void Smb4KConfigPageBookmarks::slotWorkgroupChanged(const QString &text)
{
    if (m_treeWidget->currentItem() && m_editorWidget->isVisible()) {
        Smb4KBookmark bookmark = m_treeWidget->currentItem()->data(0, DataRole).value<Smb4KBookmark>();
        m_bookmarksChanged = (bookmark.workgroupName() != text);
        Q_EMIT bookmarksModified();
    }
}

void Smb4KConfigPageBookmarks::slotWorkgroupEdited()
{
    if (m_treeWidget->currentItem() && m_editorWidget->isVisible()) {
        if (m_treeWidget->currentItem()->data(0, TypeRole).toInt() == BookmarkType) {
            Smb4KBookmark bookmark = m_treeWidget->currentItem()->data(0, DataRole).value<Smb4KBookmark>();
            bookmark.setWorkgroupName(m_workgroupEdit->text());

            QVariant variant = QVariant::fromValue(bookmark);
            m_treeWidget->currentItem()->setData(0, DataRole, variant);

            if (m_workgroupEdit->completionMode() != KCompletion::CompletionNone) {
                m_workgroupEdit->completionObject()->addItem(m_workgroupEdit->text());
            }
        }
    }
}

void Smb4KConfigPageBookmarks::slotIpAddressChanged(const QString &text)
{
    if (m_treeWidget->currentItem() && m_editorWidget->isVisible()) {
        Smb4KBookmark bookmark = m_treeWidget->currentItem()->data(0, DataRole).value<Smb4KBookmark>();
        m_bookmarksChanged = (bookmark.hostIpAddress() != text);
        Q_EMIT bookmarksModified();
    }
}

void Smb4KConfigPageBookmarks::slotIpAddressEdited()
{
    if (m_treeWidget->currentItem() && m_editorWidget->isVisible()) {
        if (m_treeWidget->currentItem()->data(0, TypeRole).toInt() == BookmarkType) {
            if (QHostAddress(m_ipAddressEdit->text()).protocol() == QAbstractSocket::UnknownNetworkLayerProtocol) {
                return;
            }

            Smb4KBookmark bookmark = m_treeWidget->currentItem()->data(0, DataRole).value<Smb4KBookmark>();
            bookmark.setHostIpAddress(m_ipAddressEdit->text());

            QVariant variant = QVariant::fromValue(bookmark);
            m_treeWidget->currentItem()->setData(0, DataRole, variant);

            if (m_ipAddressEdit->completionMode() != KCompletion::CompletionNone) {
                m_ipAddressEdit->completionObject()->addItem(m_ipAddressEdit->text());
            }

            m_bookmarksChanged = true;
            Q_EMIT bookmarksModified();
        }
    }
}

void Smb4KConfigPageBookmarks::slotEnableButtons()
{
    m_resetButton->setEnabled(m_bookmarksChanged);
    m_clearButton->setEnabled(m_treeWidget->topLevelItemCount() != 0);
    m_addCategoryButton->setEnabled(m_treeWidget->topLevelItemCount() != 0);
}

void Smb4KConfigPageBookmarks::slotIconSizeChanged(int group)
{
    switch (group) {
    case KIconLoader::Small: {
        int iconSize = KIconLoader::global()->currentSize(KIconLoader::Small);
        m_treeWidget->setIconSize(QSize(iconSize, iconSize));
        break;
    }
    default: {
        break;
    }
    }
}

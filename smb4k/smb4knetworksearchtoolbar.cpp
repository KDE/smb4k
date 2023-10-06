/*
    This class provides the network search toolbar.

    SPDX-FileCopyrightText: 2018-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4knetworksearchtoolbar.h"
#include "core/smb4kshare.h"

// Qt includes
#include <QDebug>
#include <QtGlobal>

// KDE includes

#include <KIconLoader>
#include <KLineEdit>
#include <KLocalizedString>

Smb4KNetworkSearchToolBar::Smb4KNetworkSearchToolBar(QWidget *parent)
    : QToolBar(parent)
    , m_iterator(QStringList())
{
    // Set up tool bar:
    // Use the settings suggested by the note provided in the 'Detailed Description'
    // section of KToolBar (https://api.kde.org/frameworks/kxmlgui/html/classKToolBar.html)
    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setProperty("otherToolbar", true);

    QAction *closeAction = new QAction(this);
    closeAction->setIcon(KDE::icon(QStringLiteral("window-close")));
    closeAction->setText(i18n("Close"));

    connect(closeAction, &QAction::triggered, this, &Smb4KNetworkSearchToolBar::slotCloseButtonPressed);

    addAction(closeAction);

    m_searchComboBox = new KComboBox(true, this);
    m_searchComboBox->lineEdit()->setPlaceholderText(i18n("Search string"));
    m_searchComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_searchComboBox->setCompletionMode(KCompletion::CompletionPopupAuto);

    connect(m_searchComboBox, QOverload<const QString &>::of(&KComboBox::returnPressed), this, &Smb4KNetworkSearchToolBar::slotReturnKeyPressed);
    // FIXME: Add a connection to the clearSearch() slot

    addWidget(m_searchComboBox);

    m_searchAction = new KDualAction(this);
    m_searchAction->setInactiveIcon(KDE::icon(QStringLiteral("search")));
    m_searchAction->setInactiveText(i18n("Search"));
    m_searchAction->setActiveIcon(KDE::icon(QStringLiteral("process-stop")));
    m_searchAction->setActiveText(i18n("Stop"));
    m_searchAction->setAutoToggle(false);

    connect(m_searchAction, &KDualAction::triggered, this, &Smb4KNetworkSearchToolBar::slotSearchActionTriggered);

    addAction(m_searchAction);

    m_downAction = new QAction(this);
    m_downAction->setIcon(KDE::icon(QStringLiteral("go-down-search")));
    m_downAction->setText(i18n("Item Down"));
    m_downAction->setEnabled(false);

    connect(m_downAction, &QAction::triggered, this, &Smb4KNetworkSearchToolBar::slotDownActionTriggered);

    addAction(m_downAction);

    m_upAction = new QAction(this);
    m_upAction->setIcon(KDE::icon(QStringLiteral("go-up-search")));
    m_upAction->setText(i18n("Item Up"));
    m_upAction->setEnabled(false);

    connect(m_upAction, &QAction::triggered, this, &Smb4KNetworkSearchToolBar::slotUpActionTriggered);

    addAction(m_upAction);

    m_clearAction = new QAction(this);
    m_clearAction->setIcon(KDE::icon(QStringLiteral("edit-clear-all")));
    m_clearAction->setText(i18n("Clear"));
    m_clearAction->setEnabled(false);

    connect(m_clearAction, &QAction::triggered, this, &Smb4KNetworkSearchToolBar::slotClearSearch);

    addAction(m_clearAction);
}

Smb4KNetworkSearchToolBar::~Smb4KNetworkSearchToolBar()
{
}

void Smb4KNetworkSearchToolBar::prepareInput()
{
    m_searchComboBox->lineEdit()->setFocus();
}

void Smb4KNetworkSearchToolBar::setActiveState(bool active)
{
    m_searchAction->setActive(active);
    m_searchComboBox->setEnabled(!active);
}

void Smb4KNetworkSearchToolBar::clearSearch()
{
    m_searchResults.clear();

    m_searchComboBox->clear();
    m_searchComboBox->clearEditText();

    m_downAction->setEnabled(!m_searchResults.isEmpty());
    m_upAction->setEnabled(!m_searchResults.isEmpty());
    m_clearAction->setEnabled(!m_searchResults.isEmpty());

    Q_EMIT clearSearchResults();
}

void Smb4KNetworkSearchToolBar::setSearchResults(const QList<SharePtr> &list)
{
    for (const SharePtr &share : list) {
        m_searchResults << share->url().toString();
    }

    m_searchResults.sort();

    m_iterator = m_searchResults;

    m_downAction->setEnabled(!m_searchResults.isEmpty());
    m_upAction->setEnabled(!m_searchResults.isEmpty());
    m_clearAction->setEnabled(!m_searchResults.isEmpty());
}

void Smb4KNetworkSearchToolBar::setCompletionItems(const QStringList &items)
{
    m_searchComboBox->completionObject()->setItems(items);
}

QStringList Smb4KNetworkSearchToolBar::completionItems() const
{
    return m_searchComboBox->completionObject()->items();
}

void Smb4KNetworkSearchToolBar::slotReturnKeyPressed(const QString &text)
{
    if (!text.isEmpty()) {
        m_searchComboBox->completionObject()->addItem(text);
        Q_EMIT search(text);
    }
}

void Smb4KNetworkSearchToolBar::slotSearchActionTriggered()
{
    if (!m_searchAction->isActive()) {
        if (!m_searchComboBox->currentText().isEmpty()) {
            m_searchComboBox->completionObject()->addItem(m_searchComboBox->currentText());
            Q_EMIT search(m_searchComboBox->currentText());
        }
    } else {
        Q_EMIT abort();
    }
}

void Smb4KNetworkSearchToolBar::slotCloseButtonPressed()
{
    clearSearch();

    Q_EMIT abort();
    Q_EMIT closeSearchBar();
}

void Smb4KNetworkSearchToolBar::slotDownActionTriggered()
{
    if (m_iterator.hasNext()) {
        QString url = m_iterator.next();
        Q_EMIT jumpToResult(url);
    }
}

void Smb4KNetworkSearchToolBar::slotUpActionTriggered()
{
    if (m_iterator.hasPrevious()) {
        QString url = m_iterator.previous();
        Q_EMIT jumpToResult(url);
    }
}

void Smb4KNetworkSearchToolBar::slotClearSearch()
{
    clearSearch();
}

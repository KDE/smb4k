/***************************************************************************
    Private classes for the bookmark handler
                             -------------------
    begin                : Sun Mar 20 2011
    copyright            : (C) 2011-2017 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kbookmarkhandler_p.h"
#include "smb4ksettings.h"
#include "smb4kbookmark.h"

// Qt includes
#include <QEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QHeaderView>
#include <QTreeWidgetItemIterator>
#include <QPushButton>
#include <QMenu>
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>
#include <KConfigGui/KWindowConfig>


Smb4KBookmarkDialog::Smb4KBookmarkDialog(const QList<BookmarkPtr> &bookmarks, const QStringList &groups, QWidget *parent)
: QDialog(parent)
{
  setWindowTitle(i18n("Add Bookmarks"));

  setupView();
  loadLists(bookmarks, groups);

  KConfigGroup group(Smb4KSettings::self()->config(), "BookmarkDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), group);
  m_label_edit->completionObject()->setItems(group.readEntry("LabelCompletion", QStringList()));
  m_group_combo->completionObject()->setItems(group.readEntry("GroupCompletion", m_groups));

  connect(KIconLoader::global(), SIGNAL(iconChanged(int)), SLOT(slotIconSizeChanged(int)));
}


Smb4KBookmarkDialog::~Smb4KBookmarkDialog()
{
  while (!m_bookmarks.isEmpty())
  {
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
  layout->setSpacing(5);

  QWidget *description = new QWidget(this);

  QHBoxLayout *desc_layout = new QHBoxLayout(description);
  desc_layout->setSpacing(5);
  desc_layout->setMargin(0);

  QLabel *pixmap = new QLabel(description);
  QPixmap sync_pix = KDE::icon("bookmark-new").pixmap(KIconLoader::SizeHuge);
  pixmap->setPixmap(sync_pix);
  pixmap->setAlignment(Qt::AlignBottom);

  QLabel *label = new QLabel(i18n("All listed shares will be bookmarked. To edit the label "
                                  "or group, click the respective bookmark entry."), description);
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignBottom);

  desc_layout->addWidget(pixmap, 0);
  desc_layout->addWidget(label, Qt::AlignBottom);

  m_widget = new QListWidget(this);
  m_widget->setSortingEnabled(true);
  m_widget->setSelectionMode(QAbstractItemView::SingleSelection);
  int icon_size = KIconLoader::global()->currentSize(KIconLoader::Small);
  m_widget->setIconSize(QSize(icon_size, icon_size));

  m_editors = new QWidget(this);
  m_editors->setEnabled(false);

  QGridLayout *editors_layout = new QGridLayout(m_editors);
  editors_layout->setSpacing(5);
  editors_layout->setMargin(0);

  QLabel *l_label = new QLabel(i18n("Label:"), m_editors);
  m_label_edit = new KLineEdit(m_editors);
  m_label_edit->setClearButtonShown(true);

  QLabel *g_label = new QLabel(i18n("Group:"), m_editors);
  m_group_combo = new KComboBox(true, m_editors);

  editors_layout->addWidget(l_label, 0, 0, 0);
  editors_layout->addWidget(m_label_edit, 0, 1, 0);
  editors_layout->addWidget(g_label, 1, 0, 0);
  editors_layout->addWidget(m_group_combo, 1, 1, 0);
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
  m_ok_button = buttonBox->addButton(QDialogButtonBox::Ok);
  m_cancel_button = buttonBox->addButton(QDialogButtonBox::Cancel);
  
  m_ok_button->setShortcut(Qt::CTRL|Qt::Key_Return);
  m_cancel_button->setShortcut(Qt::Key_Escape);
  
  m_ok_button->setDefault(true);

  layout->addWidget(description, 0);
  layout->addWidget(m_widget, 0);
  layout->addWidget(m_editors, 0);
  layout->addWidget(buttonBox, 0);

  setMinimumWidth(sizeHint().width() > 350 ? sizeHint().width() : 350);

  connect(m_widget, SIGNAL(itemClicked(QListWidgetItem*)), 
          this, SLOT(slotBookmarkClicked(QListWidgetItem*)));
  
  connect(m_label_edit, SIGNAL(editingFinished()), 
          this, SLOT(slotLabelEdited()));
  
  connect(m_group_combo->lineEdit(), SIGNAL(editingFinished()), 
          this, SLOT(slotGroupEdited()));
  
  connect(m_ok_button, SIGNAL(clicked()), 
          this, SLOT(slotDialogAccepted()));
  
  connect(m_cancel_button, SIGNAL(clicked()), 
          this, SLOT(reject()));
}


void Smb4KBookmarkDialog::loadLists(const QList<BookmarkPtr> &bookmarks, const QStringList &groups)
{
  // Copy the bookmarks to the internal list and add them to 
  // the list widget afterwards.
  for (const BookmarkPtr &b : bookmarks)
  {
    QListWidgetItem *item = new QListWidgetItem(b->icon(), b->unc(), m_widget);
    item->setData(Qt::UserRole, static_cast<QUrl>(b->url()));
    
    m_bookmarks << b;
  }

  m_groups = groups;
  m_group_combo->addItems(m_groups);
}


BookmarkPtr Smb4KBookmarkDialog::findBookmark(const QUrl &url)
{
  BookmarkPtr bookmark;
  
  for (const BookmarkPtr &b : m_bookmarks)
  {
    if (b->url() == url)
    {
      bookmark = b;
      break;
    }
    else
    {
      continue;
    }
  }

  return bookmark;
}


void Smb4KBookmarkDialog::slotBookmarkClicked(QListWidgetItem *bookmark_item)
{
  if (bookmark_item)
  {
    // Enable the editor widgets if necessary
    if (!m_editors->isEnabled())
    {
      m_editors->setEnabled(true);
    }
    else
    {
      // Do nothing
    }

    QUrl url = bookmark_item->data(Qt::UserRole).toUrl();
    
    BookmarkPtr bookmark = findBookmark(url);

    if (bookmark)
    {
      m_label_edit->setText(bookmark->label());
      m_group_combo->setCurrentItem(bookmark->groupName());
    }
    else
    {
      m_label_edit->clear();
      m_group_combo->clearEditText();
      m_editors->setEnabled(false);
    }
  }
  else
  {
    m_label_edit->clear();
    m_group_combo->clearEditText();
    m_editors->setEnabled(false);
  }
}


void Smb4KBookmarkDialog::slotLabelEdited()
{
  // Set the label
  QUrl url = m_widget->currentItem()->data(Qt::UserRole).toUrl();

  BookmarkPtr bookmark = findBookmark(url);

  if (bookmark)
  {
    bookmark->setLabel(m_label_edit->userText());
  }
  else
  {
    // Do nothing
  }

  // Add label to completion object
  KCompletion *completion = m_label_edit->completionObject();

  if (!m_label_edit->userText().isEmpty())
  {
    completion->addItem(m_label_edit->userText());
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkDialog::slotGroupEdited()
{
  // Set the group
  QUrl url = m_widget->currentItem()->data(Qt::UserRole).toUrl();
  
  BookmarkPtr bookmark = findBookmark(url);

  if (bookmark)
  {
    bookmark->setGroupName(m_group_combo->currentText());
  }
  else
  {
    // Do nothing
  }

  // Add the group name to the combo box
  if (m_group_combo->findText(m_group_combo->currentText()) == -1)
  {
    m_group_combo->addItem(m_group_combo->currentText());
  }
  else
  {
    // Do nothing
  }

  // Add group to completion object
  KCompletion *completion = m_group_combo->completionObject();

  if (!m_group_combo->currentText().isEmpty())
  {
    completion->addItem(m_group_combo->currentText());
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkDialog::slotDialogAccepted()
{
  KConfigGroup group(Smb4KSettings::self()->config(), "BookmarkDialog");
  KWindowConfig::saveWindowSize(windowHandle(), group);
  group.writeEntry("LabelCompletion", m_label_edit->completionObject()->items());
  group.writeEntry("GroupCompletion", m_group_combo->completionObject()->items());
  
  accept();
}


void Smb4KBookmarkDialog::slotIconSizeChanged(int group)
{
  switch (group)
  {
    case KIconLoader::Small:
    {
      int icon_size = KIconLoader::global()->currentSize(KIconLoader::Small);
      m_widget->setIconSize(QSize(icon_size, icon_size));
      break;
    }
    default:
    {
      break;
    }
  }
}



/***************************************************************************
    This file contains private helper classes for the Smb4KMounter class.
                             -------------------
    begin                : Do Jul 19 2007
    copyright            : (C) 2007-2021 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

// application specific includes
#include "smb4kmounter_p.h"
#include "smb4kbookmark.h"
#include "smb4kbookmarkhandler.h"
#include "smb4kcustomoptions.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4kglobal.h"
#include "smb4khomesshareshandler.h"
#include "smb4knotification.h"
#include "smb4ksettings.h"

// Qt includes
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QUrl>
#include <QVBoxLayout>
#include <QWindow>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KCompletion/KComboBox>
#include <KCompletion/KLineEdit>
#include <KConfigGui/KWindowConfig>
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>

using namespace Smb4KGlobal;

Smb4KMountDialog::Smb4KMountDialog(const SharePtr &share, const BookmarkPtr &bookmark, QWidget *parent)
    : QDialog(parent)
    , m_share(share)
    , m_bookmark(bookmark)
    , m_valid(false)
{
    //
    // Set the title
    //
    setWindowTitle(i18n("Mount Share"));

    //
    // Set up the view
    //
    setupView();

    //
    // Set the dialog size
    //
    create();

    KConfigGroup group(Smb4KSettings::self()->config(), "MountDialog");
    QSize dialogSize;

    if (group.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), group);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584

    setBaseSize(dialogSize);

    //
    // Get the widgets
    //
    KLineEdit *locationInput = findChild<KLineEdit *>("LocationInput");
    KLineEdit *ipInput = findChild<KLineEdit *>("IpInput");
    KLineEdit *workgroupInput = findChild<KLineEdit *>("WorkgroupInput");
    KLineEdit *labelInput = findChild<KLineEdit *>("LabelInput");
    KComboBox *categoryInput = findChild<KComboBox *>("CategoryInput");

    //
    // Fill the completion objects
    //
    if (group.hasKey("ShareNameCompletion")) {
        // For backward compatibility (since Smb4K 3.0.72).
        locationInput->completionObject()->setItems(group.readEntry("ShareNameCompletion", QStringList()));
        group.deleteEntry("ShareNameCompletion");
    } else {
        locationInput->completionObject()->setItems(group.readEntry("LocationCompletion", QStringList()));
    }

    ipInput->completionObject()->setItems(group.readEntry("IPAddressCompletion", QStringList()));
    workgroupInput->completionObject()->setItems(group.readEntry("WorkgroupCompletion", QStringList()));
    labelInput->completionObject()->setItems(group.readEntry("LabelCompletion", QStringList()));
    categoryInput->completionObject()->setItems(group.readEntry("CategoryCompletion", m_categories));
}

Smb4KMountDialog::~Smb4KMountDialog()
{
}

void Smb4KMountDialog::setupView()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    //
    // Widget for the inputs for mounting
    //
    QWidget *description = new QWidget(this);
    QHBoxLayout *descriptionLayout = new QHBoxLayout(description);

    QLabel *pixmap = new QLabel(description);
    QPixmap mount_pix = KDE::icon("view-form", QStringList("emblem-mounted")).pixmap(KIconLoader::SizeHuge);
    pixmap->setPixmap(mount_pix);
    pixmap->setAlignment(Qt::AlignBottom);

    QLabel *label = new QLabel(i18n("Enter the location and optionally the IP address and workgroup to mount a share."), description);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignBottom);

    descriptionLayout->addWidget(pixmap, 0);
    descriptionLayout->addWidget(label, Qt::AlignBottom);

    QWidget *editWidget = new QWidget(this);
    QGridLayout *editWidgetLayout = new QGridLayout(editWidget);

    QLabel *locationLabel = new QLabel(i18n("Location:"), editWidget);
    KLineEdit *locationInput = new KLineEdit(editWidget);
    locationInput->setObjectName("LocationInput");
    locationInput->setWhatsThis(
        i18n("The location of the share is provided by the Uniform Resource Locator (URL). It generally has the following syntax: "
             "[smb:]//[USER:PASSWORD@]HOST[:PORT]/SHARE. The scheme, username, password and port are optional. You should omit to enter the password here, "
             "because it is shown in cleartext."));
    //   m_share_input->setToolTip(i18n("The URL of the share"));
    locationInput->setCompletionMode(KCompletion::CompletionPopupAuto);
    locationInput->setClearButtonEnabled(true);
    locationInput->setMinimumWidth(200);
    locationInput->setFocus();

    QLabel *ipLabel = new QLabel(i18n("IP Address:"), editWidget);
    KLineEdit *ipInput = new KLineEdit(editWidget);
    ipInput->setObjectName("IpInput");
    ipInput->setWhatsThis(
        i18n("The Internet Protocol (IP) address identifies the "
             "host in the network and indicates where it is. It has two valid formats, the one "
             "known as IP version 4 (e.g. 192.168.2.11) and the version 6 format "
             "(e.g. 2001:0db8:85a3:08d3:1319:8a2e:0370:7334)."));
    //   m_ip_input->setToolTip(i18n("The IP address of the host where the share is located"));
    ipInput->setCompletionMode(KCompletion::CompletionPopupAuto);
    ipInput->setClearButtonEnabled(true);
    ipInput->setMinimumWidth(200);

    QLabel *workgroupLabel = new QLabel(i18n("Workgroup:"), editWidget);
    KLineEdit *workgroupInput = new KLineEdit(editWidget);
    workgroupInput->setObjectName("WorkgroupInput");
    workgroupInput->setWhatsThis(
        i18n("The workgroup or domain identifies the "
             "peer-to-peer computer network the host is located in."));
    //   workgroupInput->setToolTip(i18n("The workgroup where the host is located"));
    workgroupInput->setCompletionMode(KCompletion::CompletionPopupAuto);
    workgroupInput->setClearButtonEnabled(true);
    workgroupInput->setMinimumWidth(200);

    editWidgetLayout->addWidget(locationLabel, 0, 0);
    editWidgetLayout->addWidget(locationInput, 0, 1);
    editWidgetLayout->addWidget(ipLabel, 1, 0);
    editWidgetLayout->addWidget(ipInput, 1, 1);
    editWidgetLayout->addWidget(workgroupLabel, 2, 0);
    editWidgetLayout->addWidget(workgroupInput, 2, 1);

    //
    // Widget for the bookmark settings
    //
    QWidget *bookmarkWidget = new QWidget(this);
    bookmarkWidget->setObjectName("BookmarkWidget");
    bookmarkWidget->setVisible(false);
    QGridLayout *bookmarkWidgetLayout = new QGridLayout(bookmarkWidget);

    QCheckBox *addBookmark = new QCheckBox(i18n("Add this share to the bookmarks"), bookmarkWidget);
    addBookmark->setObjectName("AddBookmark");
    addBookmark->setWhatsThis(
        i18n("If you tick this checkbox, the share will be bookmarked "
             "and you can access it e.g. through the \"Bookmarks\" menu entry in the main window."));
    //   m_bookmark->setToolTip(i18n("Add this share to the bookmarks"));

    QLabel *labelLabel = new QLabel(i18n("Label:"), bookmarkWidget);
    labelLabel->setObjectName("LabelLabel");
    labelLabel->setEnabled(false);
    KLineEdit *labelInput = new KLineEdit(bookmarkWidget);
    labelInput->setObjectName("LabelInput");
    labelInput->setWhatsThis(i18n("Add a label describing your bookmark. It is then shown instead of the location."));
    labelInput->setCompletionMode(KCompletion::CompletionPopupAuto);
    labelInput->setClearButtonEnabled(true);
    labelInput->setEnabled(false);

    QLabel *categoryLabel = new QLabel(i18n("Category:"), bookmarkWidget);
    categoryLabel->setObjectName("CategoryLabel");
    categoryLabel->setEnabled(false);
    KComboBox *categoryInput = new KComboBox(true, bookmarkWidget);
    categoryInput->setObjectName("CategoryInput");
    categoryInput->setWhatsThis(i18n("Assign a category to your bookmark. This way you can organize your bookmarks."));
    categoryInput->setCompletionMode(KCompletion::CompletionPopupAuto);
    categoryInput->lineEdit()->setClearButtonEnabled(true);
    categoryInput->setEnabled(false);

    m_categories = Smb4KBookmarkHandler::self()->categoryList();
    categoryInput->addItems(m_categories);
    categoryInput->setCurrentItem("");

    bookmarkWidgetLayout->addWidget(addBookmark, 0, 0, 1, 2);
    bookmarkWidgetLayout->addWidget(labelLabel, 1, 0);
    bookmarkWidgetLayout->addWidget(labelInput, 1, 1);
    bookmarkWidgetLayout->addWidget(categoryLabel, 2, 0);
    bookmarkWidgetLayout->addWidget(categoryInput, 2, 1);

    //
    // Button box
    //
    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    QPushButton *okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    okButton->setObjectName("OkButton");
    QPushButton *cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    QPushButton *bookmarkButton =
        buttonBox->addButton(i18nc("Bookmark this share. Show input widgets in the dialog for this.", "Bookmark >>"), QDialogButtonBox::ActionRole);
    bookmarkButton->setObjectName("BookmarkButton");

    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    cancelButton->setShortcut(Qt::Key_Escape);

    layout->addWidget(description, Qt::AlignBottom);
    layout->addWidget(editWidget, 0);
    layout->addWidget(bookmarkWidget, 0);
    layout->addWidget(buttonBox, 0);

    slotChangeInputValue(locationInput->text());

    //
    // Connections
    //
    connect(locationInput, SIGNAL(textChanged(QString)), this, SLOT(slotChangeInputValue(QString)));
    connect(locationInput, SIGNAL(editingFinished()), this, SLOT(slotShareNameEntered()));
    connect(ipInput, SIGNAL(editingFinished()), this, SLOT(slotIpEntered()));
    connect(workgroupInput, SIGNAL(editingFinished()), this, SLOT(slotWorkgroupEntered()));
    connect(labelInput, SIGNAL(editingFinished()), this, SLOT(slotLabelEntered()));
    connect(categoryInput->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotCategoryEntered()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
    connect(bookmarkButton, SIGNAL(clicked()), this, SLOT(slotBookmarkButtonClicked()));
    connect(addBookmark, SIGNAL(clicked(bool)), this, SLOT(slotAddBookmarkClicked(bool)));
}

bool Smb4KMountDialog::bookmarkShare()
{
    //
    // Get the widget
    //
    QCheckBox *addBookmark = findChild<QCheckBox *>("AddBookmark");

    //
    // Return the state
    //
    return addBookmark->isChecked();
}

bool Smb4KMountDialog::validUserInput()
{
    return m_valid;
}

bool Smb4KMountDialog::validUserInput(const QString &input)
{
    //
    // Copy the input string
    //
    QString userInput = input;

    //
    // Take care of a Windows-like UNC addresses
    //
    if (userInput.startsWith(QLatin1String("\\"))) {
        userInput.replace("\\", "/");
    }

    //
    // Set the URL and adjust the scheme
    //
    QUrl smbUrl = QUrl::fromUserInput(userInput);
    smbUrl.setScheme("smb");

    //
    // Check that the URL of the share is valid
    //
    if (smbUrl.isValid() && !smbUrl.host().isEmpty() && !smbUrl.path().isEmpty() && !smbUrl.path().endsWith(QLatin1Char('/'))) {
        m_valid = true;
    } else {
        m_valid = false;
    }

    return m_valid;
}

void Smb4KMountDialog::adjustDialogSize()
{
    ensurePolished();
    layout()->activate();

    QSize newSize;
    newSize.setWidth(size().width());
    newSize.setHeight(sizeHint().height());

    resize(newSize);
}

/////////////////////////////////////////////////////////////////////////////
//  SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KMountDialog::slotChangeInputValue(const QString &_test)
{
    //
    // Find the button
    //
    QPushButton *okButton = findChild<QPushButton *>("OkButton");
    QPushButton *bookmarkButton = findChild<QPushButton *>("BookmarkButton");

    //
    // Enable/disable the buttons
    //
    bool enable = validUserInput(_test);
    okButton->setEnabled(enable);
    bookmarkButton->setEnabled(enable);
}

void Smb4KMountDialog::slotOkClicked()
{
    //
    // Get widgets
    //
    KLineEdit *locationInput = findChild<KLineEdit *>("LocationInput");
    KLineEdit *ipInput = findChild<KLineEdit *>("IpInput");
    KLineEdit *workgroupInput = findChild<KLineEdit *>("WorkgroupInput");
    QWidget *bookmarkWidget = findChild<QWidget *>("BookmarkWidget");
    KLineEdit *labelInput = findChild<KLineEdit *>("LabelInput");
    KComboBox *categoryInput = findChild<KComboBox *>("CategoryInput");

    //
    // Process the location input
    //
    if (!locationInput->text().trimmed().isEmpty()) {
        //
        // Get the URL
        //
        QString userInput = locationInput->text().trimmed();

        //
        // Check that the user input is valid
        //
        if (validUserInput(userInput)) {
            //
            // Take care of a Windows-like UNC addresses
            //
            if (userInput.startsWith(QLatin1String("\\"))) {
                userInput.replace("\\", "/");
            }

            //
            // Set the URL and adjust the scheme
            //
            QUrl smbUrl = QUrl::fromUserInput(userInput);
            smbUrl.setScheme("smb");

            //
            // Set the URL of the share
            //
            m_share->setUrl(smbUrl);
            m_share->setWorkgroupName(workgroupInput->text().trimmed());
            m_share->setHostIpAddress(ipInput->text().trimmed());
        } else {
            Smb4KNotification::invalidURLPassed();
        }
    }

    //
    // Process the bookmark, if necessary
    //
    if (bookmarkShare()) {
        m_bookmark->setUrl(m_share->url());
        m_bookmark->setWorkgroupName(m_share->workgroupName());
        m_bookmark->setHostIpAddress(m_share->hostIpAddress());
        m_bookmark->setLabel(labelInput->text().trimmed());
        m_bookmark->setCategoryName(categoryInput->currentText());
    }

    //
    // Close the bookmark widget and adjust the size
    //
    bookmarkWidget->setVisible(false);
    adjustDialogSize();

    //
    // Save the window size and the completion items
    //
    KConfigGroup group(Smb4KSettings::self()->config(), "MountDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);
    group.writeEntry("LocationCompletion", locationInput->completionObject()->items());
    group.writeEntry("IPAddressCompletion", ipInput->completionObject()->items());
    group.writeEntry("WorkgroupCompletion", workgroupInput->completionObject()->items());
    group.writeEntry("LabelCompletion", labelInput->completionObject()->items());
    group.writeEntry("CategoryCompletion", categoryInput->completionObject()->items());

    //
    // Close the dialog
    //
    accept();
}

void Smb4KMountDialog::slotCancelClicked()
{
    Smb4KMounter::self()->abort();
    reject();
}

void Smb4KMountDialog::slotBookmarkButtonClicked()
{
    //
    // Get the widget
    //
    QWidget *bookmarkWidget = findChild<QWidget *>("BookmarkWidget");

    //
    // Open / close the widget
    //
    bookmarkWidget->setVisible(!bookmarkWidget->isVisible());

    //
    // Set the height of the dialog when the widget is hidden
    //
    if (!bookmarkWidget->isVisible()) {
        adjustDialogSize();
    }
}

void Smb4KMountDialog::slotShareNameEntered()
{
    //
    // Get the widget
    //
    KLineEdit *locationInput = findChild<KLineEdit *>("LocationInput");

    //
    // Add the completion item
    //
    KCompletion *completion = locationInput->completionObject();
    QUrl url(locationInput->userText());
    url.setScheme("smb");

    if (url.isValid() && !url.isEmpty()) {
        completion->addItem(locationInput->userText());
    }
}

void Smb4KMountDialog::slotIpEntered()
{
    //
    // Get the widget
    //
    KLineEdit *ipInput = findChild<KLineEdit *>("IpInput");

    //
    // Add the completion item
    //
    KCompletion *completion = ipInput->completionObject();

    if (!ipInput->userText().isEmpty()) {
        completion->addItem(ipInput->userText());
    }
}

void Smb4KMountDialog::slotWorkgroupEntered()
{
    //
    // Get the widget
    //
    KLineEdit *workgroupInput = findChild<KLineEdit *>("WorkgroupInput");

    //
    // Add the completion item
    //
    KCompletion *completion = workgroupInput->completionObject();

    if (!workgroupInput->userText().isEmpty()) {
        completion->addItem(workgroupInput->userText());
    }
}

void Smb4KMountDialog::slotLabelEntered()
{
    //
    // Get the widget
    //
    KLineEdit *labelInput = findChild<KLineEdit *>("LabelInput");

    //
    // Add the completion item
    //
    KCompletion *completion = labelInput->completionObject();

    if (!labelInput->userText().isEmpty()) {
        completion->addItem(labelInput->userText());
    }
}

void Smb4KMountDialog::slotCategoryEntered()
{
    //
    // Get the widget
    //
    KComboBox *categoryInput = findChild<KComboBox *>("CategoryInput");

    //
    // Add the completion item
    //
    KCompletion *completion = categoryInput->completionObject();

    if (!categoryInput->currentText().isEmpty()) {
        completion->addItem(categoryInput->currentText());
    }
}

void Smb4KMountDialog::slotAddBookmarkClicked(bool on)
{
    //
    // Get the widgets
    //
    QLabel *labelLabel = findChild<QLabel *>("LabelLabel");
    KLineEdit *labelInput = findChild<KLineEdit *>("LabelInput");
    QLabel *categoryLabel = findChild<QLabel *>("CategoryLabel");
    KComboBox *categoryInput = findChild<KComboBox *>("CategoryInput");

    //
    // Enable / disable the widgets
    //
    labelLabel->setEnabled(on);
    labelInput->setEnabled(on);
    categoryLabel->setEnabled(on);
    categoryInput->setEnabled(on);
}

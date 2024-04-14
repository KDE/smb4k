/*
 *  Print dialog
 *
 *  SPDX-FileCopyrightText: 2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SMB4KPRINTDIALOG_H
#define SMB4KPRINTDIALOG_H

// application specific includes
#include "core/smb4kglobal.h"
#include "smb4kdialogs_export.h"

// Qt includes
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

// KDE includes
#include <KUrlRequester>

class SMB4KDIALOGS_EXPORT Smb4KPrintDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KPrintDialog(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    virtual ~Smb4KPrintDialog();

    /**
     * Set the printer share to which should be printed
     *
     * @param printer       The printer share
     *
     * @returns TRUE if the printer share could successfully be set
     */
    bool setPrinterShare(SharePtr printer);

protected Q_SLOTS:
    void slotPrintFile();
    void slotUrlChanged(const QString &path);
    void slotCopiesChanged(int copies);

private:
    void enablePrintButton();
    SharePtr m_printer;
    QPushButton *m_cancelButton;
    QPushButton *m_printButton;
    QLabel *m_descriptionText;
    KUrlRequester *m_fileInput;
    QSpinBox *m_copiesInput;
};

#endif

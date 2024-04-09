/*
 *  This file provides export definitions for the dialogs
 *
 *  SPDX-FileCopyrightText: 2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SMB4KDIALOGS_EXPORT_H
#define SMB4KDIALOGS_EXPORT_H

#include <QtCompilerDetection>

#ifdef SMB4KDIALOGS
#define SMB4KDIALOGS_EXPORT Q_DECL_EXPORT
#else
#define SMB4KDIALOGS_EXPORT Q_DECL_IMPORT
#endif

#endif

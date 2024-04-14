/*
 *  This file provides export definitions for the core classes
 *
 *  SPDX-FileCopyrightText: 2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SMB4KCORE_EXPORT_H
#define SMB4KCORE_EXPORT_H

#include <QtCompilerDetection>

#ifdef SMB4KCORE
#define SMB4KCORE_EXPORT Q_DECL_EXPORT
#else
#define SMB4KCORE_EXPORT Q_DECL_IMPORT
#endif

#endif

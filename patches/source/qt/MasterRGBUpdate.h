/*---------------------------------------------------------*\
| MasterRGBUpdate.h                                         |
|                                                           |
|   Checks the public GitHub releases for a newer version   |
|                                                           |
|   This file is part of the MasterRGB project              |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#ifndef MASTERRGBUPDATE_H
#define MASTERRGBUPDATE_H

#include <QString>

#include <functional>

class QObject;

namespace MasterRGBUpdate
{
    /*-----------------------------------------------------*\
    | Compares two dotted versions: >0 when a is newer,      |
    | 0 when they match and <0 when b is newer.              |
    \*-----------------------------------------------------*/
    int Compare(const QString& a, const QString& b);

    /*-----------------------------------------------------*\
    | Queries the latest published release. The callback     |
    | runs on the GUI thread with ok=false when the lookup    |
    | fails, so the caller can stay silent on startup.        |
    \*-----------------------------------------------------*/
    void Check(QObject* context,
               const QString& currentVersion,
               std::function<void(bool ok, const QString& latest, const QString& url, const QString& error)> callback);
}

#endif

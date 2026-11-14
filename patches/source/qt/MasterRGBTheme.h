/*---------------------------------------------------------*\
| MasterRGBTheme.h                                          |
|                                                           |
|   Theme selection and palette for MasterRGB               |
|                                                           |
|   This file is part of the MasterRGB project              |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <QString>

namespace MasterRGBTheme
{
    /*-----------------------------------------------------*\
    | ConfigDir returns %APPDATA%\MasterRGB                  |
    \*-----------------------------------------------------*/
    QString ConfigDir();

    /*-----------------------------------------------------*\
    | Current returns the saved preference: Auto/Light/Dark. |
    | Dark is the product default.                           |
    \*-----------------------------------------------------*/
    QString Current();

    /*-----------------------------------------------------*\
    | Save writes the preference to theme.txt                |
    \*-----------------------------------------------------*/
    void Save(const QString& name);

    /*-----------------------------------------------------*\
    | Apply sets the Qt style, palette and MasterRGB QSS.    |
    | Accepts Auto, Light or Dark and applies immediately.   |
    \*-----------------------------------------------------*/
    void Apply(const QString& name);

    /*-----------------------------------------------------*\
    | ApplySaved applies the stored preference               |
    \*-----------------------------------------------------*/
    void ApplySaved();
}

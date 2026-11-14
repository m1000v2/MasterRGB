/*---------------------------------------------------------*\
| MasterRGBUi.h                                             |
|                                                           |
|   MasterRGB pages, branding and navigation                |
|                                                           |
|   This file is part of the MasterRGB project              |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

class QMainWindow;

namespace MasterRGBUi
{
    /*-----------------------------------------------------*\
    | Attach adds the MasterRGB pages to an initialized      |
    | OpenRGB main window and applies product branding.      |
    \*-----------------------------------------------------*/
    void Attach(QMainWindow* window);
}

/*---------------------------------------------------------*\
| OpenRGBPluginInfo.h                                       |
|                                                           |
|   OpenRGB Plugin Information data structures              |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <QImage>

#define OPENRGB_PLUGIN_API_VERSION 5

/*---------------------------------------------------------*\
| Plugin Tab Location Values                                |
\*---------------------------------------------------------*/
enum
{
    OPENRGB_PLUGIN_LOCATION_TOP         = 0,    /* Top-level tab (no icon)                             */
    OPENRGB_PLUGIN_LOCATION_DEVICES     = 1,    /* Devices tab                                         */
    OPENRGB_PLUGIN_LOCATION_INFORMATION = 2,    /* Information tab                                     */
    OPENRGB_PLUGIN_LOCATION_SETTINGS    = 3,    /* Settings tab                                        */
};

struct OpenRGBPluginInfo
{
    /*-----------------------------------------------------*\
    | Plugin Details                                        |
    \*-----------------------------------------------------*/
    std::string                 Name;           /* Plugin name string                                  */
    std::string                 Description;    /* Plugin description string                           */
    std::string                 Version;        /* Plugin version string                               */
    std::string                 Commit;         /* Plugin commit (git or otherwise) string             */
    std::string                 URL;            /* Plugin project URL string                           */
    QImage                      Icon;           /* Icon image (displayed 64x64)                        */

    /*-----------------------------------------------------*\
    | Plugin Tab Configuration                              |
    \*-----------------------------------------------------*/
    unsigned int                Location;       /* Plugin tab location from Plugin Tab Location enum   */
                                                /* This field is mandatory, an invalid value will      */
                                                /* prevent plugin tab from being displayed             */
    std::string                 Label;          /* Plugin tab label string                             */
    std::string                 TabIconString;  /* Plugin tab icon string, leave empty to use custom   */
    QImage                      TabIcon;        /* Custom tab icon image (displayed 16x16)             */

    /*-----------------------------------------------------*\
    | Plugin SDK Details                                    |
    \*-----------------------------------------------------*/
    unsigned int                ProtocolVersion;/* Plugin SDK protocol version                         */
};

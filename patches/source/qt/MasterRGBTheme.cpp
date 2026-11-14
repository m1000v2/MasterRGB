/*---------------------------------------------------------*\
| MasterRGBTheme.cpp                                        |
|                                                           |
|   Theme selection and palette for MasterRGB               |
|                                                           |
|   This file is part of the MasterRGB project              |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QColor>

#include "MasterRGBTheme.h"

QString MasterRGBTheme::ConfigDir()
{
    return qEnvironmentVariable("APPDATA") + "/MasterRGB";
}

QString MasterRGBTheme::Current()
{
    QString value = "Dark";

    QFile file(ConfigDir() + "/theme.txt");

    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        value = QString::fromUtf8(file.readAll()).trimmed();
    }

    if(value.compare("Light", Qt::CaseInsensitive) == 0)
    {
        return "Light";
    }
    if(value.compare("Auto", Qt::CaseInsensitive) == 0)
    {
        return "Auto";
    }

    return "Dark";
}

void MasterRGBTheme::Save(const QString& name)
{
    QDir().mkpath(ConfigDir());

    QFile file(ConfigDir() + "/theme.txt");

    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        file.write(name.toUtf8());
        file.close();
    }
}

static bool MasterRGBThemeIsDark(const QString& name)
{
    if(name.compare("Light", Qt::CaseInsensitive) == 0)
    {
        return false;
    }

    if(name.compare("Auto", Qt::CaseInsensitive) == 0)
    {
#ifdef _WIN32
        QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);

        return settings.value("AppsUseLightTheme", 1).toInt() == 0;
#else
        return true;
#endif
    }

    return true;
}

void MasterRGBTheme::Apply(const QString& name)
{
    const bool dark = MasterRGBThemeIsDark(name);

    /*-----------------------------------------------------*\
    | Fusion gives the same widget baseline on every Windows |
    | version, so the palette and QSS are not ignored.       |
    \*-----------------------------------------------------*/
    if(QStyle* fusion = QStyleFactory::create("Fusion"))
    {
        QApplication::setStyle(fusion);
    }

    QPalette palette;

    if(dark)
    {
        palette.setColor(QPalette::WindowText,      QColor(0xE8, 0xED, 0xF7));
        palette.setColor(QPalette::Window,          QColor(0x11, 0x17, 0x22));
        palette.setColor(QPalette::Base,            QColor(0x15, 0x1E, 0x2B));
        palette.setColor(QPalette::AlternateBase,   QColor(0x1A, 0x24, 0x33));
        palette.setColor(QPalette::Text,            QColor(0xE8, 0xED, 0xF7));
        palette.setColor(QPalette::Button,          QColor(0x29, 0x36, 0x4B));
        palette.setColor(QPalette::ButtonText,      QColor(0xF4, 0xF6, 0xFC));
        palette.setColor(QPalette::BrightText,      QColor(0xFF, 0x6B, 0x6B));
        palette.setColor(QPalette::Highlight,       QColor(0x70, 0x48, 0xE8));
        palette.setColor(QPalette::HighlightedText, QColor(0xFF, 0xFF, 0xFF));
        palette.setColor(QPalette::Link,            QColor(0x8A, 0x9B, 0xFF));
        palette.setColor(QPalette::LinkVisited,     QColor(0xB9, 0xA6, 0xFF));
        palette.setColor(QPalette::ToolTipBase,     QColor(0x25, 0x2F, 0x42));
        palette.setColor(QPalette::ToolTipText,     QColor(0xFF, 0xFF, 0xFF));
        palette.setColor(QPalette::PlaceholderText, QColor(0x8C, 0x99, 0xAE));
        palette.setColor(QPalette::Dark,            QColor(0x0B, 0x10, 0x18));
        palette.setColor(QPalette::Shadow,          QColor(0x05, 0x08, 0x0D));

        palette.setColor(QPalette::Disabled, QPalette::Text,            QColor(0x69, 0x75, 0x8A));
        palette.setColor(QPalette::Disabled, QPalette::WindowText,      QColor(0x69, 0x75, 0x8A));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText,      QColor(0x69, 0x75, 0x8A));
        palette.setColor(QPalette::Disabled, QPalette::Highlight,       QColor(0x3A, 0x46, 0x5C));
        palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(0xA0, 0xA9, 0xB8));
    }
    else
    {
        palette.setColor(QPalette::WindowText,      QColor(0x20, 0x29, 0x38));
        palette.setColor(QPalette::Window,          QColor(0xF3, 0xF5, 0xF9));
        palette.setColor(QPalette::Base,            QColor(0xFF, 0xFF, 0xFF));
        palette.setColor(QPalette::AlternateBase,   QColor(0xF4, 0xF6, 0xFA));
        palette.setColor(QPalette::Text,            QColor(0x20, 0x29, 0x38));
        palette.setColor(QPalette::Button,          QColor(0xE8, 0xEC, 0xF4));
        palette.setColor(QPalette::ButtonText,      QColor(0x20, 0x29, 0x38));
        palette.setColor(QPalette::BrightText,      QColor(0xD6, 0x2A, 0x2A));
        palette.setColor(QPalette::Highlight,       QColor(0x68, 0x46, 0xD8));
        palette.setColor(QPalette::HighlightedText, QColor(0xFF, 0xFF, 0xFF));
        palette.setColor(QPalette::Link,            QColor(0x5B, 0x3C, 0xC4));
        palette.setColor(QPalette::LinkVisited,     QColor(0x7A, 0x5A, 0xD8));
        palette.setColor(QPalette::ToolTipBase,     QColor(0xFF, 0xFF, 0xFF));
        palette.setColor(QPalette::ToolTipText,     QColor(0x20, 0x29, 0x38));
        palette.setColor(QPalette::PlaceholderText, QColor(0x7A, 0x86, 0x9A));
        palette.setColor(QPalette::Dark,            QColor(0xC8, 0xCF, 0xDB));
        palette.setColor(QPalette::Shadow,          QColor(0xA8, 0xB2, 0xC2));

        palette.setColor(QPalette::Disabled, QPalette::Text,            QColor(0xA3, 0xAC, 0xBA));
        palette.setColor(QPalette::Disabled, QPalette::WindowText,      QColor(0xA3, 0xAC, 0xBA));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText,      QColor(0xA3, 0xAC, 0xBA));
        palette.setColor(QPalette::Disabled, QPalette::Highlight,       QColor(0xD8, 0xD2, 0xEE));
        palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(0x7A, 0x86, 0x9A));
    }

    QApplication::setPalette(palette);

    /*-----------------------------------------------------*\
    | Load the matching QSS. Missing files are not fatal.    |
    \*-----------------------------------------------------*/
    const QString themeName = dark ? "Dark" : "Light";
    QFile style(QCoreApplication::applicationDirPath() + "/themes/MasterRGB-" + themeName + ".qss");

    if(style.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qApp->setStyleSheet(QString::fromUtf8(style.readAll()));
        style.close();
    }
    else
    {
        qApp->setStyleSheet(QString());
    }
}

void MasterRGBTheme::ApplySaved()
{
    Apply(Current());
}

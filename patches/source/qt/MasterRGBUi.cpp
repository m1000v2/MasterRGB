/*---------------------------------------------------------*\
| MasterRGBUi.cpp                                           |
|                                                           |
|   MasterRGB pages, branding and navigation                |
|                                                           |
|   This file is part of the MasterRGB project              |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QTabBar>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QIcon>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QComboBox>
#include <QSystemTrayIcon>
#include <QCoreApplication>
#include <QStyle>
#include <QMetaObject>

#include "MasterRGBUi.h"
#include "MasterRGBTheme.h"
#include "TabLabel.h"
#include "OpenRGBFont.h"

#define MASTERRGB_VERSION "0.3.0"
#define MASTERRGB_DEFAULT_REPO "m1000v2/MasterRGB"

namespace
{
    QString SupportFilePath()
    {
        const QString userPath = MasterRGBTheme::ConfigDir() + "/support.json";

        if(QFile::exists(userPath))
        {
            return userPath;
        }

        return QCoreApplication::applicationDirPath() + "/config/support.json";
    }

    QJsonObject ReadSupport()
    {
        QFile file(SupportFilePath());

        if(!file.open(QIODevice::ReadOnly))
        {
            return QJsonObject();
        }

        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());

        return document.isObject() ? document.object() : QJsonObject();
    }

    QString SupportGithubUrl()
    {
        const QJsonObject support = ReadSupport();
        QString url = support.value("githubUrl").toString().trimmed();
        const QString repo = support.value("githubRepository").toString().trimmed();

        if(url.isEmpty() && repo.contains("/"))
        {
            url = "https://github.com/" + repo;
        }

        if(url.isEmpty())
        {
            url = QString("https://github.com/") + MASTERRGB_DEFAULT_REPO;
        }

        return url;
    }

    QString SupportReleasesUrl()
    {
        const QJsonObject support = ReadSupport();
        QString url = support.value("updatesUrl").toString().trimmed();

        if(url.isEmpty())
        {
            url = SupportGithubUrl() + "/releases";
        }

        return url;
    }

    void OpenWebUrl(const QString& value)
    {
        const QUrl url(value);

        if(url.isValid() && (url.scheme() == "https" || url.scheme() == "http"))
        {
            QDesktopServices::openUrl(url);
        }
    }

    QPushButton* MakeLinkButton(const QString& text, const QString& objectName, QWidget* parent)
    {
        QPushButton* button = new QPushButton(text, parent);
        button->setObjectName(objectName);
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumHeight(38);

        return button;
    }

    void BrandTabLabels(QMainWindow* window)
    {
        const QList<TabLabel*> labels = window->findChildren<TabLabel*>();

        for(TabLabel* tab : labels)
        {
            QLabel* name = tab->findChild<QLabel*>("name");

            if(name == nullptr)
            {
                continue;
            }

            const QString value = name->text();

            if(value.contains("About OpenRGB", Qt::CaseInsensitive) ||
               value.contains("Sobre o OpenRGB", Qt::CaseInsensitive))
            {
                name->setText("Sobre o MasterRGB");
            }
        }
    }

    void PopulateHome(QWidget* home, QTabWidget* mainTabs, QTabWidget* deviceTabs, QGridLayout* grid, QLabel* countLabel)
    {
        const int deviceCount = deviceTabs ? deviceTabs->count() : 0;

        if(deviceCount <= 0)
        {
            return;
        }

        while(QLayoutItem* item = grid->takeAt(0))
        {
            if(QWidget* widget = item->widget())
            {
                widget->deleteLater();
            }
            delete item;
        }

        countLabel->setText(QString("%1 dispositivo%2 detectado%2")
                            .arg(deviceCount)
                            .arg(deviceCount == 1 ? "" : "s"));

        QWidget* devicesPage = deviceTabs->parentWidget();

        for(int i = 0; i < deviceCount; i++)
        {
            QString deviceName = deviceTabs->tabText(i).trimmed();

            if(deviceName.isEmpty())
            {
                QWidget* button = deviceTabs->tabBar()->tabButton(i, QTabBar::LeftSide);

                if(button)
                {
                    const QList<QLabel*> pieces = button->findChildren<QLabel*>();

                    for(QLabel* piece : pieces)
                    {
                        const QString part = piece->text().trimmed();

                        if(!part.isEmpty() && !part.startsWith("<img", Qt::CaseInsensitive))
                        {
                            deviceName += " " + part;
                        }
                    }
                }
            }

            deviceName = deviceName.trimmed();

            if(deviceName.isEmpty())
            {
                deviceName = QString("Dispositivo %1").arg(i + 1);
            }

            const QString lower = deviceName.toLower();
            QString category = "PERIFÉRICO";

            if(lower.contains("dram") || lower.contains("ram") || lower.contains("spectrix"))
            {
                category = "MEMÓRIA";
            }
            else if(lower.contains("radeon") || lower.contains("geforce") || lower.contains("rtx") || lower.contains("red devil"))
            {
                category = "PLACA DE VÍDEO";
            }
            else if(lower.contains("msi") || lower.contains("motherboard") || lower.contains("placa-mãe"))
            {
                category = "PLACA-MÃE";
            }

            QFrame* card = new QFrame(home);
            card->setObjectName("MasterRGBDeviceCard");

            QVBoxLayout* cardLayout = new QVBoxLayout(card);
            cardLayout->setContentsMargins(14, 12, 14, 12);
            cardLayout->setSpacing(8);

            QLabel* categoryLabel = new QLabel(category, card);
            categoryLabel->setObjectName("MasterRGBDeviceCategory");

            QLabel* nameLabel = new QLabel(deviceName, card);
            nameLabel->setObjectName("MasterRGBDeviceName");
            nameLabel->setWordWrap(true);

            QPushButton* openButton = new QPushButton("Abrir controles", card);
            openButton->setObjectName("MasterRGBOpenDeviceButton");
            openButton->setCursor(Qt::PointingHandCursor);

            cardLayout->addWidget(categoryLabel);
            cardLayout->addWidget(nameLabel);
            cardLayout->addStretch(1);
            cardLayout->addWidget(openButton);

            grid->addWidget(card, i / 3, i % 3);

            QObject::connect(openButton, &QPushButton::clicked, card, [mainTabs, deviceTabs, devicesPage, i]()
            {
                if(deviceTabs && i < deviceTabs->count())
                {
                    deviceTabs->setCurrentIndex(i);
                }

                const int devicesIndex = mainTabs->indexOf(devicesPage);

                if(devicesIndex >= 0)
                {
                    mainTabs->setCurrentIndex(devicesIndex);
                }
            });
        }
    }

    QWidget* CreateHomePage(QMainWindow* window, QTabWidget* mainTabs, QTabWidget* deviceTabs)
    {
        QWidget* home = new QWidget(mainTabs);
        home->setObjectName("MasterRGBHomePage");

        QVBoxLayout* layout = new QVBoxLayout(home);
        layout->setContentsMargins(28, 24, 28, 24);
        layout->setSpacing(12);

        QLabel* title = new QLabel("Início", home);
        title->setObjectName("MasterRGBHomeTitle");

        QLabel* description = new QLabel(
            "O MasterRGB reúne seus dispositivos RGB em um só lugar. Este painel mostra o que foi "
            "detectado no computador e leva você direto aos controles. As abas superiores abrem "
            "Dispositivos, Informações, Configurações e Apoiar.", home);
        description->setObjectName("MasterRGBHomeDescription");
        description->setWordWrap(true);

        QLabel* countLabel = new QLabel("Detectando dispositivos...", home);
        countLabel->setObjectName("MasterRGBHomeCount");

        QGridLayout* grid = new QGridLayout();
        grid->setHorizontalSpacing(12);
        grid->setVerticalSpacing(12);

        QWidget* gridHost = new QWidget(home);
        gridHost->setObjectName("MasterRGBHomeGrid");
        gridHost->setLayout(grid);
        grid->setContentsMargins(0, 0, 0, 0);

        QHBoxLayout* actions = new QHBoxLayout();
        actions->setSpacing(10);

        QPushButton* devicesButton = MakeLinkButton("Abrir dispositivos", "MasterRGBHomeDevices", home);
        QPushButton* settingsButton = MakeLinkButton("Configurações", "MasterRGBHomeSettings", home);
        QPushButton* updatesButton = MakeLinkButton("Atualizações no GitHub", "MasterRGBHomeUpdates", home);

        actions->addWidget(devicesButton);
        actions->addWidget(settingsButton);
        actions->addWidget(updatesButton);
        actions->addStretch(1);

        layout->addWidget(title);
        layout->addWidget(description);
        layout->addWidget(countLabel);
        layout->addWidget(gridHost);
        layout->addLayout(actions);
        layout->addStretch(1);

        QWidget* devicesPage = deviceTabs ? deviceTabs->parentWidget() : nullptr;

        QObject::connect(devicesButton, &QPushButton::clicked, home, [=]()
        {
            const int index = mainTabs->indexOf(devicesPage);

            if(index >= 0)
            {
                mainTabs->setCurrentIndex(index);
            }
        });

        QObject::connect(settingsButton, &QPushButton::clicked, home, [=]()
        {
            QWidget* settingsPage = mainTabs->findChild<QWidget*>("TabSettings");
            const int index = mainTabs->indexOf(settingsPage);

            if(index >= 0)
            {
                mainTabs->setCurrentIndex(index);
            }
        });

        QObject::connect(updatesButton, &QPushButton::clicked, home, []()
        {
            OpenWebUrl(SupportReleasesUrl());
        });

        PopulateHome(home, mainTabs, deviceTabs, grid, countLabel);

        if(deviceTabs == nullptr || deviceTabs->count() <= 0)
        {
            QTimer* timer = new QTimer(home);
            timer->setInterval(750);

            QObject::connect(timer, &QTimer::timeout, home, [=]()
            {
                if(deviceTabs && deviceTabs->count() > 0)
                {
                    PopulateHome(home, mainTabs, deviceTabs, grid, countLabel);
                    timer->stop();
                }
            });

            timer->start();
        }

        (void)window;

        return home;
    }

    QWidget* CreateSupportPage(QTabWidget* mainTabs)
    {
        QWidget* page = new QWidget(mainTabs);
        page->setObjectName("MasterRGBSupportPage");

        QVBoxLayout* layout = new QVBoxLayout(page);
        layout->setContentsMargins(28, 24, 28, 24);
        layout->setSpacing(12);

        QLabel* title = new QLabel("Apoie o MasterRGB", page);
        title->setObjectName("MasterRGBSupportTitle");

        QLabel* description = new QLabel(
            "O MasterRGB é gratuito e de código aberto. Se ele ajuda no seu setup, você pode apoiar "
            "o projeto ou acompanhar as atualizações pelos links abaixo. Nada é enviado "
            "automaticamente pelo aplicativo.", page);
        description->setObjectName("MasterRGBSupportDescription");
        description->setWordWrap(true);

        layout->addWidget(title);
        layout->addWidget(description);

        const QJsonObject support = ReadSupport();
        const QString paypal = support.value("paypalUrl").toString().trimmed();

        if(!paypal.isEmpty() && QUrl(paypal).isValid())
        {
            QPushButton* donate = MakeLinkButton("Apoiar via PayPal", "MasterRGBDonateButton", page);

            QObject::connect(donate, &QPushButton::clicked, page, [paypal]()
            {
                OpenWebUrl(paypal);
            });

            layout->addWidget(donate);
        }

        const QString github = SupportGithubUrl();
        QPushButton* githubButton = MakeLinkButton("Abrir o projeto no GitHub", "MasterRGBGithubButton", page);

        QObject::connect(githubButton, &QPushButton::clicked, page, [github]()
        {
            OpenWebUrl(github);
        });

        layout->addWidget(githubButton);

        QLabel* githubLink = new QLabel(QString("<a href=\"%1\">%1</a>").arg(github), page);
        githubLink->setObjectName("MasterRGBSupportLink");
        githubLink->setTextFormat(Qt::RichText);
        githubLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
        githubLink->setOpenExternalLinks(true);
        githubLink->setWordWrap(true);
        layout->addWidget(githubLink);

        QPushButton* updates = MakeLinkButton("Verificar atualizações", "MasterRGBUpdatesButton", page);

        QObject::connect(updates, &QPushButton::clicked, page, []()
        {
            OpenWebUrl(SupportReleasesUrl());
        });

        layout->addWidget(updates);

        QLabel* note = new QLabel(
            "Atualizações são publicadas na página de Releases. O aplicativo não baixa nem "
            "instala nada sozinho.", page);
        note->setObjectName("MasterRGBSupportNote");
        note->setWordWrap(true);
        layout->addWidget(note);

        layout->addStretch(1);

        return page;
    }

    void AddHomePage(QMainWindow* window, QTabWidget* mainTabs, QTabWidget* deviceTabs)
    {
        if(mainTabs->findChild<QWidget*>("MasterRGBHomePage") != nullptr)
        {
            return;
        }

        QWidget* home = CreateHomePage(window, mainTabs, deviceTabs);
        const int index = mainTabs->insertTab(0, home, "");

        TabLabel* label = new TabLabel(OpenRGBFont::bulb, "Início", (char*)"Início", (char*)"MasterRGB");
        mainTabs->tabBar()->setTabButton(index, QTabBar::LeftSide, label);
    }

    void AddSupportPage(QTabWidget* mainTabs)
    {
        if(mainTabs->findChild<QWidget*>("MasterRGBSupportPage") != nullptr)
        {
            return;
        }

        QWidget* page = CreateSupportPage(mainTabs);
        const int index = mainTabs->addTab(page, "");

        TabLabel* label = new TabLabel(OpenRGBFont::toolbox, "Apoiar", (char*)"Apoiar", (char*)"MasterRGB");
        mainTabs->tabBar()->setTabButton(index, QTabBar::LeftSide, label);
    }

    void WireInformationPage(QMainWindow* window, QTabWidget* mainTabs, QTabWidget* infoTabs)
    {
        if(infoTabs == nullptr || window->property("mrgbInformationWired").toBool())
        {
            return;
        }

        QWidget* aboutPage = window->findChild<QWidget*>("OpenRGBSoftwareInfoPage");
        QWidget* infoContainer = infoTabs->parentWidget();

        if(aboutPage == nullptr || infoContainer == nullptr)
        {
            return;
        }

        window->setProperty("mrgbInformationWired", true);

        QObject::connect(mainTabs, &QTabWidget::currentChanged, window, [mainTabs, infoTabs, aboutPage, infoContainer](int index)
        {
            if(mainTabs->widget(index) != infoContainer)
            {
                return;
            }

            const int aboutIndex = infoTabs->indexOf(aboutPage);

            if(aboutIndex >= 0)
            {
                infoTabs->setCurrentIndex(aboutIndex);
            }
        });
    }

    void SyncThemeCombo(QMainWindow* window)
    {
        QComboBox* combo = window->findChild<QComboBox*>("ComboBoxTheme");

        if(combo == nullptr)
        {
            return;
        }

        const QString current = MasterRGBTheme::Current();

        combo->blockSignals(true);
        combo->setCurrentText(current);
        combo->blockSignals(false);
        combo->setToolTip("Tema aplicado imediatamente: Auto, Claro ou Escuro.");
    }

    void RestoreStartupProfile(QMainWindow* window)
    {
        if(!QCoreApplication::arguments().contains("--startminimized") ||
           window->property("mrgbStartupProfileStarted").toBool())
        {
            return;
        }

        window->setProperty("mrgbStartupProfileStarted", true);

        QTimer* timer = new QTimer(window);
        timer->setInterval(1000);
        timer->setProperty("tries", 0);

        QObject::connect(timer, &QTimer::timeout, window, [window, timer]()
        {
            const int tries = timer->property("tries").toInt() + 1;
            timer->setProperty("tries", tries);

            QFile file(qEnvironmentVariable("APPDATA") + "/OpenRGB/MasterRGB_LastProfile.txt");
            QString profile;

            if(file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                profile = QString::fromUtf8(file.readAll()).trimmed();
            }

            if(profile.isEmpty() || tries > 45)
            {
                timer->stop();
                return;
            }

            if(profile.endsWith(".orp", Qt::CaseInsensitive))
            {
                profile.chop(4);
            }

            QComboBox* box = window->findChild<QComboBox*>("ProfileBox");
            QPushButton* loadButton = window->findChild<QPushButton*>("ButtonLoadProfile");

            if(box == nullptr || loadButton == nullptr)
            {
                return;
            }

            const int index = box->findText(profile);

            if(index < 0)
            {
                return;
            }

            timer->stop();
            box->setCurrentIndex(index);
            QMetaObject::invokeMethod(loadButton, "click", Qt::QueuedConnection);
        });

        timer->start();
    }
}

void MasterRGBUi::Attach(QMainWindow* window)
{
    if(window == nullptr)
    {
        return;
    }

    QCoreApplication::setOrganizationName("MasterRGB");
    QCoreApplication::setApplicationName("MasterRGB");

    MasterRGBTheme::ApplySaved();

    const QString version = "MasterRGB " MASTERRGB_VERSION;
    window->setWindowTitle(version);

    const QString iconPath = QCoreApplication::applicationDirPath() + "/assets/MasterRGB-icon.png";

    if(QFile::exists(iconPath))
    {
        const QIcon icon(iconPath);
        window->setWindowIcon(icon);

        if(qApp->windowIcon().isNull())
        {
            qApp->setWindowIcon(icon);
        }
    }

    auto attachPages = [window]()
    {
        window->setWindowTitle("MasterRGB " MASTERRGB_VERSION);

        if(QFile::exists(QCoreApplication::applicationDirPath() + "/assets/MasterRGB-icon.png"))
        {
            window->setWindowIcon(QIcon(QCoreApplication::applicationDirPath() + "/assets/MasterRGB-icon.png"));
        }

        const QList<QSystemTrayIcon*> trayIcons = window->findChildren<QSystemTrayIcon*>();

        for(QSystemTrayIcon* tray : trayIcons)
        {
            tray->setToolTip("MasterRGB");

            if(QFile::exists(QCoreApplication::applicationDirPath() + "/assets/MasterRGB-icon.png"))
            {
                tray->setIcon(QIcon(QCoreApplication::applicationDirPath() + "/assets/MasterRGB-icon.png"));
            }
        }

        BrandTabLabels(window);

        QTabWidget* mainTabs = window->findChild<QTabWidget*>("MainTabBar");

        if(mainTabs == nullptr)
        {
            return;
        }

        QTabWidget* deviceTabs = window->findChild<QTabWidget*>("DevicesTabBar");
        QTabWidget* infoTabs = window->findChild<QTabWidget*>("InformationTabBar");

        AddHomePage(window, mainTabs, deviceTabs);
        AddSupportPage(mainTabs);
        WireInformationPage(window, mainTabs, infoTabs);
        SyncThemeCombo(window);
        RestoreStartupProfile(window);

        MasterRGBTheme::ApplySaved();
    };

    QTimer::singleShot(0, window, attachPages);
    QTimer::singleShot(900, window, attachPages);
}

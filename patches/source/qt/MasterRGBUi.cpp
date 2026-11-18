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
#include <QMessageBox>
#include <QPixmap>
#include <QScrollArea>
#include <QSizePolicy>

#include "MasterRGBUi.h"
#include "MasterRGBTheme.h"
#include "MasterRGBVersion.h"
#include "MasterRGBUpdate.h"
#include "TabLabel.h"
#include "OpenRGBFont.h"

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

    QLabel* MakeInfoLabel(const QString& text, const QString& objectName, QWidget* parent)
    {
        QLabel* label = new QLabel(text, parent);
        label->setObjectName(objectName);
        label->setWordWrap(true);
        label->setTextFormat(Qt::RichText);
        label->setOpenExternalLinks(true);
        label->setTextInteractionFlags(Qt::TextBrowserInteraction);

        return label;
    }

    QString VersionNotice(bool ok, const QString& latest, const QString& url, const QString& error, bool manual)
    {
        if(!ok)
        {
            return manual ? QString("Não foi possível verificar agora: %1").arg(error) : QString();
        }

        if(MasterRGBUpdate::Compare(latest, MASTERRGB_VERSION) > 0)
        {
            const QString page = url.isEmpty() ? SupportReleasesUrl() : url;

            return QString("<b>Versão %1 disponivel.</b> <a href=\"%2\">Baixar no GitHub</a>").arg(latest, page);
        }

        if(manual)
        {
            return QString("Você já está na versão mais recente (%1).").arg(MASTERRGB_VERSION);
        }

        return QString();
    }

    void ApplyUpdateNotice(QMainWindow* window, const QString& text)
    {
        QLabel* homeStatus    = window->findChild<QLabel*>("MasterRGBUpdateStatus");
        QLabel* supportStatus = window->findChild<QLabel*>("MasterRGBSupportUpdateStatus");

        for(QLabel* label : { homeStatus, supportStatus })
        {
            if(label == nullptr)
            {
                continue;
            }

            if(text.isEmpty())
            {
                label->clear();
                label->setVisible(false);
            }
            else
            {
                label->setText(text);
                label->setVisible(true);
            }
        }
    }

    void RunUpdateCheck(QMainWindow* window, bool manual)
    {
        if(window == nullptr)
        {
            return;
        }

        if(manual)
        {
            ApplyUpdateNotice(window, "Verificando atualizações...");
        }

        MasterRGBUpdate::Check(window, MASTERRGB_VERSION, [window, manual](bool ok, const QString& latest, const QString& url, const QString& error)
        {
            const QString notice = VersionNotice(ok, latest, url, error, manual);

            ApplyUpdateNotice(window, notice);

            if(!manual)
            {
                return;
            }

            QMessageBox box(window);
            box.setWindowTitle("Atualizações do MasterRGB");
            box.setIcon(ok ? QMessageBox::Information : QMessageBox::Warning);
            box.setText(ok ? QString("Versão publicada: %1\nVersão instalada: %2").arg(latest, MASTERRGB_VERSION)
                           : QString("Não foi possível consultar o GitHub.\n\n%1").arg(error));

            if(ok && MasterRGBUpdate::Compare(latest, MASTERRGB_VERSION) > 0)
            {
                box.setInformativeText("Deseja abrir a pagina da nova versão?");
                box.setStandardButtons(QMessageBox::Open | QMessageBox::Cancel);

                if(box.exec() == QMessageBox::Open)
                {
                    OpenWebUrl(url.isEmpty() ? SupportReleasesUrl() : url);
                }
            }
            else
            {
                box.exec();
            }
        });
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

    QString DeviceCategory(const QString& deviceName)
    {
        const QString lower = deviceName.toLower();

        if(lower.contains("dram") || lower.contains("ram") || lower.contains("spectrix") ||
           lower.contains("memória") || lower.contains("memória"))
        {
            return "MEMÓRIA";
        }
        if(lower.contains("radeon") || lower.contains("geforce") || lower.contains("rtx") || lower.contains("red devil"))
        {
            return "PLACA DE VÍDEO";
        }
        if(lower.contains("msi") || lower.contains("motherboard") || lower.contains("placa-mae") || lower.contains("placa-mãe"))
        {
            return "PLACA-MÃE";
        }

        return "PERIFÉRICO";
    }

    QString DeviceTabName(QTabWidget* deviceTabs, int index)
    {
        QString deviceName = deviceTabs->tabText(index).trimmed();

        if(deviceName.isEmpty())
        {
            QWidget* button = deviceTabs->tabBar()->tabButton(index, QTabBar::LeftSide);

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
            deviceName = QString("Dispositivo %1").arg(index + 1);
        }

        return deviceName;
    }

    void PopulateHome(QMainWindow* window, QWidget* home, QTabWidget* mainTabs, QTabWidget* deviceTabs, QGridLayout* grid, QLabel* countLabel)
    {
        const int deviceCount = deviceTabs ? deviceTabs->count() : 0;

        if(deviceCount <= 0)
        {
            countLabel->setText("Nenhum dispositivo detectado ainda. Use Buscar dispositivos na barra inferior.");
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
            const QString deviceName = DeviceTabName(deviceTabs, i);

            QFrame* card = new QFrame(home);
            card->setObjectName("MasterRGBDeviceCard");

            QVBoxLayout* cardLayout = new QVBoxLayout(card);
            cardLayout->setContentsMargins(14, 12, 14, 12);
            cardLayout->setSpacing(6);

            QLabel* categoryLabel = new QLabel(DeviceCategory(deviceName), card);
            categoryLabel->setObjectName("MasterRGBDeviceCategory");

            QLabel* nameLabel = new QLabel(deviceName, card);
            nameLabel->setObjectName("MasterRGBDeviceName");
            nameLabel->setWordWrap(true);

            QPushButton* openButton = new QPushButton("Ajustar", card);
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

        (void)window;
    }

    void RefreshProfileChoices(QMainWindow* window, QWidget* home)
    {
        QComboBox* source = window->findChild<QComboBox*>("ProfileBox");
        QComboBox* target = home->findChild<QComboBox*>("MasterRGBHomeProfile");
        QLabel* status    = home->findChild<QLabel*>("MasterRGBHomeProfileStatus");

        if(source == nullptr || target == nullptr)
        {
            return;
        }

        const QString previous = target->currentText();

        target->blockSignals(true);
        target->clear();

        for(int i = 0; i < source->count(); i++)
        {
            const QString name = source->itemText(i).trimmed();

            if(!name.isEmpty())
            {
                target->addItem(name);
            }
        }

        target->blockSignals(false);

        if(target->count() <= 0)
        {
            target->setEnabled(false);
            target->addItem("Nenhum perfil salvo");

            QPushButton* apply = home->findChild<QPushButton*>("MasterRGBHomeApply");

            if(apply)
            {
                apply->setEnabled(false);
            }

            if(status)
            {
                status->setText("Crie um perfil em <b>Salvar perfil</b>, na barra inferior. Depois ele aparece aqui e pode ser aplicado com um clique.");
            }

            return;
        }

        target->setEnabled(true);

        QPushButton* apply = home->findChild<QPushButton*>("MasterRGBHomeApply");

        if(apply)
        {
            apply->setEnabled(true);
        }

        const int index = target->findText(previous);

        if(index >= 0)
        {
            target->setCurrentIndex(index);
        }

        if(status && status->text().isEmpty())
        {
            status->setText("Aplicar um perfil grava as cores nos dispositivos. Depois disso, fechar o MasterRGB não apaga o efeito.");
        }
    }

    QWidget* CreateHomePage(QMainWindow* window, QTabWidget* mainTabs, QTabWidget* deviceTabs)
    {
        QWidget* home = new QWidget(mainTabs);
        home->setObjectName("MasterRGBHomePage");

        QVBoxLayout* layout = new QVBoxLayout(home);
        layout->setContentsMargins(28, 22, 28, 22);
        layout->setSpacing(14);

        QLabel* title = new QLabel("Início", home);
        title->setObjectName("MasterRGBHomeTitle");

        QLabel* description = new QLabel(
            "Seu painel de iluminação. Aplique um perfil salvo, veja o que foi detectado no "
            "computador e abra os controles de cada dispositivo.", home);
        description->setObjectName("MasterRGBHomeDescription");
        description->setWordWrap(true);

        layout->addWidget(title);
        layout->addWidget(description);

        /*-------------------------------------------------*\
        | Profile panel: the dashboard acts on the engine,  |
        | not just as a shortcut for the side tabs.         |
        \*-------------------------------------------------*/
        QFrame* profilePanel = new QFrame(home);
        profilePanel->setObjectName("MasterRGBPanel");

        QVBoxLayout* profileLayout = new QVBoxLayout(profilePanel);
        profileLayout->setContentsMargins(18, 16, 18, 16);
        profileLayout->setSpacing(8);

        QLabel* profileTitle = new QLabel("Perfil de iluminação", profilePanel);
        profileTitle->setObjectName("MasterRGBPanelTitle");

        QLabel* profileHint = new QLabel(
            "Escolha um perfil salvo e aplique sem sair desta tela.", profilePanel);
        profileHint->setObjectName("MasterRGBPanelHint");
        profileHint->setWordWrap(true);

        QHBoxLayout* profileRow = new QHBoxLayout();
        profileRow->setSpacing(10);

        QComboBox* profileCombo = new QComboBox(profilePanel);
        profileCombo->setObjectName("MasterRGBHomeProfile");
        profileCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        profileCombo->setMinimumHeight(34);

        QPushButton* applyButton = new QPushButton("Aplicar perfil", profilePanel);
        applyButton->setObjectName("MasterRGBHomeApply");
        applyButton->setCursor(Qt::PointingHandCursor);
        applyButton->setMinimumHeight(34);

        profileRow->addWidget(profileCombo, 1);
        profileRow->addWidget(applyButton);

        QLabel* profileStatus = new QLabel(profilePanel);
        profileStatus->setObjectName("MasterRGBHomeProfileStatus");
        profileStatus->setWordWrap(true);

        profileLayout->addWidget(profileTitle);
        profileLayout->addWidget(profileHint);
        profileLayout->addLayout(profileRow);
        profileLayout->addWidget(profileStatus);

        layout->addWidget(profilePanel);

        QObject::connect(applyButton, &QPushButton::clicked, home, [window, profileCombo, profileStatus]()
        {
            QComboBox* source = window->findChild<QComboBox*>("ProfileBox");
            QPushButton* load = window->findChild<QPushButton*>("ButtonLoadProfile");
            const QString name = profileCombo->currentText().trimmed();

            if(source == nullptr || load == nullptr || name.isEmpty() || profileCombo->count() <= 0)
            {
                return;
            }

            const int index = source->findText(name);

            if(index < 0)
            {
                profileStatus->setText("Esse perfil não está mais na lista. Salve-o novamente.");
                return;
            }

            source->setCurrentIndex(index);
            QMetaObject::invokeMethod(load, "click", Qt::QueuedConnection);

            profileStatus->setText(QString("Perfil <b>%1</b> enviado ao motor RGB.").arg(name.toHtmlEscaped()));
        });

        QObject::connect(profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), home, [profileStatus](int)
        {
            profileStatus->clear();
        });

        /*-------------------------------------------------*\
        | Detected devices.                                 |
        \*-------------------------------------------------*/
        QLabel* countLabel = new QLabel("Detectando dispositivos...", home);
        countLabel->setObjectName("MasterRGBHomeCount");

        QGridLayout* grid = new QGridLayout();
        grid->setHorizontalSpacing(12);
        grid->setVerticalSpacing(12);

        QWidget* gridHost = new QWidget(home);
        gridHost->setObjectName("MasterRGBHomeGrid");
        gridHost->setLayout(grid);
        grid->setContentsMargins(0, 0, 0, 0);

        layout->addWidget(countLabel);
        layout->addWidget(gridHost);

        QLabel* updateStatus = MakeInfoLabel(QString(), "MasterRGBUpdateStatus", home);
        updateStatus->setVisible(false);
        layout->addWidget(updateStatus);

        layout->addStretch(1);

        PopulateHome(window, home, mainTabs, deviceTabs, grid, countLabel);
        RefreshProfileChoices(window, home);

        if(deviceTabs == nullptr || deviceTabs->count() <= 0)
        {
            QTimer* timer = new QTimer(home);
            timer->setInterval(750);

            QObject::connect(timer, &QTimer::timeout, home, [=]()
            {
                if(deviceTabs && deviceTabs->count() > 0)
                {
                    PopulateHome(window, home, mainTabs, deviceTabs, grid, countLabel);
                    timer->stop();
                }
            });

            timer->start();
        }

        /*-------------------------------------------------*\
        | Profiles load after the dialog is built, so the   |
        | list is refreshed whenever the page is shown.     |
        \*-------------------------------------------------*/
        QObject::connect(mainTabs, &QTabWidget::currentChanged, home, [window, home, mainTabs](int index)
        {
            if(mainTabs->widget(index) == home)
            {
                RefreshProfileChoices(window, home);
            }
        });

        QTimer* profileTimer = new QTimer(home);
        profileTimer->setInterval(2000);

        QObject::connect(profileTimer, &QTimer::timeout, home, [=]()
        {
            QComboBox* source = window->findChild<QComboBox*>("ProfileBox");

            if(source && source->count() > 0)
            {
                profileTimer->stop();
            }

            RefreshProfileChoices(window, home);
        });

        profileTimer->start();

        return home;
    }

    QWidget* CreateSupportPage(QTabWidget* mainTabs)
    {
        QWidget* page = new QWidget(mainTabs);
        page->setObjectName("MasterRGBSupportPage");

        QVBoxLayout* layout = new QVBoxLayout(page);
        layout->setContentsMargins(28, 22, 28, 22);
        layout->setSpacing(10);

        QLabel* title = new QLabel("Apoie o MasterRGB", page);
        title->setObjectName("MasterRGBSupportTitle");

        QLabel* description = new QLabel(
            "O MasterRGB e gratuito e de código aberto. Se ele ajuda no seu setup, você pode apoiar "
            "o projeto com uma doacao voluntária ou acompanhar as novidades no GitHub. Nada e "
            "cobrado e nada e enviado automaticamente pelo aplicativo.", page);
        description->setObjectName("MasterRGBSupportDescription");
        description->setWordWrap(true);

        layout->addWidget(title);
        layout->addWidget(description);

        const QJsonObject support = ReadSupport();
        const QString paypal = support.value("paypalUrl").toString().trimmed();

        if(!paypal.isEmpty() && QUrl(paypal).isValid())
        {
            QHBoxLayout* donateRow = new QHBoxLayout();
            donateRow->setSpacing(16);

            QLabel* qrLabel = new QLabel(page);
            qrLabel->setObjectName("MasterRGBPayPalQr");
            qrLabel->setFixedSize(148, 148);
            qrLabel->setAlignment(Qt::AlignCenter);

            const QString qrPath = QCoreApplication::applicationDirPath() + "/assets/paypal-qr.png";
            const QPixmap qr(qrPath);

            if(!qr.isNull())
            {
                qrLabel->setPixmap(qr.scaled(144, 144, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
            else
            {
                qrLabel->setText("QR indisponivel");
            }

            QVBoxLayout* donateColumn = new QVBoxLayout();
            donateColumn->setSpacing(8);

            QLabel* donateText = new QLabel(
                "Aponte a camera do celular para o código ou use o botao, e escolha qualquer valor.", page);
            donateText->setObjectName("MasterRGBSupportText");
            donateText->setWordWrap(true);

            QPushButton* donate = MakeLinkButton("Apoiar via PayPal", "MasterRGBDonateButton", page);
            donate->setMaximumWidth(240);

            QObject::connect(donate, &QPushButton::clicked, page, [paypal]()
            {
                OpenWebUrl(paypal);
            });

            donateColumn->addWidget(donateText);
            donateColumn->addWidget(donate);
            donateColumn->addStretch(1);

            donateRow->addWidget(qrLabel);
            donateRow->addLayout(donateColumn, 1);

            layout->addLayout(donateRow);
        }

        QPushButton* githubButton = MakeLinkButton("Abrir o projeto no GitHub", "MasterRGBGithubButton", page);
        githubButton->setMaximumWidth(280);

        QObject::connect(githubButton, &QPushButton::clicked, page, []()
        {
            OpenWebUrl(SupportGithubUrl());
        });

        layout->addWidget(githubButton);

        QLabel* githubLink = MakeInfoLabel(
            QString("<a href=\"%1\">%1</a>").arg(SupportGithubUrl().toHtmlEscaped()),
            "MasterRGBSupportLink", page);
        layout->addWidget(githubLink);

        QPushButton* updates = MakeLinkButton("Verificar atualizações", "MasterRGBUpdatesButton", page);
        updates->setMaximumWidth(280);

        QObject::connect(updates, &QPushButton::clicked, page, [page]()
        {
            QMainWindow* window = qobject_cast<QMainWindow*>(page->window());
            RunUpdateCheck(window, true);
        });

        layout->addWidget(updates);

        QLabel* updateStatus = MakeInfoLabel(QString(), "MasterRGBSupportUpdateStatus", page);
        updateStatus->setVisible(false);
        layout->addWidget(updateStatus);

        QLabel* note = new QLabel(
            "Atualizações são publicadas na pagina de Releases. O aplicativo avisa quando existe "
            "versão nova, mas so baixa e instala com a sua confirmacao.", page);
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

            /*---------------------------------------------*\
            | Keep the last device information visible; the |
            | "Sobre" sub-tab stays one click away.         |
            \*---------------------------------------------*/
            if(infoTabs->currentIndex() < 0 || infoTabs->widget(infoTabs->currentIndex()) == nullptr)
            {
                const int aboutIndex = infoTabs->indexOf(aboutPage);

                if(aboutIndex >= 0)
                {
                    infoTabs->setCurrentIndex(aboutIndex);
                }
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

    /*-----------------------------------------------------*\
    | Silent update check once the window is ready. Missing  |
    | network or no release yet stays invisible.            |
    \*-----------------------------------------------------*/
    QTimer::singleShot(5000, window, [window]()
    {
        RunUpdateCheck(window, false);
    });
}

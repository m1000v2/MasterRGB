[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$SourceDir
)

$ErrorActionPreference = 'Stop'
$dialog = Join-Path $SourceDir 'qt\OpenRGBDialog\OpenRGBDialog.cpp'
if (-not (Test-Path -LiteralPath $dialog)) {
    throw "Não encontrei o arquivo de interface Qt esperado: $dialog"
}

$text = [IO.File]::ReadAllText($dialog)
$marker = 'MASTER_RGB_RUNTIME_BRANDING_023_BRANDING_THEME_FIX'
if ($text.Contains($marker)) {
    Write-Host 'Branding, tema interno e suporte MasterRGB já aplicados.' -ForegroundColor Yellow
    exit 0
}

# When rebuilding in the same workspace, remove the old 0.2.0/0.2.1 runtime block
# before inserting this revision. The source files themselves are preserved.
$legacyPattern = '(?ms)^\s*// MASTER_RGB_RUNTIME_BRANDING_0(?:20_ICON|21_SUPPORT_TAB|22_UI_REPAIR)[^\r\n]*\r?\n\s{4}\{.*?^\s{4}\}\s*\r?\n'
$legacyMatch = [regex]::Match($text, $legacyPattern)
if ($legacyMatch.Success) {
    $text = [regex]::Replace($text, $legacyPattern, '', 1)
    Write-Host 'Removida a injeção visual antiga; aplicando a revisão 0.2.3.' -ForegroundColor Yellow
}

$includes = @(
    '#include <QFile>',
    '#include <QDir>',
    '#include <QCoreApplication>',
    '#include <QIcon>',
    '#include <QApplication>',
    '#include <QTabWidget>',
    '#include <QTabBar>',
    '#include <QLabel>',
    '#include <QPushButton>',
    '#include <QLineEdit>',
    '#include <QVBoxLayout>',
    '#include <QFormLayout>',
    '#include <QGridLayout>',
    '#include <QFrame>',
    '#include <QHBoxLayout>',
    '#include <QDesktopServices>',
    '#include <QUrl>',
    '#include <QJsonDocument>',
    '#include <QJsonObject>',
    '#include <QMessageBox>',
    '#include <QSysInfo>',
    '#include <QAbstractButton>',
    '#include <QAction>',
    '#include <QSystemTrayIcon>',
    '#include <QMainWindow>',
    '#include <QDialog>',
    '#include <QGroupBox>',
    '#include <QDockWidget>',
    '#include <QTimer>',
    '#include <QComboBox>',
    '#include <QTextBrowser>',
    '#include <QTextEdit>',
    '#include <QMenu>',
    '#include <QPixmap>',
    '#include <QFileInfo>',
    '#include <memory>'
)
foreach ($inc in $includes) {
    if (-not $text.Contains($inc)) {
        $m = [regex]::Match($text, '(?m)^#include [^\r\n]+')
        if (-not $m.Success) { throw 'Não encontrei a área de includes em OpenRGBDialog.cpp.' }
        $text = $text.Insert($m.Index + $m.Length, "`r`n$inc")
    }
}

$block = @'
    // MASTER_RGB_RUNTIME_BRANDING_023_BRANDING_THEME_FIX — preserve custom OpenRGB widgets/fonts.
    {
        QCoreApplication::setOrganizationName("MasterRGB");
        QCoreApplication::setApplicationName("MasterRGB");

        const QString mrgbConfigDir = qEnvironmentVariable("APPDATA") + "/MasterRGB";
        QDir().mkpath(mrgbConfigDir);

        QString mrgbTheme = "Dark";
        QFile mrgbThemePref(mrgbConfigDir + "/theme.txt");
        if (mrgbThemePref.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            mrgbTheme = QString::fromUtf8(mrgbThemePref.readAll()).trimmed();
        }
        if (mrgbTheme.compare("Light", Qt::CaseInsensitive) != 0)
        {
            mrgbTheme = "Dark";
        }
        QFile mrgbThemeFile(QCoreApplication::applicationDirPath() + "/themes/MasterRGB-" + mrgbTheme + ".qss");
        if (mrgbThemeFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            // The MasterRGB QSS intentionally avoids global QWidget/* font and
            // background selectors so OpenRGB's custom icon fonts and paintEvent
            // widgets (including the HSV color wheel) keep drawing correctly.
            qApp->setStyleSheet(QString::fromUtf8(mrgbThemeFile.readAll()));
        }

        setWindowTitle("MasterRGB");
        const QString mrgbIconPath = QCoreApplication::applicationDirPath() + "/assets/MasterRGB-icon.png";
        if (QFile::exists(mrgbIconPath))
        {
            setWindowIcon(QIcon(mrgbIconPath));
            if (qApp->windowIcon().isNull()) qApp->setWindowIcon(QIcon(mrgbIconPath));
        }

        // Apply visible product branding after Qt creates/translates every page.
        // Keep the upstream copyright attribution intact.
        QTimer::singleShot(0, this, [this, mrgbIconPath]()
        {
            setWindowTitle("MasterRGB");
            auto mrgbBrandText = [](QString value) -> QString
            {
                if (value.contains("openrgb.org", Qt::CaseInsensitive))
                    return QString("Projeto-base OpenRGB — licença GPL-2.0");
                if (value.contains("gitlab.com/CalcProgrammer1/OpenRGB", Qt::CaseInsensitive))
                    return QString("Código-fonte do motor-base OpenRGB (GPL-2.0)");
                value.replace("About OpenRGB", "Sobre o MasterRGB", Qt::CaseInsensitive);
                value.replace("Sobre o OpenRGB", "Sobre o MasterRGB", Qt::CaseInsensitive);
                value.replace("OpenRGB, a free and open source RGB lighting control", "MasterRGB — controle unificado de iluminação RGB", Qt::CaseInsensitive);
                value.replace("OpenRGB, uma ferramenta de código aberto para controle de RGB", "MasterRGB — controle unificado de iluminação RGB", Qt::CaseInsensitive);
                value.replace("OpenRGB", "MasterRGB", Qt::CaseInsensitive);
                return value;
            };

            auto mrgbApplyBranding = [this, mrgbIconPath]()
            {
                setWindowTitle("MasterRGB");
                const auto brandText = [](QString value) -> QString
                {
                    if (value.contains("openrgb.org", Qt::CaseInsensitive))
                        return QString("Projeto-base OpenRGB — licença GPL-2.0");
                    if (value.contains("gitlab.com/CalcProgrammer1/OpenRGB", Qt::CaseInsensitive))
                        return QString("Código-fonte do motor-base OpenRGB (GPL-2.0)");
                    value.replace("About OpenRGB", "Sobre o MasterRGB", Qt::CaseInsensitive);
                    value.replace("Sobre o OpenRGB", "Sobre o MasterRGB", Qt::CaseInsensitive);
                    value.replace("OpenRGB, a free and open source RGB lighting control", "MasterRGB — controle unificado de iluminação RGB", Qt::CaseInsensitive);
                    value.replace("OpenRGB, uma ferramenta de código aberto para controle de RGB", "MasterRGB — controle unificado de iluminação RGB", Qt::CaseInsensitive);
                    value.replace("OpenRGB", "MasterRGB", Qt::CaseInsensitive);
                    return value;
                };

                const QList<QLabel*> labels = findChildren<QLabel*>();
                for (QLabel* label : labels)
                {
                    const QString original = label->text();
                    if (original.contains("Direitos autorais", Qt::CaseInsensitive) || original.contains("Copyright", Qt::CaseInsensitive))
                    {
                        label->setText("Créditos do motor-base: Adam Honse e equipe OpenRGB. MasterRGB é uma adaptação independente.");
                    }
                    else if (original.contains("Valor da versão do Qt", Qt::CaseInsensitive) || original.contains("Qt version value", Qt::CaseInsensitive))
                    {
                        label->setText(QString(QT_VERSION_STR));
                    }
                    else if (original.contains("Valor da versão do SO", Qt::CaseInsensitive) || original.contains("OS version value", Qt::CaseInsensitive))
                    {
                        label->setText(QSysInfo::prettyProductName());
                    }
                    else if (original.contains("0.9+ (1.0rc2wr0)", Qt::CaseInsensitive) || original.contains("MasterRGB 0.2.3 Preview", Qt::CaseInsensitive))
                    {
                        label->setText("MasterRGB 0.2.3 Preview");
                    }
                    else if (original.contains("OpenRGB", Qt::CaseInsensitive) || original.contains("openrgb.org", Qt::CaseInsensitive))
                    {
                        label->setText(brandText(original));
                    }
                }

                // The Information sidebar uses custom TabLabel widgets. Their text
                // is a child QLabel, not QTabWidget::tabText(), so update both.
                const QList<QTabWidget*> allTabs = findChildren<QTabWidget*>();
                for (QTabWidget* tabs : allTabs)
                {
                    for (int i = 0; i < tabs->count(); ++i)
                    {
                        QString tabText = tabs->tabText(i);
                        if (tabText.contains("OpenRGB", Qt::CaseInsensitive))
                            tabs->setTabText(i, brandText(tabText));
                        QString tooltip = tabs->tabBar()->tabToolTip(i);
                        if (tooltip.contains("OpenRGB", Qt::CaseInsensitive))
                            tabs->tabBar()->setTabToolTip(i, brandText(tooltip));
                        QWidget* tabButton = tabs->tabBar()->tabButton(i, QTabBar::LeftSide);
                        if (tabButton)
                        {
                            for (QLabel* label : tabButton->findChildren<QLabel*>())
                            {
                                QString value = label->text();
                                if (!value.isEmpty() && !value.startsWith("<img", Qt::CaseInsensitive) && value.contains("OpenRGB", Qt::CaseInsensitive))
                                    label->setText(brandText(value));
                            }
                        }
                    }
                }

                const QList<QTextBrowser*> browsers = findChildren<QTextBrowser*>();
                for (QTextBrowser* browser : browsers)
                {
                    QString html = browser->toHtml();
                    if (html.contains("OpenRGB", Qt::CaseInsensitive))
                    {
                        html.replace("OpenRGB, a free and open source RGB lighting control", "MasterRGB — controle unificado de iluminação RGB", Qt::CaseInsensitive);
                        html.replace(">OpenRGB<", ">MasterRGB<", Qt::CaseInsensitive);
                        browser->setHtml(html);
                    }
                }
                const QList<QTextEdit*> textEdits = findChildren<QTextEdit*>();
                for (QTextEdit* edit : textEdits)
                {
                    if (qobject_cast<QTextBrowser*>(edit)) continue;
                    QString value = edit->toPlainText();
                    if (value.contains("OpenRGB", Qt::CaseInsensitive))
                    {
                        value.replace("OpenRGB", "MasterRGB", Qt::CaseInsensitive);
                        edit->setPlainText(value);
                    }
                }

                const QList<QAbstractButton*> buttons = findChildren<QAbstractButton*>();
                for (QAbstractButton* button : buttons)
                    if (button->text().contains("OpenRGB", Qt::CaseInsensitive)) button->setText(brandText(button->text()));
                const QList<QAction*> actions = findChildren<QAction*>();
                for (QAction* action : actions)
                    if (action->text().contains("OpenRGB", Qt::CaseInsensitive)) action->setText(brandText(action->text()));
                const QList<QMainWindow*> childWindows = findChildren<QMainWindow*>();
                for (QMainWindow* child : childWindows) child->setWindowTitle(brandText(child->windowTitle()));
                const QList<QDialog*> childDialogs = findChildren<QDialog*>();
                for (QDialog* child : childDialogs) child->setWindowTitle(brandText(child->windowTitle()));
                const QList<QGroupBox*> groupBoxes = findChildren<QGroupBox*>();
                for (QGroupBox* group : groupBoxes)
                    if (group->title().contains("OpenRGB", Qt::CaseInsensitive)) group->setTitle(brandText(group->title()));
                const QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();
                for (QDockWidget* dock : dockWidgets) dock->setWindowTitle(brandText(dock->windowTitle()));
                const QList<QSystemTrayIcon*> trayIcons = findChildren<QSystemTrayIcon*>();
                for (QSystemTrayIcon* tray : trayIcons)
                {
                    tray->setToolTip("MasterRGB");
                    if (QFile::exists(mrgbIconPath)) tray->setIcon(QIcon(mrgbIconPath));
                }
            };
            mrgbApplyBranding();
            // The original language-change handler may rebuild labels after the
            // constructor. Re-apply branding briefly so the product title and
            // About label cannot fall back to OpenRGB during startup.
            QTimer* mrgbBrandTimer = new QTimer(this);
            mrgbBrandTimer->setInterval(450);
            auto mrgbBrandPasses = std::make_shared<int>(0);
            QObject::connect(mrgbBrandTimer, &QTimer::timeout, this, [mrgbApplyBranding, mrgbBrandTimer, mrgbBrandPasses]()
            {
                mrgbApplyBranding();
                if (++(*mrgbBrandPasses) >= 24) mrgbBrandTimer->stop();
            });
            mrgbBrandTimer->start();

            QList<QTabWidget*> tabWidgets = findChildren<QTabWidget*>();
            // The source UI names this exact top-level widget MainTabBar.
            // Resolve it first; text-based detection remains a fallback for forks.
            QTabWidget* mainTabs = findChild<QTabWidget*>("MainTabBar");
            int devicesTopIndex = -1;
            auto mrgbTabLabelText = [](QTabWidget* tabs, int index) -> QString
            {
                QString result = tabs->tabText(index);
                QWidget* tabButton = tabs->tabBar()->tabButton(index, QTabBar::LeftSide);
                if (tabButton)
                {
                    const QList<QLabel*> tabLabels = tabButton->findChildren<QLabel*>();
                    for (QLabel* label : tabLabels)
                    {
                        const QString part = label->text().trimmed();
                        // TabLabel uses an HTML <img> QLabel for its device icon.
                        // Never leak that markup into device cards or tab matching.
                        if (!part.isEmpty() && !part.startsWith("<img", Qt::CaseInsensitive))
                            result += " " + part;
                    }
                }
                return result;
            };

            if (!mainTabs)
            {
                for (QTabWidget* tabs : tabWidgets)
                {
                    bool hasDevices = false;
                    bool hasInformation = false;
                    bool hasSdk = false;
                    bool hasSettings = false;
                    for (int i = 0; i < tabs->count(); ++i)
                    {
                        const QString tabName = mrgbTabLabelText(tabs, i).toLower();
                        hasDevices = hasDevices || tabName.contains("devices") || tabName.contains("dispositivos");
                        hasInformation = hasInformation || tabName.contains("information") || tabName.contains("informações") || tabName.contains("informacoes");
                        hasSdk = hasSdk || tabName.contains("sdk");
                        hasSettings = hasSettings || tabName.contains("settings") || tabName.contains("configurações") || tabName.contains("configuracoes");
                    }
                    if (hasDevices && hasInformation && hasSdk && hasSettings)
                    {
                        mainTabs = tabs;
                        break;
                    }
                }
            }
            if (!mainTabs) mainTabs = findChild<QTabWidget*>("MainTabs");
            if (mainTabs && devicesTopIndex < 0)
            {
                for (int i = 0; i < mainTabs->count(); ++i)
                {
                    const QString tabName = mrgbTabLabelText(mainTabs, i).toLower();
                    if (tabName.contains("devices") || tabName.contains("dispositivos"))
                    {
                        devicesTopIndex = i;
                        break;
                    }
                }
            }

            // The Information area intentionally contains per-device information
            // pages as well as the About page. Its tab labels are custom widgets,
            // so inspect both tabText() and the QLabel inside each tab button.
            // Move About to the top; do not hide the device information pages.
            for (QTabWidget* tabs : tabWidgets)
            {
                for (int i = 0; i < tabs->count(); ++i)
                {
                    const QString tabName = mrgbTabLabelText(tabs, i);
                    if (tabName.contains("Sobre", Qt::CaseInsensitive) ||
                        tabName.contains("About", Qt::CaseInsensitive))
                    {
                        QWidget* tabButton = tabs->tabBar()->tabButton(i, QTabBar::LeftSide);
                        if (!tabButton)
                        {
                            const QString branded = mrgbBrandText(tabs->tabText(i));
                            if (branded != tabs->tabText(i)) tabs->setTabText(i, branded);
                        }
                        if (tabs != mainTabs && i > 0) tabs->tabBar()->moveTab(i, 0);
                        break;
                    }
                }
            }

            if (mainTabs)
            {
                // Add a first-party MasterRGB home page. The original Devices
                // page remains intact and is opened when a card is selected.
                bool dashboardExists = false;
                for (int i = 0; i < mainTabs->count(); ++i)
                {
                    const QString tabName = mrgbTabLabelText(mainTabs, i).toLower();
                    if (tabName.contains("visão geral") || tabName.contains("visao geral") || tabName.contains("dashboard")) dashboardExists = true;
                    if (devicesTopIndex < 0 && (tabName.contains("devices") || tabName.contains("dispositivos"))) devicesTopIndex = i;
                }

                if (!dashboardExists)
                {
                    QTabWidget* deviceTabs = findChild<QTabWidget*>("DevicesTabBar");
                    QWidget* dashboard = new QWidget(mainTabs);
                    dashboard->setObjectName("MasterRGBDashboardPage");
                    QVBoxLayout* dashboardLayout = new QVBoxLayout(dashboard);
                    dashboardLayout->setContentsMargins(24, 22, 24, 22);
                    dashboardLayout->setSpacing(14);

                    QLabel* dashboardTitle = new QLabel("Central de iluminação", dashboard);
                    dashboardTitle->setObjectName("MasterRGBDashboardTitle");
                    QLabel* dashboardSubtitle = new QLabel("Selecione um dispositivo para abrir seus controles RGB.", dashboard);
                    dashboardSubtitle->setObjectName("MasterRGBDashboardSubtitle");
                    dashboardLayout->addWidget(dashboardTitle);
                    dashboardLayout->addWidget(dashboardSubtitle);

                    QGridLayout* deviceGrid = new QGridLayout();
                    deviceGrid->setHorizontalSpacing(12);
                    deviceGrid->setVerticalSpacing(12);
                    QLabel* deviceCountLabel = new QLabel("Detectando dispositivos...", dashboard);
                    deviceCountLabel->setObjectName("MasterRGBDashboardCount");
                    dashboardLayout->addWidget(deviceCountLabel);

                    // Device discovery is asynchronous. Populate the cards when
                    // the controller tabs are actually available, rather than
                    // freezing an empty dashboard during the first event-loop tick.
                    auto populateMasterRGBDashboard = [dashboard, deviceTabs, mainTabs, devicesTopIndex,
                                                       deviceGrid, deviceCountLabel, mrgbTabLabelText]() -> bool
                    {
                        const int deviceCount = deviceTabs ? deviceTabs->count() : 0;
                        if (deviceCount <= 0) return false;

                        while (QLayoutItem* item = deviceGrid->takeAt(0))
                        {
                            if (QWidget* widget = item->widget()) widget->deleteLater();
                            delete item;
                        }

                        deviceCountLabel->setText(QString("%1 dispositivos detectados").arg(deviceCount));
                        for (int i = 0; i < deviceCount; ++i)
                        {
                            QString deviceName = mrgbTabLabelText(deviceTabs, i).trimmed();
                            if (deviceName.isEmpty()) deviceName = QString("Dispositivo %1").arg(i + 1);
                            QString lowerName = deviceName.toLower();
                            QString category = "PERIFÉRICO";
                            if (lowerName.contains("dram") || lowerName.contains("ram")) category = "MEMÓRIA";
                            else if (lowerName.contains("radeon") || lowerName.contains("geforce") || lowerName.contains("rtx") || lowerName.contains("red devil")) category = "PLACA DE VÍDEO";
                            else if (lowerName.contains("msi") || lowerName.contains("motherboard") || lowerName.contains("placa-mãe")) category = "PLACA-MÃE";

                            QFrame* card = new QFrame(dashboard);
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
                            cardLayout->addWidget(categoryLabel);
                            cardLayout->addWidget(nameLabel);
                            cardLayout->addStretch(1);
                            cardLayout->addWidget(openButton);
                            deviceGrid->addWidget(card, i / 3, i % 3);
                            QObject::connect(openButton, &QPushButton::clicked, card,
                                [mainTabs, deviceTabs, i, devicesTopIndex]()
                                {
                                    if (deviceTabs && i < deviceTabs->count()) deviceTabs->setCurrentIndex(i);
                                    if (devicesTopIndex >= 0) mainTabs->setCurrentIndex(devicesTopIndex + 1);
                                });
                        }
                        return true;
                    };

                    if (!populateMasterRGBDashboard())
                    {
                        QLabel* waiting = new QLabel("Aguardando a detecção dos dispositivos RGB...", dashboard);
                        waiting->setObjectName("MasterRGBDashboardSubtitle");
                        deviceGrid->addWidget(waiting, 0, 0);
                        QTimer* dashboardTimer = new QTimer(dashboard);
                        dashboardTimer->setInterval(750);
                        QObject::connect(dashboardTimer, &QTimer::timeout, dashboard,
                            [populateMasterRGBDashboard, dashboardTimer]()
                            {
                                if (populateMasterRGBDashboard()) dashboardTimer->stop();
                            });
                        dashboardTimer->start();
                    }
                    dashboardLayout->addLayout(deviceGrid);
                    dashboardLayout->addStretch(1);
                    mainTabs->insertTab(0, dashboard, "Visão geral");
                }

                // Add a real in-app theme selector inside Configurações > Aparência.
                // The previous launcher-only preference did not let users switch themes live.
                QTabWidget* settingsTabs = findChild<QTabWidget*>("SettingsTabBar");
                bool appearanceExists = false;
                if (settingsTabs)
                {
                    for (int i = 0; i < settingsTabs->count(); ++i)
                    {
                        if (settingsTabs->widget(i) && settingsTabs->widget(i)->objectName() == "MasterRGBAppearancePage")
                        {
                            appearanceExists = true;
                            break;
                        }
                    }
                }
                if (settingsTabs && !appearanceExists)
                {
                    QWidget* appearancePage = new QWidget(settingsTabs);
                    appearancePage->setObjectName("MasterRGBAppearancePage");
                    QVBoxLayout* appearanceLayout = new QVBoxLayout(appearancePage);
                    appearanceLayout->setContentsMargins(24, 22, 24, 22);
                    appearanceLayout->setSpacing(12);

                    QLabel* appearanceTitle = new QLabel("Aparência do MasterRGB", appearancePage);
                    appearanceTitle->setObjectName("MasterRGBAppearanceTitle");
                    QLabel* appearanceDescription = new QLabel("Escolha o tema da interface. A alteração é aplicada imediatamente e fica salva para a próxima inicialização.", appearancePage);
                    appearanceDescription->setObjectName("MasterRGBAppearanceDescription");
                    appearanceDescription->setWordWrap(true);
                    QGroupBox* themeGroup = new QGroupBox("Tema da interface", appearancePage);
                    QVBoxLayout* themeLayout = new QVBoxLayout(themeGroup);
                    QComboBox* themeCombo = new QComboBox(themeGroup);
                    themeCombo->setObjectName("MasterRGBThemeSelector");
                    themeCombo->addItem("Escuro", "Dark");
                    themeCombo->addItem("Claro", "Light");
                    QLabel* themeStatus = new QLabel("O tema escuro é o padrão.", themeGroup);
                    themeStatus->setObjectName("MasterRGBThemeStatus");
                    themeLayout->addWidget(themeCombo);
                    themeLayout->addWidget(themeStatus);
                    appearanceLayout->addWidget(appearanceTitle);
                    appearanceLayout->addWidget(appearanceDescription);
                    appearanceLayout->addWidget(themeGroup);
                    appearanceLayout->addStretch(1);

                    const QString appConfigDir = qEnvironmentVariable("APPDATA") + "/MasterRGB";
                    const QString themePreferencePath = appConfigDir + "/theme.txt";
                    QString savedTheme = "Dark";
                    QFile savedThemeFile(themePreferencePath);
                    if (savedThemeFile.open(QIODevice::ReadOnly | QIODevice::Text))
                    {
                        savedTheme = QString::fromUtf8(savedThemeFile.readAll()).trimmed();
                        savedThemeFile.close();
                    }
                    if (savedTheme != "Light") savedTheme = "Dark";
                    themeCombo->setCurrentIndex(savedTheme == "Light" ? 1 : 0);

                    auto applySelectedTheme = [themeCombo, themeStatus, themePreferencePath]()
                    {
                        const QString themeName = themeCombo->currentData().toString() == "Light" ? "Light" : "Dark";
                        const QString themePath = QCoreApplication::applicationDirPath() + "/themes/MasterRGB-" + themeName + ".qss";
                        QFile themeFile(themePath);
                        if (!themeFile.open(QIODevice::ReadOnly | QIODevice::Text))
                        {
                            themeStatus->setText("Não foi possível carregar o arquivo do tema.");
                            return;
                        }
                        qApp->setStyleSheet(QString::fromUtf8(themeFile.readAll()));
                        themeFile.close();
                        QDir().mkpath(QFileInfo(themePreferencePath).absolutePath());
                        QFile preference(themePreferencePath);
                        if (preference.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
                        {
                            preference.write(themeName.toUtf8());
                            preference.close();

                            // Keep the external launcher in sync so opening its
                            // control center cannot overwrite the in-app choice.
                            const QString settingsPath = QFileInfo(themePreferencePath).absolutePath() + "/settings.json";
                            QJsonObject launcherSettings;
                            QFile existingSettings(settingsPath);
                            if (existingSettings.open(QIODevice::ReadOnly))
                            {
                                const QJsonDocument existingDoc = QJsonDocument::fromJson(existingSettings.readAll());
                                if (existingDoc.isObject()) launcherSettings = existingDoc.object();
                                existingSettings.close();
                            }
                            launcherSettings.insert("theme", themeName);
                            QFile updatedSettings(settingsPath);
                            if (updatedSettings.open(QIODevice::WriteOnly | QIODevice::Truncate))
                            {
                                updatedSettings.write(QJsonDocument(launcherSettings).toJson(QJsonDocument::Indented));
                                updatedSettings.close();
                            }
                            themeStatus->setText(themeName == "Light" ? "Tema claro aplicado e salvo." : "Tema escuro aplicado e salvo.");
                        }
                        else
                        {
                            themeStatus->setText("Tema aplicado, mas não foi possível salvar a preferência.");
                        }
                    };
                    QObject::connect(themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), appearancePage,
                        [applySelectedTheme](int) { applySelectedTheme(); });
                    settingsTabs->addTab(appearancePage, "Aparência");
                    const int appearanceIndex = settingsTabs->indexOf(appearancePage);
                    if (appearanceIndex >= 0) settingsTabs->setTabToolTip(appearanceIndex, "Tema escuro ou claro do MasterRGB");
                }

                bool supportExists = false;
                for (int i = 0; i < mainTabs->count(); ++i)
                {
                    if (mainTabs->tabText(i).compare("Apoiar", Qt::CaseInsensitive) == 0 ||
                        mainTabs->tabText(i).compare("Suporte", Qt::CaseInsensitive) == 0)
                    {
                        supportExists = true;
                        break;
                    }
                }

                if (!supportExists)
                {
                    QWidget* supportPage = new QWidget(mainTabs);
                    supportPage->setObjectName("MasterRGBSupportPage");
                    QVBoxLayout* layout = new QVBoxLayout(supportPage);
                    layout->setContentsMargins(28, 24, 28, 24);
                    layout->setSpacing(14);

                    QLabel* title = new QLabel("Apoie o desenvolvimento do MasterRGB", supportPage);
                    title->setObjectName("MasterRGBSupportTitle");
                    QLabel* description = new QLabel(
                        "O apoio ajuda a manter o projeto, testar novos controladores e ampliar a compatibilidade.",
                        supportPage);
                    description->setObjectName("MasterRGBSupportDescription");
                    description->setWordWrap(true);

                    QLabel* paypalLabel = new QLabel("Link de doação PayPal", supportPage);
                    QLineEdit* paypalEdit = new QLineEdit(supportPage);
                    paypalEdit->setObjectName("MasterRGBPaypalUrl");
                    paypalEdit->setPlaceholderText("https://paypal.me/seu-link");
                    QLabel* githubLabel = new QLabel("Repositório GitHub (opcional)", supportPage);
                    QLineEdit* githubEdit = new QLineEdit(supportPage);
                    githubEdit->setObjectName("MasterRGBGithubUrl");
                    githubEdit->setPlaceholderText("https://github.com/usuario/MasterRGB");

                    QFormLayout* fields = new QFormLayout();
                    fields->addRow(paypalLabel, paypalEdit);
                    fields->addRow(githubLabel, githubEdit);

                    QHBoxLayout* buttons = new QHBoxLayout();
                    QPushButton* donateButton = new QPushButton("Apoiar via PayPal", supportPage);
                    donateButton->setObjectName("MasterRGBDonateButton");
                    QPushButton* saveButton = new QPushButton("Salvar links", supportPage);
                    saveButton->setObjectName("MasterRGBSaveSupportButton");
                    QPushButton* githubButton = new QPushButton("Projeto e atualizações", supportPage);
                    githubButton->setObjectName("MasterRGBGithubButton");
                    buttons->addWidget(donateButton);
                    buttons->addWidget(saveButton);
                    buttons->addWidget(githubButton);
                    buttons->addStretch(1);

                    QLabel* setupNote = new QLabel(
                        "Cole aqui o mesmo endereço PayPal usado no YT Desk Monitor. "
                        "Os links ficam salvos localmente e não são enviados automaticamente.", supportPage);
                    setupNote->setObjectName("MasterRGBSupportNote");
                    setupNote->setWordWrap(true);

                    layout->addWidget(title);
                    layout->addWidget(description);
                    layout->addLayout(fields);
                    layout->addLayout(buttons);
                    layout->addWidget(setupNote);
                    layout->addStretch(1);

                    auto readSupportJson = []() -> QJsonObject
                    {
                        const QString appData = qEnvironmentVariable("APPDATA");
                        const QString userPath = appData + "/MasterRGB/support.json";
                        const QString bundledPath = QCoreApplication::applicationDirPath() + "/config/support.json";
                        QString path = QFile::exists(userPath) ? userPath : bundledPath;
                        QFile file(path);
                        if (!file.open(QIODevice::ReadOnly)) return QJsonObject();
                        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
                        return document.isObject() ? document.object() : QJsonObject();
                    };
                    const QJsonObject savedSupport = readSupportJson();
                    paypalEdit->setText(savedSupport.value("paypalUrl").toString());
                    QString savedGithub = savedSupport.value("githubUrl").toString();
                    const QString savedRepository = savedSupport.value("githubRepository").toString();
                    if (savedGithub.isEmpty() && savedRepository.contains("/") && !savedRepository.contains(" "))
                        savedGithub = "https://github.com/" + savedRepository;
                    githubEdit->setText(savedGithub);

                    QObject::connect(saveButton, &QPushButton::clicked, supportPage, [paypalEdit, githubEdit, supportPage]()
                    {
                        const QString paypalUrl = paypalEdit->text().trimmed();
                        if (!paypalUrl.isEmpty())
                        {
                            const QUrl url(paypalUrl);
                            const QString host = url.host().toLower();
                            const bool trustedHost = host == "paypal.com" || host.endsWith(".paypal.com") || host == "paypal.me" || host.endsWith(".paypal.me");
                            if (url.scheme() != "https" || !trustedHost || !url.isValid())
                            {
                                QMessageBox::warning(supportPage, "Link inválido", "Use um endereço HTTPS oficial do PayPal ou deixe o campo vazio.");
                                return;
                            }
                        }
                        const QString githubUrl = githubEdit->text().trimmed();
                        if (!githubUrl.isEmpty())
                        {
                            const QUrl url(githubUrl);
                            if (url.scheme() != "https" || url.host().compare("github.com", Qt::CaseInsensitive) != 0 || !url.isValid())
                            {
                                QMessageBox::warning(supportPage, "Link inválido", "Use um endereço HTTPS de github.com ou deixe o campo vazio.");
                                return;
                            }
                        }
                        const QString dir = qEnvironmentVariable("APPDATA") + "/MasterRGB";
                        QDir().mkpath(dir);
                        QFile file(dir + "/support.json");
                        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
                        {
                            QMessageBox::critical(supportPage, "MasterRGB", "Não foi possível salvar support.json em %APPDATA%\\MasterRGB.");
                            return;
                        }
                        QJsonObject config;
                        config.insert("product", "MasterRGB");
                        config.insert("paypalUrl", paypalUrl);
                        config.insert("githubUrl", githubUrl);
                        config.insert("supportLabel", "Apoiar o MasterRGB");
                        file.write(QJsonDocument(config).toJson(QJsonDocument::Indented));
                        QMessageBox::information(supportPage, "MasterRGB", "Links de suporte salvos neste computador.");
                    });

                    QObject::connect(donateButton, &QPushButton::clicked, supportPage, [paypalEdit, supportPage]()
                    {
                        const QUrl url(paypalEdit->text().trimmed());
                        const QString host = url.host().toLower();
                        const bool trustedHost = host == "paypal.com" || host.endsWith(".paypal.com") || host == "paypal.me" || host.endsWith(".paypal.me");
                        if (url.scheme() == "https" && trustedHost && url.isValid()) QDesktopServices::openUrl(url);
                        else QMessageBox::information(supportPage, "Apoiar o MasterRGB", "Insira e salve o mesmo link PayPal usado no YT Desk Monitor.");
                    });

                    QObject::connect(githubButton, &QPushButton::clicked, supportPage, [githubEdit, supportPage]()
                    {
                        const QUrl url(githubEdit->text().trimmed());
                        if (url.scheme() == "https" && url.host().compare("github.com", Qt::CaseInsensitive) == 0 && url.isValid()) QDesktopServices::openUrl(url);
                        else QMessageBox::information(supportPage, "MasterRGB", "O repositório GitHub será configurado quando o projeto for publicado.");
                    });

                    mainTabs->addTab(supportPage, "Apoiar");
                }
            }
        });
    }
'@

$needle = 'OpenRGBThemeManager::Init();'
$insertAt = $text.IndexOf($needle, [StringComparison]::Ordinal)
if ($insertAt -lt 0) {
    throw 'Não encontrei OpenRGBThemeManager::Init(); na base WinRing0 esperada. Nenhuma alteração foi aplicada.'
}
$insertAt += $needle.Length
$text = $text.Insert($insertAt, "`r`n" + $block)
$text = $text.Replace('trayIcon->setToolTip("OpenRGB");', 'trayIcon->setToolTip("MasterRGB");')

# Rebrand CLI strings while preserving upstream source attribution and symbols.
$cli = Join-Path $SourceDir 'cli.cpp'
if (Test-Path -LiteralPath $cli) {
    $cliText = [IO.File]::ReadAllText($cli)
    $cliText = $cliText.Replace('help_text += "OpenRGB ";', 'help_text += "MasterRGB ";')
    $cliText = $cliText.Replace('version_text += "OpenRGB ";', 'version_text += "MasterRGB ";')
    $cliText = $cliText.Replace('Run `OpenRGB --help` for syntax', 'Run `MasterRGB --help` for syntax')
    $cliText = $cliText.Replace('Usage: OpenRGB (', 'Usage: MasterRGB (')
    [IO.File]::WriteAllText($cli, $cliText, [Text.UTF8Encoding]::new($false))
}

[IO.File]::WriteAllText($dialog, $text, [Text.UTF8Encoding]::new($true))
Write-Host 'Marca MasterRGB, seletor de tema em Configurações, rebranding e aba Apoiar aplicados.' -ForegroundColor Green

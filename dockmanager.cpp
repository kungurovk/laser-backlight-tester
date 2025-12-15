#include "dockmanager.h"
#include "modecontrolform.h"
#include "blocktableform.h"
#include "sensorstableform.h"
#include "limitandtargetvaluesform.h"
#include "generatorsetterform.h"
#include "modbusclient.h"

#include <QDockWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QListWidget>
#include <QLabel>
#include <QAction>
#include <QCloseEvent>
#include <QSettings>
#include <QScreen>
#include <QApplication>
#include <QVariant>
#include <QtMath>
#include <QLayout>
#include <QComboBox>
#include <QPushButton>
#include <QDebug>
#include <QTimer>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

static QString settingsOrg() { return QStringLiteral("Lassard"); }
static QString settingsApp() { return QStringLiteral("Laser Backlight Tester"); }

DockManager::DockManager(QWidget *parent)
    : QMainWindow{parent}
{
    // createUi();
    m_requestAllTimer = new QTimer(this);
    m_requestAllTimer->setSingleShot(false);
    m_requestAllTimer->setTimerType(Qt::PreciseTimer);
    connect(m_requestAllTimer, &QTimer::timeout, [this](){
        requestAllValues();
    });

    createActions();
    createMenusAndToolbars();

    setDockOptions(QMainWindow::AllowTabbedDocks | QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::GroupedDragging);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    // statusBar()->showMessage(tr("Ready"));

    if (findChildren<QDockWidget*>().isEmpty()) {
        // Fallback demo content if nothing restored
        addModeControlWidget();
        addSensorTableWidget();
        addBlockTableWidget();
        addValuesWidget();
        addGeneratorWidget();
    }
    // Try restore previous session
    restoreLayout();
}

void DockManager::setModbusClient(ModbusClient *client)
{
    if (m_modbusClient == client) {
        return;
    }

    if (m_modbusClient) {
        disconnect(m_modbusClient, nullptr, this, nullptr);
    }

    m_modbusClient = client;
    const auto widgets = findChildren<QWidget*>();
    for (auto *widget : widgets) {
        if (auto *form = dynamic_cast<ModbusBase*>(widget)) {
            form->setModbusClient(m_modbusClient);
        }
    }

    if (m_modbusClient) {
        connect(m_modbusClient, &ModbusClient::connectionStateChanged, this, &DockManager::onConnectionStateChanged);
        onConnectionStateChanged(m_modbusClient->isConnected());
    } else {
        onConnectionStateChanged(false);
    }
}

void DockManager::createUi()
{
    auto *center = new QLabel(tr("Добавляйте элементы через меню/панель инструментов"), this);
    center->setAlignment(Qt::AlignCenter);
    setCentralWidget(center);

    resize(QGuiApplication::primaryScreen()->availableSize() * 0.7);
}

void DockManager::createActions()
{
    m_actAddSensorTable = new QAction(tr("Показания датчиков"), this);
    m_actAddSensorTable->setCheckable(true);
    connect(m_actAddSensorTable, &QAction::toggled, this, &DockManager::toggleSensorsTable);

    m_actAddBlockTable = new QAction(tr("Статусы блоков"), this);
    m_actAddBlockTable->setCheckable(true);
    connect(m_actAddBlockTable, &QAction::toggled, this, &DockManager::toggleBlockTable);

    m_actAddModeControl = new QAction(tr("Управление режимами"), this);
    m_actAddModeControl->setCheckable(true);
    connect(m_actAddModeControl, &QAction::toggled, this, &DockManager::toggleModeControl);

    m_actAddValuesTable = new QAction(tr("Предельные и целевые значения"), this);
    m_actAddValuesTable->setCheckable(true);
    connect(m_actAddValuesTable, &QAction::toggled, this, &DockManager::toggleValueTable);

    m_actAddGeneratorTable = new QAction(tr("Задающий генератор"), this);
    m_actAddGeneratorTable->setCheckable(true);
    connect(m_actAddGeneratorTable, &QAction::toggled, this, &DockManager::toggleGeneratorTable);

    m_actShowTitles = new QAction(tr("Показывать заголовки"), this);
    m_actShowTitles->setCheckable(true);
    m_actShowTitles->setChecked(true);
    connect(m_actShowTitles, &QAction::toggled, this, &DockManager::toggleDockTitles);

    m_actSaveLayout = new QAction(tr("Сохранить раскладку"), this);
    connect(m_actSaveLayout, &QAction::triggered, this, &DockManager::saveLayout);

    m_actRestoreLayout = new QAction(tr("Восстановить раскладку"), this);
    connect(m_actRestoreLayout, &QAction::triggered, this, &DockManager::restoreLayout);

    m_actTile = new QAction(tr("Разложить плиткой"), this);
    connect(m_actTile, &QAction::triggered, this, &DockManager::tileDocks);

    m_actCascade = new QAction(tr("Каскад"), this);
    connect(m_actCascade, &QAction::triggered, this, &DockManager::cascadeDocks);

    m_actCloseAll = new QAction(tr("Закрыть все"), this);
    connect(m_actCloseAll, &QAction::triggered, this, &DockManager::closeAllDocks);
}

void DockManager::createMenusAndToolbars()
{
    m_fileMenu = menuBar()->addMenu(tr("Файл"));
    m_fileMenu->addAction(m_actSaveLayout);
    m_fileMenu->addAction(m_actRestoreLayout);

    m_viewMenu = menuBar()->addMenu(tr("Вид"));
    m_viewMenu->addAction(m_actShowTitles);

    m_windowMenu = menuBar()->addMenu(tr("Окна"));
    m_windowMenu->addAction(m_actAddModeControl);
    m_windowMenu->addAction(m_actAddSensorTable);
    m_windowMenu->addAction(m_actAddBlockTable);
    m_windowMenu->addAction(m_actAddValuesTable);
    m_windowMenu->addAction(m_actAddGeneratorTable);
    m_windowMenu->addSeparator();
    // m_windowMenu->addAction(m_actTile);
    // m_windowMenu->addAction(m_actCascade);
    m_windowMenu->addSeparator();
    m_windowMenu->addAction(m_actCloseAll);

    m_mainToolbar = addToolBar(tr("Главная"));
    m_mainToolbar->setObjectName(QStringLiteral("MainToolbar"));
    m_buttonConnect = new QPushButton;
    connect(m_buttonConnect, &QPushButton::clicked, this, &DockManager::toggleConnect);
    m_mainToolbar->addWidget(m_buttonConnect);
    m_mainToolbar->addSeparator();

    m_mainToolbar->addWidget(new QLabel("Период опроса (сек.)", this));
    auto comboBox = new QComboBox(this);
    comboBox->addItems({"3", "5", "10", "15", "30", "60"});
    connect(comboBox, &QComboBox::currentTextChanged, [this](const QString &text){
        m_requestAllTimer->setInterval(text.toUInt() * 1000);
    });
    comboBox->setCurrentText("5");
    m_mainToolbar->addWidget(comboBox);
    m_startStopButton = new QPushButton(this);
    m_startStopButton->setFixedHeight(comboBox->height());
    m_startStopButton->setFixedWidth(50);
    startStopButton();
    connect(m_startStopButton, &QPushButton::clicked, this, &DockManager::startStopButton);
    m_mainToolbar->addWidget(m_startStopButton);

    m_mainToolbar->addSeparator();
    m_mainToolbar->addAction(m_actAddModeControl);
    m_mainToolbar->addAction(m_actAddSensorTable);
    m_mainToolbar->addAction(m_actAddBlockTable);
    m_mainToolbar->addAction(m_actAddValuesTable);
    m_mainToolbar->addAction(m_actAddGeneratorTable);
    m_mainToolbar->addSeparator();
    // m_mainToolbar->addAction(m_actTile);
    // m_mainToolbar->addAction(m_actCascade);
    m_mainToolbar->layout()->setSpacing(5);

}

QDockWidget* DockManager::createDockFor(QWidget *content, const QString &title)
{
    auto *dock = new QDockWidget(title, this);
    dock->setObjectName(QStringLiteral("Dock_%1_%2").arg(title).arg(++m_dockCounter));
    dock->setWidget(content);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    connectDockSignals(dock);
    return dock;
}

QDockWidget* DockManager::createDockFor(QWidget *content, const QString &title, const QString &objectNameOverride)
{
    auto *dock = new QDockWidget(title, this);
    dock->setObjectName(objectNameOverride.isEmpty() ? QStringLiteral("Dock_%1_%2").arg(title).arg(++m_dockCounter) : objectNameOverride);
    dock->setWidget(content);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    connectDockSignals(dock);
    return dock;
}

void DockManager::addSensorTableWidget()
{
    auto *sensors = new SensorsTableForm(this);
    auto *dock = createDockFor(sensors, tr("Показания датчиков"));
    dock->show();
    updateActionChecks();
}

void DockManager::addBlockTableWidget()
{
    auto *block = new BlockTableForm(this);
    auto *dock = createDockFor(block, tr("Статусы блоков"));
    dock->show();
    updateActionChecks();
}

void DockManager::addModeControlWidget()
{
    auto *modeControlForm = new ModeControlForm(this);
    connect(modeControlForm, &ModeControlForm::modeRequested, this, &DockManager::modeRequested);
    auto *dock = createDockFor(modeControlForm, "Управление режимами");
    dock->show();
    updateActionChecks();
}

void DockManager::addValuesWidget()
{
    auto *valuesForm = new LimitAndTargetValuesForm(this);
    auto *dock = createDockFor(valuesForm, "Предельные и целевые значения");
    dock->show();
    updateActionChecks();
}

void DockManager::addGeneratorWidget()
{
    auto *generatorForm = new GeneratorSetterForm(this);
    auto *dock = createDockFor(generatorForm, "Задающий генератор");
    dock->show();
    updateActionChecks();
}

void DockManager::toggleConnect(bool /*on*/)
{
    qDebug() << "toggleConnect";
    if (m_isConnected) {
        emit disconnectFromTcp();
    } else {
        // emit connectToTcpPort("127.0.0.1", 502);
        emit connectToTcpPort("172.16.5.101", 502);
    }
}

void DockManager::onConnectionStateChanged(bool connected)
{
    m_isConnected = connected;

    if (m_buttonConnect)
        m_buttonConnect->setText(connected ? tr("Отключить") : tr("Подключить"));

    statusBar()->showMessage(connected ? tr("Подключено") : tr("Отключено"), 3000);

    if (connected)
    {
        requestAllValues();
    }
}

void DockManager::startStopButton()
{
    if (!m_isStartedPool)
    {
        m_startStopButton->setIcon(QIcon("://icons/start-on.svg"));
        m_startStopButton->setToolTip("Старт");
        m_requestAllTimer->stop();
    }
    else
    {
        m_startStopButton->setIcon(QIcon("://icons/stop-on.svg"));
        m_startStopButton->setToolTip("Стоп");
        m_requestAllTimer->start();
    }
    m_isStartedPool = !m_isStartedPool;
}

void DockManager::connectDockSignals(QDockWidget *dock)
{
    if (!dock) return;
    connect(dock, &QDockWidget::visibilityChanged, this, [this](bool){ updateActionChecks(); });
    connect(dock, &QObject::destroyed, this, [this](QObject*){ updateActionChecks(); });
        connect(dock, &QDockWidget::topLevelChanged, [dock](bool floating) {
        if (floating) {
            // Defer the native window modifications to ensure the window handle is valid
            // and to avoid race conditions with Qt's own window state management.
            QTimer::singleShot(0, dock, [=](){
#ifdef Q_OS_WIN
                HWND hwnd = reinterpret_cast<HWND>(dock->winId());
                if (hwnd) {
                    SetWindowLongPtr(hwnd, GWLP_HWNDPARENT, 0);
                    SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | WS_CAPTION);
                    SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
                }
#else
                dock->setWindowFlags(dock->windowFlags() & ~Qt::Tool | Qt::Window);
#endif
                dock->show();
            });
        }
    });
}

void DockManager::updateActionChecks()
{
    bool anySensorsVisible = false;
    bool anyBlocksVisible = false;
    bool anyModeVisible = false;
    bool anyValuesVisible = false;
    bool anyGeneratorVisible = false;
    const auto docks = findChildren<QDockWidget*>();
    for (auto *dock : docks) {
        if (!dock->isVisible()) continue;
        QWidget *w = dock->widget();
        if (qobject_cast<SensorsTableForm*>(w)) anySensorsVisible = true;
        else if (qobject_cast<BlockTableForm*>(w)) anyBlocksVisible = true;
        else if (qobject_cast<ModeControlForm*>(w)) anyModeVisible = true;
        else if (qobject_cast<LimitAndTargetValuesForm*>(w)) anyValuesVisible = true;
        else if (qobject_cast<GeneratorSetterForm*>(w)) anyGeneratorVisible = true;
    }
    if (m_actAddSensorTable) m_actAddSensorTable->blockSignals(true), m_actAddSensorTable->setChecked(anySensorsVisible), m_actAddSensorTable->blockSignals(false);
    if (m_actAddBlockTable) m_actAddBlockTable->blockSignals(true), m_actAddBlockTable->setChecked(anyBlocksVisible), m_actAddBlockTable->blockSignals(false);
    if (m_actAddModeControl) m_actAddModeControl->blockSignals(true), m_actAddModeControl->setChecked(anyModeVisible), m_actAddModeControl->blockSignals(false);
    if (m_actAddValuesTable) m_actAddValuesTable->blockSignals(true), m_actAddValuesTable->setChecked(anyValuesVisible), m_actAddValuesTable->blockSignals(false);
    if (m_actAddGeneratorTable) m_actAddGeneratorTable->blockSignals(true), m_actAddGeneratorTable->setChecked(anyGeneratorVisible), m_actAddGeneratorTable->blockSignals(false);
}

void DockManager::toggleSensorsTable(bool on)
{
    bool found = false;
    const auto docks = findChildren<QDockWidget*>();
    for (auto *dock : docks) {
        if (qobject_cast<SensorsTableForm*>(dock->widget())) {
            found = true;
            if (on) dock->show(); else dock->close();
        }
    }
    if (on && !found) addSensorTableWidget();
    updateActionChecks();
}

void DockManager::toggleBlockTable(bool on)
{
    bool found = false;
    const auto docks = findChildren<QDockWidget*>();
    for (auto *dock : docks) {
        if (qobject_cast<BlockTableForm*>(dock->widget())) {
            found = true;
            if (on) dock->show(); else dock->close();
        }
    }
    if (on && !found) addBlockTableWidget();
    updateActionChecks();
}

void DockManager::toggleModeControl(bool on)
{
    bool found = false;
    const auto docks = findChildren<QDockWidget*>();
    for (auto *dock : docks) {
        if (qobject_cast<ModeControlForm*>(dock->widget())) {
            found = true;
            if (on) dock->show(); else dock->close();
        }
    }
    if (on && !found) addModeControlWidget();
    updateActionChecks();
}

void DockManager::toggleValueTable(bool on)
{
    bool found = false;
    const auto docks = findChildren<QDockWidget*>();
    for (auto *dock : docks) {
        if (qobject_cast<LimitAndTargetValuesForm*>(dock->widget())) {
            found = true;
            if (on) dock->show(); else dock->close();
        }
    }
    if (on && !found) addValuesWidget();
    updateActionChecks();
}

void DockManager::toggleGeneratorTable(bool on)
{
    bool found = false;
    const auto docks = findChildren<QDockWidget*>();
    for (auto *dock : docks) {
        if (qobject_cast<GeneratorSetterForm*>(dock->widget())) {
            found = true;
            if (on) dock->show(); else dock->close();
        }
    }
    if (on && !found) addGeneratorWidget();
    updateActionChecks();
}
void DockManager::toggleDockTitles(bool show)
{
    const auto docks = findChildren<QDockWidget*>();
    for (auto *dock : docks) dock->setTitleBarWidget(show ? nullptr : new QWidget(dock));
}

void DockManager::saveDockContents(QSettings &settings)
{
    settings.beginGroup("docks");
    const auto docks = findChildren<QDockWidget*>();
    for (auto *dock : docks) {
        if (dock->isFloating() || !dock->isVisible())
            continue;

        // Use objectName as a unique key to ensure overwriting
        settings.beginGroup(dock->objectName());
        settings.setValue("title", dock->windowTitle());
        QWidget *content = dock->widget();
        const QString type = detectDockType(content);
        settings.setValue("type", type);
        if (auto *te = qobject_cast<QTextEdit*>(content)) {
            settings.setValue("payload", te->toPlainText());
        } else if (auto *lw = qobject_cast<QListWidget*>(content)) {
            QStringList items;
            for (int r = 0; r < lw->count(); ++r) items << lw->item(r)->text();
            settings.setValue("payload", items);
        } else if (auto *lab = qobject_cast<QLabel*>(content)) {
            settings.setValue("payload", lab->text());
        }
        settings.endGroup();
    }
    settings.endGroup();
}

void DockManager::loadDockContents(QSettings &settings)
{
    settings.beginGroup("docks");
    const QStringList dockObjectNames = settings.childGroups();
    for (const QString &objName : dockObjectNames) {
        settings.beginGroup(objName);
        const QString title = settings.value("title").toString();
        const QString type = settings.value("type").toString();
        const QVariant payload = settings.value("payload");
        QWidget *content = createWidgetFromType(type, payload);
        auto *dock = createDockFor(content, title.isEmpty() ? QStringLiteral("Dock") : title, objName);
        dock->hide();
        settings.endGroup();
    }
    settings.endGroup();
    updateActionChecks();
}

QString DockManager::detectDockType(QWidget *content) const
{
    if (qobject_cast<SensorsTableForm*>(content)) return QStringLiteral("sensorsTableForm");
    if (qobject_cast<BlockTableForm*>(content)) return QStringLiteral("blockTableForm");
    if (qobject_cast<ModeControlForm*>(content)) return QStringLiteral("modeControlForm");
    if (qobject_cast<LimitAndTargetValuesForm*>(content)) return QStringLiteral("valuesForm");
    if (qobject_cast<GeneratorSetterForm*>(content)) return QStringLiteral("generatorForm");
    return QStringLiteral("unknown");
}

QWidget* DockManager::createWidgetFromType(const QString &typeName, const QVariant &payload)
{
    Q_UNUSED(payload)
    if (typeName == QLatin1String("sensorsTableForm")) {
        auto *w = new SensorsTableForm(this);
        return w;
    } else if (typeName == QLatin1String("blockTableForm")) {
        auto *w = new BlockTableForm(this);
        return w;
    } else if (typeName == QLatin1String("modeControlForm")) {
        auto *modeControlForm = new ModeControlForm;
        connect(modeControlForm, &ModeControlForm::modeRequested, this, &DockManager::modeRequested);
        return modeControlForm;
    } else if (typeName == QLatin1String("valuesForm")) {
        auto *valuesForm = new LimitAndTargetValuesForm;
        return valuesForm;
    } else if (typeName == QLatin1String("generatorForm")) {
        auto *valuesForm = new GeneratorSetterForm;
        return valuesForm;
    }
    auto *fallback = new QLabel(tr("Неизвестный тип: %1").arg(typeName), this);
    fallback->setAlignment(Qt::AlignCenter);
    return fallback;
}

void DockManager::requestAllValues()
{
    const auto widgets = findChildren<QWidget*>();
    for (auto *widget : widgets) {
        if (auto *form = dynamic_cast<ModbusBase*>(widget)) {
            if (widget->isVisible())
                form->requestAllValues();
        }
    }
}

void DockManager::saveLayout()
{
    QSettings s(settingsOrg(), settingsApp());

    // Explicitly remove old groups before saving new ones
    s.remove("docks");
    s.remove("state");

    s.setValue("geometry", saveGeometry());

    const auto widget = findChild<BlockTableForm*>();
    if (widget) {
        widget->getSplitterSizes();
        // s.setValue("splitterSizes", widget->getSplitterSizes());
    }

    saveDockContents(s);
    s.setValue("state", saveState());
    s.sync(); // Force sync to registry
    statusBar()->showMessage(tr("Раскладка сохранена"), 2000);
}

void DockManager::restoreLayout()
{
    QSettings s(settingsOrg(), settingsApp());
    restoreGeometry(s.value("geometry").toByteArray());

    // const auto widget = findChild<BlockTableForm*>();
    // if (widget) {
    //     widget->getSplitterSizes();
    // }

    loadDockContents(s);
    restoreState(s.value("state").toByteArray());
    statusBar()->showMessage(tr("Раскладка восстановлена"), 2000);
    updateActionChecks();
}

void DockManager::tileDocks()
{
    relayoutGrid(false);
}

void DockManager::cascadeDocks()
{
    relayoutGrid(true);
}

void DockManager::closeAllDocks()
{
    const auto docks = findChildren<QDockWidget*>();
    for (auto *dock : docks) dock->close();
}

void DockManager::relayoutGrid(bool cascade)
{
    const auto docks = findChildren<QDockWidget*>();
    if (docks.isEmpty()) return;

    // if (!centralWidget()) {
    //     setCentralWidget(new QWidget(this));
    // }

    // Simple tiling/cascade: remove and re-add docks to enforce layout
    for (auto *dock : docks) removeDockWidget(dock);

    int index = 0;
    const int cols = qCeil(qSqrt(docks.size()));
    const int rows = qCeil(double(docks.size()) / cols);
    Q_UNUSED(rows)

    for (auto *dock : docks) {
        Qt::DockWidgetArea area;
        if (cascade) {
            area = Qt::RightDockWidgetArea;
        } else {
            int col = index % cols;
            area = (col % 2 == 0) ? Qt::LeftDockWidgetArea : Qt::RightDockWidgetArea;
        }
        dock->setFloating(false);
        addDockWidget(area, dock);
        dock->show();
        ++index;
    }
}

void DockManager::closeEvent(QCloseEvent *event)
{
    // saveLayout(); bad way
    QMainWindow::closeEvent(event);
}


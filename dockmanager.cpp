#include "dockmanager.h"
#include "modecontrolform.h"
#include "blocktableform.h"
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

static QString settingsOrg() { return QStringLiteral("Lassard"); }
static QString settingsApp() { return QStringLiteral("Laser Backlight Tester"); }

DockManager::DockManager(QWidget *parent)
    : QMainWindow{parent}
{
    // createUi();
    createActions();
    createMenusAndToolbars();

    setDockOptions(QMainWindow::AllowTabbedDocks | QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::GroupedDragging);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    statusBar()->showMessage(tr("Ready"));

    // Try restore previous session
    restoreLayout();
    if (findChildren<QDockWidget*>().isEmpty()) {
        // Fallback demo content if nothing restored
        addSampleTextWidget();
        addBlockTableWidget();
    }
}

void DockManager::setModbusClient(ModbusClient *client)
{
    if (m_modbusClient == client) {
        return;
    }

    m_modbusClient = client;
    const auto blockForms = findChildren<BlockTableForm*>();
    for (auto *form : blockForms) {
        form->setModbusClient(m_modbusClient);
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
    m_actAddText = new QAction(tr("Добавить Текст"), this);
    m_actAddText->setCheckable(true);
    connect(m_actAddText, &QAction::toggled, this, &DockManager::toggleText);

    m_actAddBlockTable = new QAction(tr("Статусы блоков"), this);
    m_actAddBlockTable->setCheckable(true);
    connect(m_actAddBlockTable, &QAction::toggled, this, &DockManager::toggleBlockTable);

    m_actAddModeControl = new QAction(tr("Управление режимами"), this);
    m_actAddModeControl->setCheckable(true);
    connect(m_actAddModeControl, &QAction::toggled, this, &DockManager::toggleModeControl);

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
    m_windowMenu->addAction(m_actAddText);
    m_windowMenu->addAction(m_actAddBlockTable);
    m_windowMenu->addSeparator();
    m_windowMenu->addAction(m_actTile);
    m_windowMenu->addAction(m_actCascade);
    m_windowMenu->addSeparator();
    m_windowMenu->addAction(m_actCloseAll);

    m_mainToolbar = addToolBar(tr("Главная"));
    m_mainToolbar->setObjectName(QStringLiteral("MainToolbar"));
    m_mainToolbar->addAction(m_actAddModeControl);
    m_mainToolbar->addAction(m_actAddText);
    m_mainToolbar->addAction(m_actAddBlockTable);
    m_mainToolbar->addSeparator();
    m_mainToolbar->addAction(m_actTile);
    m_mainToolbar->addAction(m_actCascade);
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

void DockManager::addSampleTextWidget()
{
    auto *text = new QTextEdit(this);
    text->setPlainText(tr("Это пример текстового редактора. Перетащите в любую область или объедините во вкладки."));
    auto *dock = createDockFor(text, tr("Текст"));
    dock->show();
    updateActionChecks();
}

void DockManager::addBlockTableWidget()
{
    auto *list = new BlockTableForm(this);
    list->setModbusClient(m_modbusClient);
    auto *dock = createDockFor(list, tr("Статусы блоков"));
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

void DockManager::connectDockSignals(QDockWidget *dock)
{
    if (!dock) return;
    connect(dock, &QDockWidget::visibilityChanged, this, [this](bool){ updateActionChecks(); });
    connect(dock, &QObject::destroyed, this, [this](QObject*){ updateActionChecks(); });
}

void DockManager::updateActionChecks()
{
    bool anyTextVisible = false;
    bool anyBlocksVisible = false;
    bool anyModeVisible = false;
    const auto docks = findChildren<QDockWidget*>();
    for (auto *dock : docks) {
        if (!dock->isVisible()) continue;
        QWidget *w = dock->widget();
        if (qobject_cast<QTextEdit*>(w)) anyTextVisible = true;
        else if (qobject_cast<BlockTableForm*>(w)) anyBlocksVisible = true;
        else if (qobject_cast<ModeControlForm*>(w)) anyModeVisible = true;
    }
    if (m_actAddText) m_actAddText->blockSignals(true), m_actAddText->setChecked(anyTextVisible), m_actAddText->blockSignals(false);
    if (m_actAddBlockTable) m_actAddBlockTable->blockSignals(true), m_actAddBlockTable->setChecked(anyBlocksVisible), m_actAddBlockTable->blockSignals(false);
    if (m_actAddModeControl) m_actAddModeControl->blockSignals(true), m_actAddModeControl->setChecked(anyModeVisible), m_actAddModeControl->blockSignals(false);
}

void DockManager::toggleText(bool on)
{
    bool found = false;
    const auto docks = findChildren<QDockWidget*>();
    for (auto *dock : docks) {
        if (qobject_cast<QTextEdit*>(dock->widget())) {
            found = true;
            if (on) dock->show(); else dock->close();
        }
    }
    if (on && !found) addSampleTextWidget();
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
void DockManager::toggleDockTitles(bool show)
{
    const auto docks = findChildren<QDockWidget*>();
    for (auto *dock : docks) dock->setTitleBarWidget(show ? nullptr : new QWidget(dock));
}

void DockManager::saveDockContents(QSettings &settings)
{
    settings.beginGroup("docks");
    settings.remove("");
    const auto docks = findChildren<QDockWidget*>();
    settings.setValue("count", docks.size());
    int maxCounter = m_dockCounter;
    int i = 0;
    for (auto *dock : docks) {
        settings.beginGroup(QString::number(i));
        settings.setValue("objectName", dock->objectName());
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
        const QString on = dock->objectName();
        const int idx = on.lastIndexOf('_');
        if (idx > 0) {
            bool ok = false; int num = on.mid(idx + 1).toInt(&ok);
            if (ok) maxCounter = qMax(maxCounter, num);
        }
        settings.endGroup();
        ++i;
    }
    settings.setValue("counter", maxCounter);
    settings.endGroup();
}

void DockManager::loadDockContents(QSettings &settings)
{
    settings.beginGroup("docks");
    const int count = settings.value("count", 0).toInt();
    m_dockCounter = settings.value("counter", 0).toInt();
    for (int i = 0; i < count; ++i) {
        settings.beginGroup(QString::number(i));
        const QString obj = settings.value("objectName").toString();
        const QString title = settings.value("title").toString();
        const QString type = settings.value("type").toString();
        const QVariant payload = settings.value("payload");
        QWidget *content = createWidgetFromType(type, payload);
        auto *dock = createDockFor(content, title.isEmpty() ? QStringLiteral("Dock") : title, obj);
        dock->hide();
        settings.endGroup();
    }
    settings.endGroup();
    updateActionChecks();
}

QString DockManager::detectDockType(QWidget *content) const
{
    if (qobject_cast<QTextEdit*>(content)) return QStringLiteral("text");
    if (qobject_cast<BlockTableForm*>(content)) return QStringLiteral("blockTableForm");
    if (qobject_cast<ModeControlForm*>(content)) return QStringLiteral("modeControlForm");
    return QStringLiteral("unknown");
}

QWidget* DockManager::createWidgetFromType(const QString &typeName, const QVariant &payload)
{
    Q_UNUSED(payload)
    if (typeName == QLatin1String("text")) {
        auto *w = new QTextEdit(this);
        w->setPlainText(payload.toString().isEmpty() ? tr("Восстановленный текстовый виджет") : payload.toString());
        return w;
    } else if (typeName == QLatin1String("blockTableForm")) {
        auto *w = new BlockTableForm(this);
        w->setModbusClient(m_modbusClient);
        return w;
    } else if (typeName == QLatin1String("modeControlForm")) {
        auto *modeControlForm = new ModeControlForm;
        connect(modeControlForm, &ModeControlForm::modeRequested, this, &DockManager::modeRequested);
        return modeControlForm;
    }
    auto *fallback = new QLabel(tr("Неизвестный тип: %1").arg(typeName), this);
    fallback->setAlignment(Qt::AlignCenter);
    return fallback;
}

void DockManager::saveLayout()
{
    QSettings s(settingsOrg(), settingsApp());
    s.setValue("geometry", saveGeometry());
    saveDockContents(s);
    s.setValue("state", saveState());
    statusBar()->showMessage(tr("Раскладка сохранена"), 2000);
}

void DockManager::restoreLayout()
{
    QSettings s(settingsOrg(), settingsApp());
    restoreGeometry(s.value("geometry").toByteArray());
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

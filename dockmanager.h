#ifndef DOCKMANAGER_H
#define DOCKMANAGER_H

#include <QMainWindow>
#include <qpushbutton.h>

#include "enums.h"

class ModbusClient;
class QMenu;
class QToolBar;
class QDockWidget;
class QListWidget;
class QAction;
class QWidget;
class QSettings;
class QLabel;

class DockManager : public QMainWindow
{
    Q_OBJECT
public:
    explicit DockManager(QWidget *parent = nullptr);
    void setModbusClient(ModbusClient *client);

signals:
    void modeRequested(Mode mode);
    void connectToTcpPort(const QString &host, quint16 port);
    void disconnectFromTcp();

protected:
    void closeEvent(QCloseEvent *event) override;
private slots:
    void addSensorTableWidget();
    void addBlockTableWidget();
    void addModeControlWidget();
    void addValuesWidget();
    void addGeneratorWidget();
    void toggleConnect(bool on);
    void toggleSensorsTable(bool on);
    void toggleBlockTable(bool on);
    void toggleModeControl(bool on);
    void toggleValueTable(bool on);
    void toggleGeneratorTable(bool on);
    void toggleDockTitles(bool show);
    void saveLayout();
    void restoreLayout();
    void tileDocks();
    void cascadeDocks();
    void closeAllDocks();
    void onConnectionStateChanged(bool connected);
    void startStopButton();

private:
    void createUi();
    void createActions();
    void createMenusAndToolbars();
    QDockWidget* createDockFor(QWidget *content, const QString &title);
    QDockWidget* createDockFor(QWidget *content, const QString &title, const QString &objectNameOverride);
    void updateActionChecks();
    void connectDockSignals(QDockWidget *dock);
    void relayoutGrid(bool cascade);
    void saveDockContents(QSettings &settings);
    void loadDockContents(QSettings &settings);
    QString detectDockType(QWidget *content) const;
    QWidget* createWidgetFromType(const QString &typeName, const QVariant &payload);
    void requestAllValues();
    void startReconnectionAttempts();
    void attemptReconnection();

private:
    QMenu *m_fileMenu = nullptr;
    QMenu *m_viewMenu = nullptr;
    QMenu *m_windowMenu = nullptr;
    QMenu *m_versionMenu = nullptr;
    QToolBar *m_mainToolbar = nullptr;
    QPushButton *m_buttonConnect = nullptr;
    QLabel *m_connectionStatusLabel = nullptr;
    QLabel *m_gitTagLabel = nullptr;
    QAction *m_actAddSensorTable = nullptr;
    QAction *m_actAddBlockTable = nullptr;
    QAction *m_actAddModeControl = nullptr;
    QAction *m_actAddValuesTable = nullptr;
    QAction *m_actAddGeneratorTable = nullptr;
    QAction *m_actShowTitles = nullptr;
    QAction *m_actSaveLayout = nullptr;
    QAction *m_actRestoreLayout = nullptr;
    QAction *m_actTile = nullptr;
    QAction *m_actCascade = nullptr;
    QAction *m_actCloseAll = nullptr;
    int m_dockCounter = 0;
    ModbusClient *m_modbusClient = nullptr;
    bool m_isConnected = false;
    QTimer *m_requestAllTimer = nullptr;
    bool m_isStartedPool = false;
    QPushButton *m_startStopButton = nullptr;
    QTimer *m_reconnectionTimer = nullptr;
    int m_reconnectionAttempts = 0;
    const int MAX_RECONNECTION_ATTEMPTS = 10;
};

#endif // DOCKMANAGER_H

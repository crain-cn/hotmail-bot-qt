#pragma once

#include "core/ConfigStore.h"
#include "core/RegistrationResult.h"
#include "core/ResultStore.h"
#include "core/TaskManager.h"

#include <QMainWindow>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class QSpinBox;
class QTableWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onStartClicked();
    void onListClicked();
    void onFeatureItemClicked();
    void onTaskStarted(int index);
    void onTaskProgress(int index, int percent);
    void onTaskFinished(int index, bool success, const QString &message);
    void onRegistrationCompleted(const RegistrationResult &result);
    void onAllTasksFinished();
    void onLogMessage(const QString &message);
    void refreshAccountTable();

private:
    void setupUi();
    void setupTitleBar(QWidget *titleBar);
    void wireTaskManager();
    void loadConfig();
    void saveConfig();
    UiConfigBindings configBindings() const;

    TaskManager m_taskManager;
    ResultStore m_resultStore;

    QPushButton *m_minButton = nullptr;
    QPushButton *m_maxButton = nullptr;
    QPushButton *m_closeButton = nullptr;

    QCheckBox *m_hotmailCheck = nullptr;
    QCheckBox *m_emailCheck = nullptr;
    QCheckBox *m_apiCheck = nullptr;
    QCheckBox *m_gmailCheck = nullptr;

    QLineEdit *m_delayEdit = nullptr;
    QLineEdit *m_botsEdit = nullptr;
    QLineEdit *m_apiKeyEdit = nullptr;
    QLineEdit *m_tempMailEdit = nullptr;
    QComboBox *m_batchCombo = nullptr;
    QComboBox *m_browserCombo = nullptr;
    QComboBox *m_captchaCombo = nullptr;
    QLineEdit *m_webDriverUrlEdit = nullptr;
    QLineEdit *m_captchaApiKeyEdit = nullptr;
    QCheckBox *m_headlessCheck = nullptr;
    QSpinBox *m_numericSpin = nullptr;
    QSpinBox *m_retrySpin = nullptr;

    QGroupBox *m_proxyGroup = nullptr;
    QPlainTextEdit *m_proxyEdit = nullptr;

    QListWidget *m_featureList = nullptr;
    QPushButton *m_listButton = nullptr;
    QPushButton *m_startButton = nullptr;

    QVector<QProgressBar *> m_progressBars;
    QTableWidget *m_accountTable = nullptr;
    QPlainTextEdit *m_logView = nullptr;

    bool m_dragging = false;
    QPoint m_dragOffset;
};

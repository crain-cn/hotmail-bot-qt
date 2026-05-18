#pragma once

#include "core/RegistrationResult.h"

#include <QObject>
#include <QString>
#include <QThread>
#include <QVector>

class AccountWorker;

class TaskManager : public QObject
{
    Q_OBJECT

public:
    enum class BrowserBackend { Mock, WebDriver };
    enum class CaptchaMode { Skip, Manual, HttpApi };

    struct Settings {
        BrowserBackend browserBackend = BrowserBackend::Mock;
        CaptchaMode captchaMode = CaptchaMode::Skip;
        QString webDriverUrl = QStringLiteral("http://127.0.0.1:9515");
        QString captchaApiKey;
        bool headlessBrowser = true;
        int maxRetries = 2;
        int manualCaptchaWaitSec = 120;
        bool hotmailEnabled = true;
        bool emailEnabled = false;
        bool apiEnabled = false;
        bool gmailEnabled = false;
        int botCount = 1;
        int delayMinMs = 4000;
        int delayMaxMs = 12000;
        QString apiKey;
        QString tempMailEndpoint;
        QString selectorsSyncUrl;
        QStringList proxies;
        int batchIndex = 0;
        int numericValue = 0;
    };

    explicit TaskManager(QObject *parent = nullptr);
    ~TaskManager() override;

    bool isRunning() const;

public slots:
    void start(const Settings &settings);
    void stop();

signals:
    void taskStarted(int index);
    void taskProgress(int index, int percent);
    void taskFinished(int index, bool success, const QString &message);
    void registrationCompleted(const RegistrationResult &result);
    void allTasksFinished();
    void logMessage(const QString &message);

private slots:
    void onWorkerStarted(int index);
    void onWorkerProgress(int index, int percent);
    void onWorkerFinished(int index, RegistrationResult result);

private:
    void clearWorkers();
    void checkAllFinished();

    QVector<QThread *> m_threads;
    QVector<AccountWorker *> m_workers;
    Settings m_settings;
    bool m_running = false;
    int m_finishedCount = 0;
};

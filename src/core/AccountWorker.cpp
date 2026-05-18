#include "core/AccountWorker.h"

#include "bridge/AutomationClient.h"
#include "net/ProxyManager.h"

#include <QThread>

AccountWorker::AccountWorker(int index, TaskManager::Settings settings, QObject *parent)
    : QObject(parent)
    , m_index(index)
    , m_settings(std::move(settings))
{
}

void AccountWorker::run()
{
    emit started(m_index);

    const QString proxy = ProxyManager::pickRotating(m_settings.proxies, m_index);

    AutomationClient client;
    const RegistrationResult result = client.runTask(
        m_index,
        m_settings,
        proxy,
        [this](int percent, RegistrationStage) { emit progress(m_index, percent); },
        []() { return QThread::currentThread()->isInterruptionRequested(); });

    emit progress(m_index, result.success ? 100 : 0);
    emit finished(m_index, result);
}

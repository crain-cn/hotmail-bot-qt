#include "core/TaskManager.h"

#include "core/AccountWorker.h"

#include <QThread>

TaskManager::TaskManager(QObject *parent)
    : QObject(parent)
{
}

TaskManager::~TaskManager()
{
    stop();
}

bool TaskManager::isRunning() const
{
    return m_running;
}

void TaskManager::start(const Settings &settings)
{
    if (m_running) {
        return;
    }

    stop();

    m_settings = settings;
    m_running = true;
    m_finishedCount = 0;

    const int workerCount = qMin(settings.botCount, 10);
    emit logMessage(QStringLiteral("Starting %1 worker(s)...").arg(workerCount));

    m_threads.reserve(workerCount);
    m_workers.reserve(workerCount);
    for (int i = 0; i < workerCount; ++i) {
        auto *thread = new QThread(this);
        auto *worker = new AccountWorker(i, settings);
        worker->moveToThread(thread);

        connect(thread, &QThread::started, worker, &AccountWorker::run);
        connect(worker, &AccountWorker::started, this, &TaskManager::onWorkerStarted);
        connect(worker, &AccountWorker::progress, this, &TaskManager::onWorkerProgress);
        connect(worker, &AccountWorker::finished, this, &TaskManager::onWorkerFinished);
        connect(worker, &AccountWorker::finished, thread, &QThread::quit);
        connect(worker, &AccountWorker::finished, worker, &AccountWorker::deleteLater);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        m_threads.append(thread);
        m_workers.append(worker);
        thread->start();
    }
}

void TaskManager::stop()
{
    if (!m_running && m_workers.isEmpty()) {
        return;
    }

    m_running = false;
    clearWorkers();
    emit logMessage(QStringLiteral("Workers stopped."));
}

void TaskManager::onWorkerStarted(int index)
{
    emit taskStarted(index);
    emit logMessage(QStringLiteral("Worker %1 started.").arg(index + 1));
}

void TaskManager::onWorkerProgress(int index, int percent)
{
    emit taskProgress(index, percent);
}

void TaskManager::onWorkerFinished(int index, RegistrationResult result)
{
    emit registrationCompleted(result);
    emit taskFinished(index,
                      result.success,
                      QStringLiteral("%1 | %2ms | %3")
                          .arg(result.message)
                          .arg(result.elapsedMs)
                          .arg(result.email));
    ++m_finishedCount;
    checkAllFinished();
}

void TaskManager::clearWorkers()
{
    for (QThread *thread : m_threads) {
        thread->requestInterruption();
        thread->quit();
        thread->wait(2000);
    }
    m_threads.clear();
    m_workers.clear();
}

void TaskManager::checkAllFinished()
{
    if (!m_running) {
        return;
    }

    const int activeWorkers = qMin(m_settings.botCount, 10);
    if (m_finishedCount < activeWorkers) {
        return;
    }

    m_running = false;
    m_finishedCount = 0;
    clearWorkers();
    emit allTasksFinished();
}

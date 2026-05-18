#pragma once

#include "core/RegistrationResult.h"
#include "core/TaskManager.h"

#include <QObject>

class AccountWorker : public QObject
{
    Q_OBJECT

public:
    AccountWorker(int index, TaskManager::Settings settings, QObject *parent = nullptr);

public slots:
    void run();

signals:
    void started(int index);
    void progress(int index, int percent);
    void finished(int index, RegistrationResult result);

private:
    int m_index;
    TaskManager::Settings m_settings;
};

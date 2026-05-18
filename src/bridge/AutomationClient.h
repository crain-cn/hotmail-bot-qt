#pragma once

#include "core/RegistrationResult.h"
#include "core/TaskManager.h"

#include <functional>

class AutomationClient
{
public:
    using ProgressCallback = std::function<void(int percent, RegistrationStage stage)>;
    using InterruptCheck = std::function<bool()>;

    RegistrationResult runTask(int workerIndex,
                               const TaskManager::Settings &settings,
                               const QString &proxy,
                               ProgressCallback onProgress,
                               InterruptCheck shouldStop);
};

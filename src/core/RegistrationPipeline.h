#pragma once

#include "core/AccountProfile.h"
#include "core/RegistrationResult.h"
#include "core/TaskManager.h"

#include <functional>

class RegistrationPipeline
{
public:
    using ProgressCallback = std::function<void(int percent, RegistrationStage stage)>;
    using InterruptCheck = std::function<bool()>;

    RegistrationResult run(const AccountProfile &profile,
                           const TaskManager::Settings &settings,
                           const QString &proxy,
                           int workerIndex,
                           ProgressCallback onProgress,
                           InterruptCheck shouldStop) const;
};

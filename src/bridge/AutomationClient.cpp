#include "bridge/AutomationClient.h"

#include "core/AccountProfile.h"
#include "core/RegistrationPipeline.h"

RegistrationResult AutomationClient::runTask(int workerIndex,
                                             const TaskManager::Settings &settings,
                                             const QString &proxy,
                                             ProgressCallback onProgress,
                                             InterruptCheck shouldStop)
{
    const AccountProfile profile = AccountProfile::generate(workerIndex, settings);
    RegistrationPipeline pipeline;
    return pipeline.run(profile, settings, proxy, workerIndex, onProgress, shouldStop);
}

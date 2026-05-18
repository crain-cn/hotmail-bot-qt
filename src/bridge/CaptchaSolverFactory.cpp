#include "bridge/CaptchaSolverFactory.h"

#include "bridge/HttpCaptchaSolver.h"
#include "bridge/ManualCaptchaSolver.h"
#include "bridge/SkipCaptchaSolver.h"

std::unique_ptr<CaptchaSolver> CaptchaSolverFactory::create(const TaskManager::Settings &settings)
{
    switch (settings.captchaMode) {
    case TaskManager::CaptchaMode::Manual:
        return std::make_unique<ManualCaptchaSolver>();
    case TaskManager::CaptchaMode::HttpApi:
        return std::make_unique<HttpCaptchaSolver>();
    case TaskManager::CaptchaMode::Skip:
    default:
        return std::make_unique<SkipCaptchaSolver>();
    }
}

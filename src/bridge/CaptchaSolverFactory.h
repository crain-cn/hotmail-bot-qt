#pragma once

#include "bridge/CaptchaSolver.h"
#include "core/TaskManager.h"

#include <memory>

class CaptchaSolverFactory
{
public:
    static std::unique_ptr<CaptchaSolver> create(const TaskManager::Settings &settings);
};

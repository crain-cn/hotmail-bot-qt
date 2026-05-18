#pragma once

#include "bridge/BrowserDriver.h"
#include "core/TaskManager.h"

#include <memory>

class BrowserDriverFactory
{
public:
    static std::unique_ptr<BrowserDriver> create(const TaskManager::Settings &settings);
};

#include "bridge/BrowserDriverFactory.h"

#include "bridge/MockBrowserDriver.h"
#include "bridge/WebDriverBrowserDriver.h"

std::unique_ptr<BrowserDriver> BrowserDriverFactory::create(const TaskManager::Settings &settings)
{
    if (settings.browserBackend == TaskManager::BrowserBackend::WebDriver) {
        return std::make_unique<WebDriverBrowserDriver>(settings.webDriverUrl);
    }
    return std::make_unique<MockBrowserDriver>();
}

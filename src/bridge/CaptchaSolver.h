#pragma once

#include "core/TaskManager.h"

#include <QString>

class WebDriverClient;

struct CaptchaContext {
    WebDriverClient *client = nullptr;
    QString sessionId;
    TaskManager::Settings settings;
};

class CaptchaSolver
{
public:
    virtual ~CaptchaSolver() = default;
    virtual QString name() const = 0;
    virtual bool solve(const CaptchaContext &context, QString *error) = 0;
};

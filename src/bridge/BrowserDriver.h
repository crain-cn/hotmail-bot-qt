#pragma once

#include "core/AccountProfile.h"
#include "core/TaskManager.h"

#include <QString>

class BrowserDriver
{
public:
    virtual ~BrowserDriver() = default;

    virtual QString backendName() const = 0;
    virtual bool start(const TaskManager::Settings &settings, const QString &proxy, QString *error) = 0;
    virtual void stop() = 0;
    virtual bool openSignupPage(QString *error) = 0;
    virtual bool fillRegistrationForm(const AccountProfile &profile, QString *error) = 0;
    virtual bool submitRegistration(QString *error) = 0;
    virtual bool handleVerification(const TaskManager::Settings &settings, QString *error) = 0;
    virtual bool handleCaptcha(QString *error) = 0;
};

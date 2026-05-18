#pragma once

#include "bridge/BrowserDriver.h"

class MockBrowserDriver : public BrowserDriver
{
public:
    QString backendName() const override;
    bool start(const TaskManager::Settings &settings, const QString &proxy, QString *error) override;
    void stop() override;
    bool openSignupPage(QString *error) override;
    bool fillRegistrationForm(const AccountProfile &profile, QString *error) override;
    bool submitRegistration(QString *error) override;
    bool handleVerification(const TaskManager::Settings &settings, QString *error) override;
    bool handleCaptcha(QString *error) override;

private:
    bool waitSlice(int ms, QString *error) const;

    TaskManager::Settings m_settings;
    int m_stepDelayMs = 400;
};

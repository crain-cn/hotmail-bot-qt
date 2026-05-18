#pragma once

#include "bridge/BrowserDriver.h"
#include "bridge/WebDriverClient.h"
#include "core/AccountProfile.h"

class WebDriverBrowserDriver : public BrowserDriver
{
public:
    explicit WebDriverBrowserDriver(QString baseUrl);

    QString backendName() const override;
    bool start(const TaskManager::Settings &settings, const QString &proxy, QString *error) override;
    void stop() override;
    bool openSignupPage(QString *error) override;
    bool fillRegistrationForm(const AccountProfile &profile, QString *error) override;
    bool submitRegistration(QString *error) override;
    bool handleVerification(const TaskManager::Settings &settings, QString *error) override;
    bool handleCaptcha(QString *error) override;

private:
    bool fillFirstMatchingInput(const QStringList &selectors,
                                const QString &text,
                                QString *error);
    bool clickFirstMatching(const QStringList &selectors, QString *error);
    void captureFailure(const QString &tag) const;

    TaskManager::Settings m_settings;
    AccountProfile m_lastProfile;
    WebDriverClient m_client;
    QString m_sessionId;
    QString m_proxy;
};

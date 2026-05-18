#include "bridge/MockBrowserDriver.h"

#include "bridge/CaptchaSolverFactory.h"

#include <QThread>

QString MockBrowserDriver::backendName() const
{
    return QStringLiteral("Mock");
}

bool MockBrowserDriver::start(const TaskManager::Settings &settings, const QString &proxy, QString *error)
{
    Q_UNUSED(proxy)
    m_settings = settings;
    m_stepDelayMs = qMax(200, (settings.delayMinMs + settings.delayMaxMs) / 8);
    if (error) {
        *error = QString();
    }
    return waitSlice(m_stepDelayMs, error);
}

void MockBrowserDriver::stop() {}

bool MockBrowserDriver::openSignupPage(QString *error)
{
    return waitSlice(m_stepDelayMs, error);
}

bool MockBrowserDriver::fillRegistrationForm(const AccountProfile &profile, QString *error)
{
    Q_UNUSED(profile)
    return waitSlice(m_stepDelayMs, error);
}

bool MockBrowserDriver::submitRegistration(QString *error)
{
    return waitSlice(m_stepDelayMs, error);
}

bool MockBrowserDriver::handleVerification(const TaskManager::Settings &settings, QString *error)
{
    if (!settings.apiEnabled) {
        if (error) {
            *error = QString();
        }
        return true;
    }
    if (settings.apiKey.isEmpty()) {
        if (error) {
            *error = QStringLiteral("API mode enabled but API key is empty.");
        }
        return false;
    }
    return waitSlice(m_stepDelayMs, error);
}

bool MockBrowserDriver::handleCaptcha(QString *error)
{
    auto solver = CaptchaSolverFactory::create(m_settings);
    CaptchaContext context;
    context.settings = m_settings;
    return solver->solve(context, error) && waitSlice(m_stepDelayMs, nullptr);
}

bool MockBrowserDriver::waitSlice(int ms, QString *error) const
{
    Q_UNUSED(error)
    QThread::msleep(static_cast<unsigned long>(ms));
    return true;
}

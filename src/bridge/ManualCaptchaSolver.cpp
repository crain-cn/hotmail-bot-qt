#include "bridge/ManualCaptchaSolver.h"

#include "bridge/WebDriverClient.h"

#include <QThread>

QString ManualCaptchaSolver::name() const
{
    return QStringLiteral("Manual");
}

bool ManualCaptchaSolver::solve(const CaptchaContext &context, QString *error)
{
    if (!context.client || context.sessionId.isEmpty()) {
        if (error) {
            *error = QString();
        }
        return true;
    }

    if (!context.client->isCaptchaPresent(context.sessionId)) {
        if (error) {
            *error = QString();
        }
        return true;
    }

    if (context.settings.headlessBrowser) {
        if (error) {
            *error = QStringLiteral("Captcha requires visible browser. Disable Headless.");
        }
        return false;
    }

    const int waitSec = qMax(30, context.settings.manualCaptchaWaitSec);
    const int stepMs = 2000;
    int waited = 0;
    while (waited < waitSec * 1000) {
        if (!context.client->isCaptchaPresent(context.sessionId)) {
            if (error) {
                *error = QString();
            }
            return true;
        }
        QThread::msleep(static_cast<unsigned long>(stepMs));
        waited += stepMs;
    }

    if (error) {
        *error = QStringLiteral("Manual captcha wait timed out after %1s.").arg(waitSec);
    }
    return false;
}

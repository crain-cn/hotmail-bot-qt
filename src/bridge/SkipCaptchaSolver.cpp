#include "bridge/SkipCaptchaSolver.h"

#include "bridge/WebDriverClient.h"

QString SkipCaptchaSolver::name() const
{
    return QStringLiteral("Skip");
}

bool SkipCaptchaSolver::solve(const CaptchaContext &context, QString *error)
{
    if (!context.client || context.sessionId.isEmpty()) {
        if (error) {
            *error = QString();
        }
        return true;
    }

    if (context.client->isCaptchaPresent(context.sessionId)) {
        if (error) {
            *error = QStringLiteral("Captcha visible and solver mode is Skip.");
        }
        return false;
    }
    if (error) {
        *error = QString();
    }
    return true;
}

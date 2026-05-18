#include "core/PageStateDetector.h"

#include "bridge/WebDriverClient.h"
#include "core/SignupSelectors.h"

namespace {

bool anyExists(const WebDriverClient &client,
               const QString &sessionId,
               const QStringList &selectors)
{
    for (const QString &selector : selectors) {
        bool exists = false;
        if (client.elementExists(sessionId, selector, &exists, nullptr) && exists) {
            return true;
        }
    }
    return false;
}

} // namespace

SignupPageState PageStateDetector::detect(const WebDriverClient &client, const QString &sessionId)
{
    if (client.isCaptchaPresent(sessionId)) {
        return SignupPageState::Captcha;
    }
    if (anyExists(client, sessionId, SignupSelectors::otpFields())) {
        return SignupPageState::Otp;
    }
    if (anyExists(client, sessionId, SignupSelectors::birthYearFields())) {
        return SignupPageState::Birthdate;
    }
    if (anyExists(client, sessionId, SignupSelectors::firstNameFields())) {
        return SignupPageState::Profile;
    }
    if (anyExists(client, sessionId, SignupSelectors::passwordFields())) {
        return SignupPageState::Password;
    }
    if (anyExists(client, sessionId, SignupSelectors::emailFields())) {
        return SignupPageState::Email;
    }

    QString url;
    if (client.getCurrentUrl(sessionId, &url, nullptr)) {
        if (url.contains(QStringLiteral("passkey"), Qt::CaseInsensitive)
            || url.contains(QStringLiteral("account.microsoft.com"), Qt::CaseInsensitive)) {
            return SignupPageState::Done;
        }
    }
    return SignupPageState::Unknown;
}

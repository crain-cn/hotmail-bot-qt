#include "core/SignupFlowRunner.h"

#include "bridge/WebDriverClient.h"
#include "core/PageStateDetector.h"
#include "core/SignupSelectors.h"

namespace {

bool fillInput(WebDriverClient &client,
               const QString &sessionId,
               const QStringList &selectors,
               const QString &text,
               QString *error)
{
    for (const QString &selector : selectors) {
        QString elementId;
        if (!client.waitForElement(sessionId,
                                   QStringLiteral("css selector"),
                                   selector,
                                   4000,
                                   &elementId,
                                   nullptr)) {
            continue;
        }
        return client.typeText(sessionId, elementId, text, error);
    }
    return false;
}

bool fillBirthdate(WebDriverClient &client, const QString &sessionId, const AccountProfile &profile)
{
    const QString month = QString::number(profile.birthDate.month());
    const QString day = QString::number(profile.birthDate.day());
    const QString year = QString::number(profile.birthDate.year());

    for (const QString &selector : SignupSelectors::birthMonthFields()) {
        client.setSelectValue(sessionId, selector, month, nullptr);
    }
    for (const QString &selector : SignupSelectors::birthDayFields()) {
        client.setSelectValue(sessionId, selector, day, nullptr);
    }
    for (const QString &selector : SignupSelectors::birthYearFields()) {
        client.setSelectValue(sessionId, selector, year, nullptr);
    }
    return true;
}

} // namespace

bool SignupFlowRunner::runWizard(WebDriverClient &client,
                                 const QString &sessionId,
                                 const AccountProfile &profile,
                                 QString *error)
{
    for (int step = 0; step < 20; ++step) {
        const SignupPageState state = PageStateDetector::detect(client, sessionId);

        switch (state) {
        case SignupPageState::Done:
            if (error) {
                error->clear();
            }
            return true;
        case SignupPageState::Captcha:
            if (error) {
                error->clear();
            }
            return true;
        case SignupPageState::Email:
            if (!fillInput(client, sessionId, SignupSelectors::emailFields(), profile.email, error)) {
                return false;
            }
            client.clickNextIfPresent(sessionId, SignupSelectors::nextStepButtons(), nullptr);
            break;
        case SignupPageState::Password:
            if (!fillInput(client, sessionId, SignupSelectors::passwordFields(), profile.password, error)) {
                return false;
            }
            client.clickNextIfPresent(sessionId, SignupSelectors::nextStepButtons(), nullptr);
            break;
        case SignupPageState::Profile:
            fillInput(client, sessionId, SignupSelectors::firstNameFields(), profile.firstName, nullptr);
            fillInput(client, sessionId, SignupSelectors::lastNameFields(), profile.lastName, nullptr);
            client.clickNextIfPresent(sessionId, SignupSelectors::nextStepButtons(), nullptr);
            break;
        case SignupPageState::Birthdate:
            fillBirthdate(client, sessionId, profile);
            client.clickNextIfPresent(sessionId, SignupSelectors::nextStepButtons(), nullptr);
            break;
        case SignupPageState::Otp:
            if (error) {
                error->clear();
            }
            return true;
        case SignupPageState::Unknown:
        default:
            client.clickNextIfPresent(sessionId, SignupSelectors::nextStepButtons(), nullptr);
            break;
        }
    }

    if (error) {
        *error = QStringLiteral("Signup wizard exceeded step limit.");
    }
    return false;
}

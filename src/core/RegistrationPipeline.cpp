#include "core/RegistrationPipeline.h"

#include "bridge/BrowserDriverFactory.h"
#include "core/RetryPolicy.h"

#include <QElapsedTimer>

RegistrationResult RegistrationPipeline::run(const AccountProfile &profile,
                                             const TaskManager::Settings &settings,
                                             const QString &proxy,
                                             int workerIndex,
                                             ProgressCallback onProgress,
                                             InterruptCheck shouldStop) const
{
    QElapsedTimer timer;
    timer.start();

    RegistrationResult result;
    result.workerIndex = workerIndex;
    result.email = profile.email;
    result.password = profile.password;

    QString error;

    auto fail = [&](RegistrationStage stage, const QString &message) {
        result.success = false;
        result.lastStage = stage;
        result.message = message;
        result.elapsedMs = timer.elapsed();
        if (onProgress) {
            onProgress(0, RegistrationStage::Failed);
        }
        return result;
    };

    if (!settings.hotmailEnabled && !settings.gmailEnabled && !settings.emailEnabled) {
        return fail(RegistrationStage::Init,
                    QStringLiteral("No registration mode enabled."));
    }

    auto driver = BrowserDriverFactory::create(settings);

    const auto runStep = [&](int percent,
                             RegistrationStage stage,
                             const std::function<bool(QString *)> &action) {
        if (shouldStop && shouldStop()) {
            error = QStringLiteral("Cancelled.");
            return false;
        }
        if (onProgress) {
            onProgress(percent, stage);
        }
        return RetryPolicy::run(settings.maxRetries, action, &error);
    };

    if (!runStep(5, RegistrationStage::Init, [&](QString *stepError) {
            return driver->start(settings, proxy, stepError);
        })) {
        driver->stop();
        return fail(RegistrationStage::Init, error);
    }

    if (!runStep(20, RegistrationStage::OpenSignup, [&](QString *stepError) {
            return driver->openSignupPage(stepError);
        })) {
        driver->stop();
        return fail(RegistrationStage::OpenSignup, error);
    }

    if (!runStep(45, RegistrationStage::FillForm, [&](QString *stepError) {
            return driver->fillRegistrationForm(profile, stepError);
        })) {
        driver->stop();
        return fail(RegistrationStage::FillForm, error);
    }

    if (!runStep(60, RegistrationStage::Submit, [&](QString *stepError) {
            return driver->submitRegistration(stepError);
        })) {
        driver->stop();
        return fail(RegistrationStage::Submit, error);
    }

    if (settings.apiEnabled) {
        if (!runStep(75, RegistrationStage::VerifyEmail, [&](QString *stepError) {
                return driver->handleVerification(settings, stepError);
            })) {
            driver->stop();
            return fail(RegistrationStage::VerifyEmail, error);
        }
    }

    if (!runStep(88, RegistrationStage::Captcha, [&](QString *stepError) {
            return driver->handleCaptcha(stepError);
        })) {
        driver->stop();
        return fail(RegistrationStage::Captcha, error);
    }

    driver->stop();

    if (onProgress) {
        onProgress(100, RegistrationStage::Complete);
    }

    result.success = true;
    result.lastStage = RegistrationStage::Complete;
    result.message = QStringLiteral("Registration flow finished via %1 backend.")
                         .arg(driver->backendName());
    result.elapsedMs = timer.elapsed();
    return result;
}

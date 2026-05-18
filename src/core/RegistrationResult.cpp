#include "core/RegistrationResult.h"

QString registrationStageName(RegistrationStage stage)
{
    switch (stage) {
    case RegistrationStage::Idle:
        return QStringLiteral("Idle");
    case RegistrationStage::Init:
        return QStringLiteral("Init");
    case RegistrationStage::OpenSignup:
        return QStringLiteral("OpenSignup");
    case RegistrationStage::FillForm:
        return QStringLiteral("FillForm");
    case RegistrationStage::Submit:
        return QStringLiteral("Submit");
    case RegistrationStage::VerifyEmail:
        return QStringLiteral("VerifyEmail");
    case RegistrationStage::Captcha:
        return QStringLiteral("Captcha");
    case RegistrationStage::Complete:
        return QStringLiteral("Complete");
    case RegistrationStage::Failed:
        return QStringLiteral("Failed");
    }
    return QStringLiteral("Unknown");
}

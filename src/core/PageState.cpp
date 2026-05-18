#include "core/PageState.h"

QString signupPageStateName(SignupPageState state)
{
    switch (state) {
    case SignupPageState::Unknown:
        return QStringLiteral("Unknown");
    case SignupPageState::Email:
        return QStringLiteral("Email");
    case SignupPageState::Password:
        return QStringLiteral("Password");
    case SignupPageState::Profile:
        return QStringLiteral("Profile");
    case SignupPageState::Birthdate:
        return QStringLiteral("Birthdate");
    case SignupPageState::Otp:
        return QStringLiteral("Otp");
    case SignupPageState::Captcha:
        return QStringLiteral("Captcha");
    case SignupPageState::Done:
        return QStringLiteral("Done");
    }
    return QStringLiteral("Unknown");
}

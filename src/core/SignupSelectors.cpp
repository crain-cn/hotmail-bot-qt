#include "core/SignupSelectors.h"

#include "core/SelectorConfig.h"

QStringList SignupSelectors::signupUrls()
{
    return SelectorConfig::instance().signupUrls();
}

QStringList SignupSelectors::nextStepButtons()
{
    return SelectorConfig::instance().nextStepButtons();
}

QStringList SignupSelectors::emailFields()
{
    return SelectorConfig::instance().emailFields();
}

QStringList SignupSelectors::passwordFields()
{
    return SelectorConfig::instance().passwordFields();
}

QStringList SignupSelectors::firstNameFields()
{
    return SelectorConfig::instance().firstNameFields();
}

QStringList SignupSelectors::lastNameFields()
{
    return SelectorConfig::instance().lastNameFields();
}

QStringList SignupSelectors::birthMonthFields()
{
    return SelectorConfig::instance().birthMonthFields();
}

QStringList SignupSelectors::birthDayFields()
{
    return SelectorConfig::instance().birthDayFields();
}

QStringList SignupSelectors::birthYearFields()
{
    return SelectorConfig::instance().birthYearFields();
}

QStringList SignupSelectors::submitButtons()
{
    return SelectorConfig::instance().submitButtons();
}

QStringList SignupSelectors::captchaMarkers()
{
    return SelectorConfig::instance().captchaMarkers();
}

QStringList SignupSelectors::otpFields()
{
    return SelectorConfig::instance().otpFields();
}

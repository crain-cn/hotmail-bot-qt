#pragma once

#include <QStringList>

class SignupSelectors
{
public:
    static QStringList signupUrls();
    static QStringList nextStepButtons();
    static QStringList emailFields();
    static QStringList passwordFields();
    static QStringList firstNameFields();
    static QStringList lastNameFields();
    static QStringList birthMonthFields();
    static QStringList birthDayFields();
    static QStringList birthYearFields();
    static QStringList submitButtons();
    static QStringList captchaMarkers();
    static QStringList otpFields();
};

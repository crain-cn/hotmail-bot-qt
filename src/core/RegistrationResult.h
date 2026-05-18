#pragma once

#include <QString>

enum class RegistrationStage {
    Idle,
    Init,
    OpenSignup,
    FillForm,
    Submit,
    VerifyEmail,
    Captcha,
    Complete,
    Failed
};

QString registrationStageName(RegistrationStage stage);

struct RegistrationResult {
    bool success = false;
    int workerIndex = -1;
    QString email;
    QString password;
    QString message;
    RegistrationStage lastStage = RegistrationStage::Idle;
    qint64 elapsedMs = 0;
};

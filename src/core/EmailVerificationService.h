#pragma once

#include <QString>

class EmailVerificationService
{
public:
    bool pollInbox(const QString &endpoint,
                   const QString &apiKey,
                   const QString &email,
                   int timeoutSec,
                   QString *otpCode,
                   QString *error) const;
};

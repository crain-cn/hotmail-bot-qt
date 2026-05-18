#pragma once

#include "core/AccountProfile.h"

class WebDriverClient;

class SignupFlowRunner
{
public:
    static bool runWizard(WebDriverClient &client,
                          const QString &sessionId,
                          const AccountProfile &profile,
                          QString *error);
};

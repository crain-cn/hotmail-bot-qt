#pragma once

#include "core/PageState.h"

class WebDriverClient;

class PageStateDetector
{
public:
    static SignupPageState detect(const WebDriverClient &client, const QString &sessionId);
};

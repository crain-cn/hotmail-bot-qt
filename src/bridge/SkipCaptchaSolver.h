#pragma once

#include "bridge/CaptchaSolver.h"

class SkipCaptchaSolver : public CaptchaSolver
{
public:
    QString name() const override;
    bool solve(const CaptchaContext &context, QString *error) override;
};

#pragma once

#include <functional>
#include <QString>

class RetryPolicy
{
public:
    static bool run(int maxAttempts,
                    const std::function<bool(QString *)> &action,
                    QString *error);
};

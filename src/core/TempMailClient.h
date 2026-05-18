#pragma once

#include <QString>

struct TempMailResult {
    bool success = false;
    QString code;
    QString message;
};

class TempMailClient
{
public:
    TempMailResult pollCode(const QString &endpointTemplate,
                            const QString &apiKey,
                            const QString &email,
                            int timeoutSec) const;
};

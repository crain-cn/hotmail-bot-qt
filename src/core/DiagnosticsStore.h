#pragma once

#include <QString>

class WebDriverClient;

class DiagnosticsStore
{
public:
    static QString outputDirectory();
    static bool saveFailureDump(WebDriverClient *client,
                                const QString &sessionId,
                                const QString &tag,
                                QString *savedPath,
                                QString *error);
};

#pragma once

#include <QString>

class ConfigSyncService
{
public:
    static bool syncGithubSelectors(const QString &rawUrl, QString *message);
    static bool resetSelectors(QString *message);
};

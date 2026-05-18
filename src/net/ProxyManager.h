#pragma once

#include <QNetworkProxy>
#include <QStringList>

class ProxyManager
{
public:
    static QStringList parseList(const QString &text);
    static bool applyToRequest(QNetworkProxy &proxy, const QString &entry);
    static QString pickRotating(const QStringList &proxies, int index);
};

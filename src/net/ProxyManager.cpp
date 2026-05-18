#include "net/ProxyManager.h"

#include <QUrl>

QStringList ProxyManager::parseList(const QString &text)
{
    QStringList lines;
    const QStringList raw = text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (QString line : raw) {
        line = line.trimmed();
        if (!line.isEmpty()) {
            lines.append(line);
        }
    }
    return lines;
}

bool ProxyManager::applyToRequest(QNetworkProxy &proxy, const QString &entry)
{
    QString normalized = entry.trimmed();
    if (normalized.isEmpty()) {
        return false;
    }

    if (!normalized.contains(QStringLiteral("://"))) {
        normalized.prepend(QStringLiteral("http://"));
    }

    const QUrl url(normalized);
    if (!url.isValid() || url.host().isEmpty()) {
        return false;
    }

    const QString scheme = url.scheme().toLower();
    if (scheme == QStringLiteral("socks5") || scheme == QStringLiteral("socks5h")) {
        proxy.setType(QNetworkProxy::Socks5Proxy);
    } else {
        proxy.setType(QNetworkProxy::HttpProxy);
    }

    proxy.setHostName(url.host());
    proxy.setPort(url.port(url.scheme() == QStringLiteral("https") ? 443 : 80));

    if (!url.userName().isEmpty()) {
        proxy.setUser(url.userName());
        proxy.setPassword(url.password());
    }

    return true;
}

QString ProxyManager::pickRotating(const QStringList &proxies, int index)
{
    if (proxies.isEmpty()) {
        return {};
    }
    return proxies.at(index % proxies.size());
}

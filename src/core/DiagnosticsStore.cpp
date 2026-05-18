#include "core/DiagnosticsStore.h"

#include "bridge/WebDriverClient.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

QString DiagnosticsStore::outputDirectory()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + QStringLiteral("/diagnostics");
    QDir().mkpath(dir);
    return dir;
}

bool DiagnosticsStore::saveFailureDump(WebDriverClient *client,
                                       const QString &sessionId,
                                       const QString &tag,
                                       QString *savedPath,
                                       QString *error)
{
    if (!client || sessionId.isEmpty()) {
        if (error) {
            *error = QStringLiteral("No active WebDriver session for diagnostics.");
        }
        return false;
    }

    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
    const QString base = outputDirectory() + QLatin1Char('/') + tag + QLatin1Char('_') + stamp;

    QString pngError;
    const QString pngPath = base + QStringLiteral(".png");
    if (client->saveScreenshot(sessionId, pngPath, &pngError)) {
        if (savedPath) {
            *savedPath = pngPath;
        }
    }

    QString html;
    if (client->getPageSource(sessionId, &html, &pngError)) {
        QFile htmlFile(base + QStringLiteral(".html"));
        if (htmlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            htmlFile.write(html.toUtf8());
            htmlFile.close();
        }
    }

    if (error) {
        error->clear();
    }
    return true;
}

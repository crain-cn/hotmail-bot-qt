#include "core/ConfigSyncService.h"

#include "core/SelectorConfig.h"

namespace {

QString defaultGithubSelectorsUrl()
{
    return QStringLiteral(
        "https://raw.githubusercontent.com/SeseydOw/Hotmail-Outlook-Create-Account-Register-Auto/main/hotmail-bot-qt/resources/config/signup_selectors.json");
}

} // namespace

bool ConfigSyncService::syncGithubSelectors(const QString &rawUrl, QString *message)
{
    const bool useDefaultUrl = rawUrl.trimmed().isEmpty();
    QString url = rawUrl.trimmed();
    if (useDefaultUrl) {
        url = defaultGithubSelectorsUrl();
    }

    QString error;
    if (SelectorConfig::instance().downloadFromUrl(url, &error)) {
        if (message) {
            *message = QStringLiteral("Selectors synced from Github.");
        }
        return true;
    }

    if (useDefaultUrl && resetSelectors(nullptr)) {
        if (message) {
            *message = QStringLiteral(
                             "Remote selectors unavailable (%1). Applied bundled defaults.")
                             .arg(error);
        }
        return true;
    }

    if (message) {
        *message = QStringLiteral("Github sync failed: %1").arg(error);
    }
    return false;
}

bool ConfigSyncService::resetSelectors(QString *message)
{
    if (!SelectorConfig::instance().saveDefaults()) {
        if (message) {
            *message = QStringLiteral("Failed to reset selectors.");
        }
        return false;
    }
    SelectorConfig::instance().load();
    if (message) {
        *message = QStringLiteral("Selectors reset to defaults.");
    }
    return true;
}

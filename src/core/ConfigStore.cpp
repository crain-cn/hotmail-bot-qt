#include "core/ConfigStore.h"

#include "net/ProxyManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSettings>
#include <QSpinBox>

namespace {

QSettings settings()
{
    return QSettings(QStringLiteral("HotmailBot"), QStringLiteral("HotmailBotQt"));
}

void applySettingsToUi(const UiConfigBindings &ui, const TaskManager::Settings &settings)
{
    if (ui.hotmail) {
        ui.hotmail->setChecked(settings.hotmailEnabled);
    }
    if (ui.email) {
        ui.email->setChecked(settings.emailEnabled);
    }
    if (ui.api) {
        ui.api->setChecked(settings.apiEnabled);
    }
    if (ui.gmail) {
        ui.gmail->setChecked(settings.gmailEnabled);
    }
    if (ui.delay) {
        ui.delay->setText(QStringLiteral("%1-%2")
                              .arg(settings.delayMinMs / 1000)
                              .arg(settings.delayMaxMs / 1000));
    }
    if (ui.bots) {
        ui.bots->setText(QString::number(settings.botCount));
    }
    if (ui.apiKey) {
        ui.apiKey->setText(settings.apiKey);
    }
    if (ui.tempMail) {
        ui.tempMail->setText(settings.tempMailEndpoint);
    }
    if (ui.batch && settings.batchIndex >= 0 && settings.batchIndex < ui.batch->count()) {
        ui.batch->setCurrentIndex(settings.batchIndex);
    }
    if (ui.numeric) {
        ui.numeric->setValue(settings.numericValue);
    }
    if (ui.proxies) {
        ui.proxies->setPlainText(settings.proxies.join(QLatin1Char('\n')));
    }
    if (ui.browser) {
        ui.browser->setCurrentIndex(settings.browserBackend == TaskManager::BrowserBackend::WebDriver
                                      ? 1
                                      : 0);
    }
    if (ui.webDriverUrl) {
        ui.webDriverUrl->setText(settings.webDriverUrl);
    }
    if (ui.headless) {
        ui.headless->setChecked(settings.headlessBrowser);
    }
    if (ui.captcha) {
        int captchaIndex = 0;
        switch (settings.captchaMode) {
        case TaskManager::CaptchaMode::Manual:
            captchaIndex = 1;
            break;
        case TaskManager::CaptchaMode::HttpApi:
            captchaIndex = 2;
            break;
        case TaskManager::CaptchaMode::Skip:
        default:
            captchaIndex = 0;
            break;
        }
        ui.captcha->setCurrentIndex(captchaIndex);
    }
    if (ui.captchaApiKey) {
        ui.captchaApiKey->setText(settings.captchaApiKey);
    }
    if (ui.retry) {
        ui.retry->setValue(settings.maxRetries);
    }
}

} // namespace

void ConfigStore::loadIntoUi(const UiConfigBindings &ui, TaskManager::Settings *settingsOut)
{
    QSettings s = settings();
    TaskManager::Settings loaded;
    loaded.hotmailEnabled = s.value(QStringLiteral("hotmail"), true).toBool();
    loaded.emailEnabled = s.value(QStringLiteral("email"), false).toBool();
    loaded.apiEnabled = s.value(QStringLiteral("api"), false).toBool();
    loaded.gmailEnabled = s.value(QStringLiteral("gmail"), false).toBool();
    loaded.botCount = s.value(QStringLiteral("bots"), 1).toInt();
    loaded.delayMinMs = s.value(QStringLiteral("delayMinMs"), 4000).toInt();
    loaded.delayMaxMs = s.value(QStringLiteral("delayMaxMs"), 12000).toInt();
    loaded.apiKey = s.value(QStringLiteral("apiKey")).toString();
    loaded.tempMailEndpoint = s.value(QStringLiteral("tempMailEndpoint")).toString();
    loaded.selectorsSyncUrl = s.value(QStringLiteral("selectorsSyncUrl")).toString();
    loaded.batchIndex = s.value(QStringLiteral("batchIndex"), 0).toInt();
    loaded.numericValue = s.value(QStringLiteral("numericValue"), 0).toInt();
    loaded.proxies = s.value(QStringLiteral("proxies")).toString().split(QLatin1Char('\n'),
                                                                         Qt::SkipEmptyParts);
    loaded.browserBackend = s.value(QStringLiteral("browserBackend"), 0).toInt() == 1
        ? TaskManager::BrowserBackend::WebDriver
        : TaskManager::BrowserBackend::Mock;
    loaded.webDriverUrl = s.value(QStringLiteral("webDriverUrl"), QStringLiteral("http://127.0.0.1:9515"))
                              .toString();
    loaded.headlessBrowser = s.value(QStringLiteral("headlessBrowser"), true).toBool();
    loaded.maxRetries = s.value(QStringLiteral("maxRetries"), 2).toInt();
    loaded.manualCaptchaWaitSec = s.value(QStringLiteral("manualCaptchaWaitSec"), 120).toInt();
    loaded.captchaApiKey = s.value(QStringLiteral("captchaApiKey")).toString();
    switch (s.value(QStringLiteral("captchaMode"), 0).toInt()) {
    case 1:
        loaded.captchaMode = TaskManager::CaptchaMode::Manual;
        break;
    case 2:
        loaded.captchaMode = TaskManager::CaptchaMode::HttpApi;
        break;
    default:
        loaded.captchaMode = TaskManager::CaptchaMode::Skip;
        break;
    }

    applySettingsToUi(ui, loaded);
    if (settingsOut) {
        *settingsOut = loaded;
    }
}

void ConfigStore::saveFromUi(const UiConfigBindings &ui)
{
    const TaskManager::Settings values = readSettings(ui);
    QSettings s = settings();
    s.setValue(QStringLiteral("hotmail"), values.hotmailEnabled);
    s.setValue(QStringLiteral("email"), values.emailEnabled);
    s.setValue(QStringLiteral("api"), values.apiEnabled);
    s.setValue(QStringLiteral("gmail"), values.gmailEnabled);
    s.setValue(QStringLiteral("bots"), values.botCount);
    s.setValue(QStringLiteral("delayMinMs"), values.delayMinMs);
    s.setValue(QStringLiteral("delayMaxMs"), values.delayMaxMs);
    s.setValue(QStringLiteral("apiKey"), values.apiKey);
    s.setValue(QStringLiteral("tempMailEndpoint"), values.tempMailEndpoint);
    s.setValue(QStringLiteral("selectorsSyncUrl"), values.selectorsSyncUrl);
    s.setValue(QStringLiteral("batchIndex"), values.batchIndex);
    s.setValue(QStringLiteral("numericValue"), values.numericValue);
    s.setValue(QStringLiteral("proxies"), values.proxies.join(QLatin1Char('\n')));
    s.setValue(QStringLiteral("browserBackend"),
               static_cast<int>(values.browserBackend));
    s.setValue(QStringLiteral("webDriverUrl"), values.webDriverUrl);
    s.setValue(QStringLiteral("headlessBrowser"), values.headlessBrowser);
    s.setValue(QStringLiteral("maxRetries"), values.maxRetries);
    s.setValue(QStringLiteral("manualCaptchaWaitSec"), values.manualCaptchaWaitSec);
    s.setValue(QStringLiteral("captchaApiKey"), values.captchaApiKey);
    s.setValue(QStringLiteral("captchaMode"), static_cast<int>(values.captchaMode));
}

TaskManager::Settings ConfigStore::readSettings(const UiConfigBindings &ui)
{
    TaskManager::Settings settings;
    if (ui.hotmail) {
        settings.hotmailEnabled = ui.hotmail->isChecked();
    }
    if (ui.email) {
        settings.emailEnabled = ui.email->isChecked();
    }
    if (ui.api) {
        settings.apiEnabled = ui.api->isChecked();
    }
    if (ui.gmail) {
        settings.gmailEnabled = ui.gmail->isChecked();
    }
    if (ui.apiKey) {
        settings.apiKey = ui.apiKey->text().trimmed();
    }
    if (ui.tempMail) {
        settings.tempMailEndpoint = ui.tempMail->text().trimmed();
    }
    if (ui.proxies) {
        settings.proxies = ProxyManager::parseList(ui.proxies->toPlainText());
    }
    if (ui.batch) {
        settings.batchIndex = ui.batch->currentIndex();
    }
    if (ui.numeric) {
        settings.numericValue = ui.numeric->value();
        settings.manualCaptchaWaitSec = qMax(30, settings.numericValue);
    }
    if (ui.browser) {
        settings.browserBackend = ui.browser->currentIndex() == 1
            ? TaskManager::BrowserBackend::WebDriver
            : TaskManager::BrowserBackend::Mock;
    }
    if (ui.webDriverUrl) {
        settings.webDriverUrl = ui.webDriverUrl->text().trimmed();
        if (settings.webDriverUrl.isEmpty()) {
            settings.webDriverUrl = QStringLiteral("http://127.0.0.1:9515");
        }
    }
    if (ui.headless) {
        settings.headlessBrowser = ui.headless->isChecked();
    }
    if (ui.captcha) {
        switch (ui.captcha->currentIndex()) {
        case 1:
            settings.captchaMode = TaskManager::CaptchaMode::Manual;
            break;
        case 2:
            settings.captchaMode = TaskManager::CaptchaMode::HttpApi;
            break;
        default:
            settings.captchaMode = TaskManager::CaptchaMode::Skip;
            break;
        }
    }
    if (ui.captchaApiKey) {
        settings.captchaApiKey = ui.captchaApiKey->text().trimmed();
    }
    if (ui.retry) {
        settings.maxRetries = ui.retry->value();
    }
    if (ui.bots) {
        bool ok = false;
        const int bots = ui.bots->text().trimmed().toInt(&ok);
        settings.botCount = ok && bots > 0 ? bots : 1;
    }

    if (ui.delay) {
        const QStringList parts = ui.delay->text().trimmed().split(QLatin1Char('-'));
        if (parts.size() == 2) {
            bool minOk = false;
            bool maxOk = false;
            const int minSec = parts.at(0).trimmed().toInt(&minOk);
            const int maxSec = parts.at(1).trimmed().toInt(&maxOk);
            if (minOk && maxOk && minSec > 0 && maxSec >= minSec) {
                settings.delayMinMs = minSec * 1000;
                settings.delayMaxMs = maxSec * 1000;
            }
        }
    }

    if (settings.batchIndex >= 0 && settings.batchIndex < 4) {
        static const int ranges[4][2] = {{1, 5}, {10, 50}, {100, 500}, {500, 1000}};
        const int minBots = ranges[settings.batchIndex][0];
        const int maxBots = ranges[settings.batchIndex][1];
        if (settings.botCount < minBots || settings.botCount > maxBots) {
            settings.botCount = minBots;
        }
    }

    return settings;
}

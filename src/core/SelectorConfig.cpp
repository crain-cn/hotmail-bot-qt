#include "core/SelectorConfig.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QStandardPaths>

namespace {

constexpr auto kBundledSelectorsResource = ":/config/signup_selectors.json";

QStringList fromArray(const QJsonArray &array)
{
    QStringList list;
    for (const QJsonValue &value : array) {
        if (value.isString()) {
            list.append(value.toString());
        }
    }
    return list;
}

QJsonObject loadJsonObject(QIODevice &device)
{
    const QJsonDocument doc = QJsonDocument::fromJson(device.readAll());
    if (!doc.isObject()) {
        return {};
    }
    return doc.object();
}

QJsonObject bundledRoot()
{
    QFile file(QString::fromUtf8(kBundledSelectorsResource));
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return loadJsonObject(file);
}

QJsonObject defaultRoot()
{
    return bundledRoot();
}

} // namespace

SelectorConfig &SelectorConfig::instance()
{
    static SelectorConfig config;
    return config;
}

SelectorConfig::SelectorConfig()
{
    m_root = defaultRoot();
    load();
}

QString SelectorConfig::configPath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/signup_selectors.json");
}

bool SelectorConfig::saveDefaults() const
{
    const QJsonObject root = defaultRoot();
    if (root.isEmpty()) {
        return false;
    }

    QFile file(configPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool SelectorConfig::load()
{
    QFile file(configPath());
    if (!file.exists()) {
        saveDefaults();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        m_root = defaultRoot();
        m_loaded = !m_root.isEmpty();
        return m_loaded;
    }

    const QJsonObject loaded = loadJsonObject(file);
    if (loaded.isEmpty()) {
        m_root = defaultRoot();
        m_loaded = !m_root.isEmpty();
        return m_loaded;
    }

    m_root = loaded;
    m_loaded = true;
    return true;
}

bool SelectorConfig::importFromFile(const QString &path, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = QStringLiteral("Cannot read selector file: %1").arg(path);
        }
        return false;
    }

    const QJsonObject imported = loadJsonObject(file);
    if (imported.isEmpty()) {
        if (error) {
            *error = QStringLiteral("Invalid selector JSON.");
        }
        return false;
    }

    QFile out(configPath());
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) {
            *error = QStringLiteral("Cannot write selector config.");
        }
        return false;
    }
    out.write(QJsonDocument(imported).toJson(QJsonDocument::Indented));
    m_root = imported;
    m_loaded = true;
    return true;
}

bool SelectorConfig::downloadFromUrl(const QString &url, QString *error)
{
    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const QByteArray body = reply->readAll();
    const QString networkMessage = reply->errorString();
    const auto networkError = reply->error();
    reply->deleteLater();

    if (networkError != QNetworkReply::NoError) {
        if (error) {
            *error = networkMessage;
        }
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        if (error) {
            *error = QStringLiteral("Downloaded selectors are not valid JSON.");
        }
        return false;
    }

    QFile out(configPath());
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) {
            *error = QStringLiteral("Cannot save downloaded selectors.");
        }
        return false;
    }
    out.write(doc.toJson(QJsonDocument::Indented));
    m_root = doc.object();
    m_loaded = true;
    return true;
}

QStringList SelectorConfig::readList(const char *key, const QStringList &fallback) const
{
    Q_UNUSED(fallback)
    QStringList values = fromArray(m_root.value(QString::fromUtf8(key)).toArray());
    if (!values.isEmpty()) {
        return values;
    }
    return fromArray(defaultRoot().value(QString::fromUtf8(key)).toArray());
}

QStringList SelectorConfig::signupUrls() const
{
    return readList("signupUrls", {});
}

QStringList SelectorConfig::nextStepButtons() const
{
    return readList("nextStepButtons", {});
}

QStringList SelectorConfig::emailFields() const
{
    return readList("emailFields", {});
}

QStringList SelectorConfig::passwordFields() const
{
    return readList("passwordFields", {});
}

QStringList SelectorConfig::firstNameFields() const
{
    return readList("firstNameFields", {});
}

QStringList SelectorConfig::lastNameFields() const
{
    return readList("lastNameFields", {});
}

QStringList SelectorConfig::birthMonthFields() const
{
    return readList("birthMonthFields", {});
}

QStringList SelectorConfig::birthDayFields() const
{
    return readList("birthDayFields", {});
}

QStringList SelectorConfig::birthYearFields() const
{
    return readList("birthYearFields", {});
}

QStringList SelectorConfig::submitButtons() const
{
    return readList("submitButtons", {});
}

QStringList SelectorConfig::captchaMarkers() const
{
    return readList("captchaMarkers", {});
}

QStringList SelectorConfig::otpFields() const
{
    return readList("otpFields", {});
}

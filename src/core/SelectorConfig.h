#pragma once

#include <QJsonObject>
#include <QStringList>

class SelectorConfig
{
public:
    static SelectorConfig &instance();

    QString configPath() const;
    bool load();
    bool saveDefaults() const;
    bool importFromFile(const QString &path, QString *error);
    bool downloadFromUrl(const QString &url, QString *error);

    QStringList signupUrls() const;
    QStringList nextStepButtons() const;
    QStringList emailFields() const;
    QStringList passwordFields() const;
    QStringList firstNameFields() const;
    QStringList lastNameFields() const;
    QStringList birthMonthFields() const;
    QStringList birthDayFields() const;
    QStringList birthYearFields() const;
    QStringList submitButtons() const;
    QStringList captchaMarkers() const;
    QStringList otpFields() const;

private:
    SelectorConfig();
    QStringList readList(const char *key, const QStringList &fallback) const;

    QJsonObject m_root;
    bool m_loaded = false;
};

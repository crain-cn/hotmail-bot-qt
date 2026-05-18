#pragma once

#include <QJsonObject>
#include <QString>

class WebDriverClient
{
public:
    explicit WebDriverClient(QString baseUrl);

    bool isReachable(QString *error) const;
    bool createSession(const QJsonObject &capabilities, QString *sessionId, QString *error);
    bool deleteSession(const QString &sessionId, QString *error);
    bool navigate(const QString &sessionId, const QString &url, QString *error);
    bool findElement(const QString &sessionId,
                     const QString &usingStrategy,
                     const QString &value,
                     QString *elementId,
                     QString *error) const;
    bool typeText(const QString &sessionId,
                  const QString &elementId,
                  const QString &text,
                  QString *error) const;
    bool clickElement(const QString &sessionId, const QString &elementId, QString *error) const;
    bool getCurrentUrl(const QString &sessionId, QString *url, QString *error) const;
    bool waitForElement(const QString &sessionId,
                        const QString &usingStrategy,
                        const QString &value,
                        int timeoutMs,
                        QString *elementId,
                        QString *error) const;
    bool elementExists(const QString &sessionId,
                       const QString &cssSelector,
                       bool *exists,
                       QString *error) const;
    bool isCaptchaPresent(const QString &sessionId) const;
    bool getPageSource(const QString &sessionId, QString *source, QString *error) const;
    bool saveScreenshot(const QString &sessionId, const QString &filePath, QString *error) const;
    bool clickNextIfPresent(const QString &sessionId, const QStringList &selectors, QString *error) const;
    bool setSelectValue(const QString &sessionId,
                        const QString &cssSelector,
                        const QString &value,
                        QString *error) const;

private:
    bool executeScript(const QString &sessionId,
                       const QString &script,
                       QJsonObject *response,
                       QString *error) const;
    bool request(const QString &method,
                 const QString &path,
                 const QJsonObject &body,
                 QJsonObject *response,
                 QString *error) const;

    QString m_baseUrl;
};

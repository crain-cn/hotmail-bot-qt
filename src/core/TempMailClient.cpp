#include "core/TempMailClient.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QRegularExpression>
#include <QThread>
#include <QUrl>

namespace {

QString extractCode(const QString &text)
{
    static const QRegularExpression pattern(QStringLiteral("\\b(\\d{4,8})\\b"));
    const QRegularExpressionMatch match = pattern.match(text);
    if (match.hasMatch()) {
        return match.captured(1);
    }
    return {};
}

QString resolveEndpoint(QString endpointTemplate, const QString &email, const QString &apiKey)
{
    endpointTemplate.replace(QStringLiteral("{email}"), QUrl::toPercentEncoding(email));
    endpointTemplate.replace(QStringLiteral("{apiKey}"), QUrl::toPercentEncoding(apiKey));
    return endpointTemplate;
}

QString findCodeInJson(const QJsonValue &value)
{
    if (value.isString()) {
        return extractCode(value.toString());
    }
    if (value.isObject()) {
        const QJsonObject obj = value.toObject();
        const QStringList keys = {QStringLiteral("code"),
                                  QStringLiteral("otp"),
                                  QStringLiteral("verificationCode"),
                                  QStringLiteral("text"),
                                  QStringLiteral("body"),
                                  QStringLiteral("subject")};
        for (const QString &key : keys) {
            if (!obj.contains(key)) {
                continue;
            }
            const QString code = findCodeInJson(obj.value(key));
            if (!code.isEmpty()) {
                return code;
            }
        }
    }
    if (value.isArray()) {
        for (const QJsonValue &item : value.toArray()) {
            const QString code = findCodeInJson(item);
            if (!code.isEmpty()) {
                return code;
            }
        }
    }
    return {};
}

} // namespace

TempMailResult TempMailClient::pollCode(const QString &endpointTemplate,
                                        const QString &apiKey,
                                        const QString &email,
                                        int timeoutSec) const
{
    TempMailResult result;
    if (endpointTemplate.trimmed().isEmpty()) {
        result.message = QStringLiteral("Temp-mail endpoint is empty.");
        return result;
    }

    const int maxSec = qMax(10, timeoutSec);
    const QUrl endpoint(resolveEndpoint(endpointTemplate, email, apiKey));
    if (!endpoint.isValid()) {
        result.message = QStringLiteral("Temp-mail endpoint URL is invalid.");
        return result;
    }

    QNetworkAccessManager manager;
    for (int elapsed = 0; elapsed < maxSec; elapsed += 3) {
        QNetworkRequest request(endpoint);
        if (!apiKey.isEmpty() && !endpointTemplate.contains(QStringLiteral("{apiKey}"))) {
            request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());
        }

        QNetworkReply *reply = manager.get(request);
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        const QByteArray raw = reply->readAll();
        reply->deleteLater();

        const QJsonDocument doc = QJsonDocument::fromJson(raw);
        QString code;
        if (doc.isArray()) {
            code = findCodeInJson(doc.array());
        } else if (doc.isObject()) {
            code = findCodeInJson(doc.object());
        } else {
            code = extractCode(QString::fromUtf8(raw));
        }
        if (!code.isEmpty()) {
            result.success = true;
            result.code = code;
            result.message = QStringLiteral("OTP received.");
            return result;
        }

        QThread::msleep(3000);
    }

    result.message = QStringLiteral("Temp-mail polling timed out.");
    return result;
}

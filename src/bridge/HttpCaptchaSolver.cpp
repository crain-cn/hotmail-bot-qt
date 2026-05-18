#include "bridge/HttpCaptchaSolver.h"

#include "bridge/ManualCaptchaSolver.h"
#include "bridge/WebDriverClient.h"
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QThread>
#include <QUrl>
#include <QUrlQuery>

namespace {

bool httpGet(const QUrl &url, QString *body, QString *error)
{
    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const QString text = QString::fromUtf8(reply->readAll());
    const auto networkError = reply->error();
    const QString networkMessage = reply->errorString();
    reply->deleteLater();

    if (networkError != QNetworkReply::NoError) {
        if (error) {
            *error = networkMessage;
        }
        return false;
    }
    if (body) {
        *body = text.trimmed();
    }
    return true;
}

} // namespace

QString HttpCaptchaSolver::name() const
{
    return QStringLiteral("HttpApi");
}

bool HttpCaptchaSolver::solve(const CaptchaContext &context, QString *error)
{
    if (!context.client || context.sessionId.isEmpty()) {
        if (error) {
            *error = QString();
        }
        return true;
    }

    if (!context.client->isCaptchaPresent(context.sessionId)) {
        if (error) {
            *error = QString();
        }
        return true;
    }

    if (context.settings.captchaApiKey.isEmpty()) {
        if (error) {
            *error = QStringLiteral("Captcha API key is empty. Falling back to manual wait.");
        }
        ManualCaptchaSolver manual;
        return manual.solve(context, error);
    }

    QString pageUrl;
    if (!context.client->getCurrentUrl(context.sessionId, &pageUrl, error)) {
        return false;
    }

    const QUrl submitUrl(QStringLiteral("https://2captcha.com/in.php"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("key"), context.settings.captchaApiKey);
    query.addQueryItem(QStringLiteral("method"), QStringLiteral("userrecaptcha"));
    query.addQueryItem(QStringLiteral("pageurl"), pageUrl);
    query.addQueryItem(QStringLiteral("json"), QStringLiteral("1"));

    QUrl submit = submitUrl;
    submit.setQuery(query);

    QString submitBody;
    if (!httpGet(submit, &submitBody, error)) {
        return false;
    }

    if (!submitBody.contains(QStringLiteral("\"status\":1"))
        && !submitBody.startsWith(QStringLiteral("OK|"))) {
        ManualCaptchaSolver manual;
        if (error) {
            *error = QStringLiteral("Captcha API submit failed (%1). Manual fallback.")
                         .arg(submitBody.left(120));
        }
        return manual.solve(context, error);
    }

    QString taskId;
    if (submitBody.contains(QStringLiteral("request"))) {
        const int idx = submitBody.indexOf(QStringLiteral("request"));
        taskId = submitBody.mid(idx).section(QLatin1Char('"'), 3, 3);
    } else {
        taskId = submitBody.section(QLatin1Char('|'), 1, 1);
    }

    const int maxPollSec = qMax(30, context.settings.manualCaptchaWaitSec);
    for (int i = 0; i < maxPollSec; i += 5) {
        QThread::msleep(5000);

        const QUrl resultBase(QStringLiteral("https://2captcha.com/res.php"));
        QUrlQuery resultQuery;
        resultQuery.addQueryItem(QStringLiteral("key"), context.settings.captchaApiKey);
        resultQuery.addQueryItem(QStringLiteral("action"), QStringLiteral("get"));
        resultQuery.addQueryItem(QStringLiteral("id"), taskId);
        resultQuery.addQueryItem(QStringLiteral("json"), QStringLiteral("1"));
        QUrl resultUrl = resultBase;
        resultUrl.setQuery(resultQuery);

        QString pollBody;
        if (!httpGet(resultUrl, &pollBody, error)) {
            return false;
        }

        if (pollBody.contains(QStringLiteral("CAPCHA_NOT_READY"))) {
            continue;
        }
        if (pollBody.contains(QStringLiteral("OK|"))
            || pollBody.contains(QStringLiteral("\"status\":1"))) {
            if (error) {
                *error = QString();
            }
            return true;
        }
    }

    ManualCaptchaSolver manual;
    if (error) {
        *error = QStringLiteral("Captcha API polling timed out. Manual fallback.");
    }
    return manual.solve(context, error);
}

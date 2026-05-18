#include "bridge/WebDriverClient.h"

#include "core/SignupSelectors.h"

#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QThread>
#include <QUrl>

namespace {

QString normalizedBase(QString url)
{
    url = url.trimmed();
    while (url.endsWith(QLatin1Char('/'))) {
        url.chop(1);
    }
    return url;
}

QString elementIdFromResponse(const QJsonObject &response)
{
    const QJsonValue value = response.value(QStringLiteral("value"));
    if (value.isString()) {
        return value.toString();
    }
    if (value.isObject()) {
        return value.toObject().value(QStringLiteral("ELEMENT")).toString();
    }
    return {};
}

} // namespace

WebDriverClient::WebDriverClient(QString baseUrl)
    : m_baseUrl(normalizedBase(std::move(baseUrl)))
{
}

bool WebDriverClient::isReachable(QString *error) const
{
    QJsonObject ignored;
    return request(QStringLiteral("GET"), QStringLiteral("/status"), {}, &ignored, error);
}

bool WebDriverClient::createSession(const QJsonObject &capabilities,
                                    QString *sessionId,
                                    QString *error)
{
    QJsonObject body;
    body.insert(QStringLiteral("capabilities"), capabilities);

    QJsonObject response;
    if (!request(QStringLiteral("POST"), QStringLiteral("/session"), body, &response, error)) {
        return false;
    }

    const QJsonObject value = response.value(QStringLiteral("value")).toObject();
    const QString id = value.value(QStringLiteral("sessionId")).toString();
    if (id.isEmpty()) {
        if (error) {
            *error = QStringLiteral("WebDriver sessionId missing in response.");
        }
        return false;
    }

    if (sessionId) {
        *sessionId = id;
    }
    return true;
}

bool WebDriverClient::deleteSession(const QString &sessionId, QString *error)
{
    QJsonObject ignored;
    return request(QStringLiteral("DELETE"),
                   QStringLiteral("/session/") + sessionId,
                   {},
                   &ignored,
                   error);
}

bool WebDriverClient::navigate(const QString &sessionId, const QString &url, QString *error)
{
    QJsonObject body;
    body.insert(QStringLiteral("url"), url);
    QJsonObject ignored;
    return request(QStringLiteral("POST"),
                   QStringLiteral("/session/") + sessionId + QStringLiteral("/url"),
                   body,
                   &ignored,
                   error);
}

bool WebDriverClient::findElement(const QString &sessionId,
                                  const QString &usingStrategy,
                                  const QString &value,
                                  QString *elementId,
                                  QString *error) const
{
    QJsonObject body;
    body.insert(QStringLiteral("using"), usingStrategy);
    body.insert(QStringLiteral("value"), value);

    QJsonObject response;
    if (!request(QStringLiteral("POST"),
                 QStringLiteral("/session/") + sessionId + QStringLiteral("/element"),
                 body,
                 &response,
                 error)) {
        return false;
    }

    const QString id = elementIdFromResponse(response);
    if (id.isEmpty()) {
        if (error) {
            *error = QStringLiteral("Element not found: %1").arg(value);
        }
        return false;
    }
    if (elementId) {
        *elementId = id;
    }
    return true;
}

bool WebDriverClient::typeText(const QString &sessionId,
                               const QString &elementId,
                               const QString &text,
                               QString *error) const
{
    QJsonObject body;
    body.insert(QStringLiteral("text"), text);
    QJsonObject ignored;
    return request(QStringLiteral("POST"),
                   QStringLiteral("/session/") + sessionId + QStringLiteral("/element/")
                       + elementId + QStringLiteral("/value"),
                   body,
                   &ignored,
                   error);
}

bool WebDriverClient::clickElement(const QString &sessionId,
                                   const QString &elementId,
                                   QString *error) const
{
    QJsonObject ignored;
    return request(QStringLiteral("POST"),
                   QStringLiteral("/session/") + sessionId + QStringLiteral("/element/")
                       + elementId + QStringLiteral("/click"),
                   {},
                   &ignored,
                   error);
}

bool WebDriverClient::getCurrentUrl(const QString &sessionId, QString *url, QString *error) const
{
    QJsonObject response;
    if (!request(QStringLiteral("GET"),
                 QStringLiteral("/session/") + sessionId + QStringLiteral("/url"),
                 {},
                 &response,
                 error)) {
        return false;
    }
    if (url) {
        *url = response.value(QStringLiteral("value")).toString();
    }
    return true;
}

bool WebDriverClient::waitForElement(const QString &sessionId,
                                     const QString &usingStrategy,
                                     const QString &value,
                                     int timeoutMs,
                                     QString *elementId,
                                     QString *error) const
{
    const int stepMs = 500;
    int waited = 0;
    while (waited <= timeoutMs) {
        QString foundId;
        QString localError;
        if (findElement(sessionId, usingStrategy, value, &foundId, &localError)) {
            if (elementId) {
                *elementId = foundId;
            }
            if (error) {
                *error = QString();
            }
            return true;
        }
        QThread::msleep(static_cast<unsigned long>(stepMs));
        waited += stepMs;
    }
    if (error) {
        *error = QStringLiteral("Timeout waiting for element: %1").arg(value);
    }
    return false;
}

bool WebDriverClient::elementExists(const QString &sessionId,
                                    const QString &cssSelector,
                                    bool *exists,
                                    QString *error) const
{
    QString elementId;
    QString localError;
    const bool found = findElement(sessionId,
                                   QStringLiteral("css selector"),
                                   cssSelector,
                                   &elementId,
                                   &localError);
    if (exists) {
        *exists = found;
    }
    if (error) {
        error->clear();
    }
    return true;
}

bool WebDriverClient::isCaptchaPresent(const QString &sessionId) const
{
    for (const QString &selector : SignupSelectors::captchaMarkers()) {
        bool exists = false;
        elementExists(sessionId, selector, &exists, nullptr);
        if (exists) {
            return true;
        }
    }
    return false;
}

bool WebDriverClient::getPageSource(const QString &sessionId, QString *source, QString *error) const
{
    QJsonObject response;
    if (!request(QStringLiteral("GET"),
                 QStringLiteral("/session/") + sessionId + QStringLiteral("/source"),
                 {},
                 &response,
                 error)) {
        return false;
    }
    if (source) {
        *source = response.value(QStringLiteral("value")).toString();
    }
    return true;
}

bool WebDriverClient::saveScreenshot(const QString &sessionId,
                                       const QString &filePath,
                                       QString *error) const
{
    QJsonObject response;
    if (!request(QStringLiteral("GET"),
                 QStringLiteral("/session/") + sessionId + QStringLiteral("/screenshot"),
                 {},
                 &response,
                 error)) {
        return false;
    }

    const QString base64 = response.value(QStringLiteral("value")).toString();
    const QByteArray bytes = QByteArray::fromBase64(base64.toUtf8());
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error) {
            *error = QStringLiteral("Cannot write screenshot: %1").arg(filePath);
        }
        return false;
    }
    file.write(bytes);
    file.close();
    return true;
}

bool WebDriverClient::setSelectValue(const QString &sessionId,
                                     const QString &cssSelector,
                                     const QString &value,
                                     QString *error) const
{
    const QString escapedValue = value;
    const QString script = QStringLiteral(
                               "var el=document.querySelector('%1');"
                               "if(!el){return false;}"
                               "el.value='%2';"
                               "el.dispatchEvent(new Event('change',{bubbles:true}));"
                               "return true;")
                               .arg(cssSelector, escapedValue);

    QJsonObject response;
    if (!executeScript(sessionId, script, &response, error)) {
        return false;
    }

    return response.value(QStringLiteral("value")).toBool();
}

bool WebDriverClient::clickNextIfPresent(const QString &sessionId,
                                         const QStringList &selectors,
                                         QString *error) const
{
    for (const QString &selector : selectors) {
        QString elementId;
        if (!waitForElement(sessionId,
                            QStringLiteral("css selector"),
                            selector,
                            1500,
                            &elementId,
                            nullptr)) {
            continue;
        }
        return clickElement(sessionId, elementId, error);
    }
    if (error) {
        error->clear();
    }
    return true;
}

bool WebDriverClient::executeScript(const QString &sessionId,
                                    const QString &script,
                                    QJsonObject *response,
                                    QString *error) const
{
    QJsonObject body;
    body.insert(QStringLiteral("script"), script);
    body.insert(QStringLiteral("args"), QJsonArray{});
    return request(QStringLiteral("POST"),
                   QStringLiteral("/session/") + sessionId + QStringLiteral("/execute/sync"),
                   body,
                   response,
                   error);
}

bool WebDriverClient::request(const QString &method,
                              const QString &path,
                              const QJsonObject &body,
                              QJsonObject *response,
                              QString *error) const
{
    QNetworkAccessManager manager;
    const QUrl url(m_baseUrl + path);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QNetworkReply *reply = nullptr;
    const QByteArray payload = body.isEmpty() ? QByteArray() : QJsonDocument(body).toJson();

    if (method == QStringLiteral("GET")) {
        reply = manager.get(request);
    } else if (method == QStringLiteral("POST")) {
        reply = manager.post(request, payload);
    } else if (method == QStringLiteral("DELETE")) {
        reply = manager.deleteResource(request);
    } else {
        if (error) {
            *error = QStringLiteral("Unsupported HTTP method: %1").arg(method);
        }
        return false;
    }

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray raw = reply->readAll();
    const auto networkError = reply->error();
    const QString networkMessage = reply->errorString();
    reply->deleteLater();

    if (networkError != QNetworkReply::NoError && status == 0) {
        if (error) {
            *error = QStringLiteral("WebDriver HTTP error: %1").arg(networkMessage);
        }
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &parseError);
    if (!doc.isObject()) {
        if (status >= 200 && status < 300 && raw.isEmpty()) {
            if (response) {
                *response = {};
            }
            return true;
        }
        if (error) {
            *error = QStringLiteral("Invalid WebDriver JSON: %1").arg(parseError.errorString());
        }
        return false;
    }

    const QJsonObject obj = doc.object();
    if (status >= 400) {
        const QJsonObject value = obj.value(QStringLiteral("value")).toObject();
        const QString message = value.value(QStringLiteral("message")).toString();
        if (error) {
            *error = message.isEmpty() ? QStringLiteral("WebDriver HTTP %1").arg(status) : message;
        }
        return false;
    }

    if (response) {
        *response = obj;
    }
    return true;
}

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

QByteArray stripBom(QByteArray raw)
{
    if (raw.startsWith("\xEF\xBB\xBF")) {
        raw.remove(0, 3);
    }
    return raw;
}

QByteArray requestPayload(const QJsonObject &body)
{
    if (body.isEmpty()) {
        return QByteArrayLiteral("{}");
    }
    return QJsonDocument(body).toJson(QJsonDocument::Compact);
}

QString parseFailureHint(const QByteArray &raw, int status)
{
    const QString prefix = QString::fromUtf8(raw.left(120)).trimmed();
    if (prefix.startsWith(QLatin1Char('<'))
        || prefix.contains(QStringLiteral("<!DOCTYPE"), Qt::CaseInsensitive)
        || prefix.contains(QStringLiteral("<html"), Qt::CaseInsensitive)) {
        return QStringLiteral(
            " Response is HTML, not JSON. Check Driver URL (use http://127.0.0.1:9515) and ensure ChromeDriver is running.");
    }
    if (raw.contains("Infinity") || raw.contains("NaN")) {
        return QStringLiteral(
            " Response contains non-standard JSON numbers. The endpoint may not be ChromeDriver.");
    }
    if (status == 0) {
        return QStringLiteral(" No HTTP status received. Is ChromeDriver running?");
    }
    return {};
}

QString elementIdFromResponse(const QJsonObject &response)
{
    const QJsonValue value = response.value(QStringLiteral("value"));
    if (value.isString()) {
        return value.toString();
    }
    if (!value.isObject()) {
        return {};
    }

    const QJsonObject obj = value.toObject();
    const QString legacy = obj.value(QStringLiteral("ELEMENT")).toString();
    if (!legacy.isEmpty()) {
        return legacy;
    }

    for (auto it = obj.begin(); it != obj.end(); ++it) {
        if (it.value().isString()) {
            return it.value().toString();
        }
    }
    return {};
}

QString webDriverErrorMessage(const QJsonObject &obj, int status)
{
    const QJsonValue value = obj.value(QStringLiteral("value"));
    if (value.isObject()) {
        const QJsonObject errorObj = value.toObject();
        const QString message = errorObj.value(QStringLiteral("message")).toString();
        if (!message.isEmpty()) {
            return message;
        }
    }
    if (value.isString() && !value.toString().isEmpty()) {
        return value.toString();
    }
    return QStringLiteral("WebDriver HTTP %1").arg(status);
}

QJsonObject buildTypeTextBody(const QString &text)
{
    QJsonArray chars;
    for (const QChar ch : text) {
        chars.append(QString(ch));
    }

    QJsonObject body;
    body.insert(QStringLiteral("text"), text);
    body.insert(QStringLiteral("value"), chars);
    return body;
}

} // namespace

WebDriverClient::WebDriverClient(QString baseUrl)
    : m_baseUrl(normalizedBase(std::move(baseUrl)))
{
}

bool WebDriverClient::isReachable(QString *error) const
{
    QJsonObject response;
    if (!request(QStringLiteral("GET"), QStringLiteral("/status"), {}, &response, error)) {
        return false;
    }

    const bool ready = response.value(QStringLiteral("value")).toObject().value(QStringLiteral("ready")).toBool();
    if (!ready) {
        if (error) {
            *error = QStringLiteral("ChromeDriver is not ready.");
        }
        return false;
    }
    return true;
}

bool WebDriverClient::createSession(const QJsonObject &capabilities,
                                    QString *sessionId,
                                    QString *error)
{
    QJsonObject caps = capabilities;
    if (!caps.contains(QStringLiteral("firstMatch"))) {
        caps.insert(QStringLiteral("firstMatch"), QJsonArray{QJsonObject{}});
    }

    QJsonObject body;
    body.insert(QStringLiteral("capabilities"), caps);

    QJsonObject response;
    if (!request(QStringLiteral("POST"), QStringLiteral("/session"), body, &response, error)) {
        return false;
    }

    const QJsonObject value = response.value(QStringLiteral("value")).toObject();
    QString id = value.value(QStringLiteral("sessionId")).toString();
    if (id.isEmpty()) {
        id = response.value(QStringLiteral("sessionId")).toString();
    }
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
    const QJsonObject body = buildTypeTextBody(text);
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
                error->clear();
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
    QJsonArray args;
    args.append(cssSelector);
    args.append(value);

    QJsonObject response;
    if (!executeScript(sessionId,
                       QStringLiteral("var el=document.querySelector(arguments[0]);"
                                      "if(!el){return false;}"
                                      "el.value=arguments[1];"
                                      "el.dispatchEvent(new Event('change',{bubbles:true}));"
                                      "return true;"),
                       args,
                       &response,
                       error)) {
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
                                    const QJsonArray &args,
                                    QJsonObject *response,
                                    QString *error) const
{
    QJsonObject body;
    body.insert(QStringLiteral("script"), script);
    body.insert(QStringLiteral("args"), args);
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
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json; charset=utf-8"));

    QNetworkReply *reply = nullptr;
    const QByteArray payload = (method == QStringLiteral("POST")) ? requestPayload(body) : QByteArray();

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
    const QByteArray raw = stripBom(reply->readAll());
    const auto networkError = reply->error();
    const QString networkMessage = reply->errorString();
    reply->deleteLater();

    if (networkError != QNetworkReply::NoError && status == 0) {
        if (error) {
            *error = QStringLiteral("WebDriver HTTP error: %1").arg(networkMessage);
        }
        return false;
    }

    if (raw.isEmpty()) {
        if (status >= 200 && status < 300) {
            if (response) {
                *response = {};
            }
            return true;
        }
        if (error) {
            *error = QStringLiteral("Empty WebDriver response (HTTP %1) for %2")
                         .arg(status)
                         .arg(path);
        }
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &parseError);
    if (!doc.isObject()) {
        if (error) {
            *error = QStringLiteral("Invalid WebDriver JSON (%1, HTTP %2): %3.%4")
                         .arg(parseError.errorString())
                         .arg(status)
                         .arg(QString::fromUtf8(raw.left(160)))
                         .arg(parseFailureHint(raw, status));
        }
        return false;
    }

    const QJsonObject obj = doc.object();
    if (status >= 400) {
        if (error) {
            *error = webDriverErrorMessage(obj, status);
        }
        return false;
    }

    const QJsonValue value = obj.value(QStringLiteral("value"));
    if (value.isObject()) {
        const QJsonObject valueObj = value.toObject();
        if (valueObj.contains(QStringLiteral("error"))) {
            if (error) {
                *error = webDriverErrorMessage(obj, status > 0 ? status : 500);
            }
            return false;
        }
    }

    if (response) {
        *response = obj;
    }
    return true;
}

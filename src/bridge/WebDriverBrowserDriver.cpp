#include "bridge/WebDriverBrowserDriver.h"

#include "bridge/CaptchaSolverFactory.h"
#include "core/DiagnosticsStore.h"
#include "core/EmailVerificationService.h"
#include "core/SignupFlowRunner.h"
#include "core/SignupSelectors.h"
#include "net/ProxyManager.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkProxy>

namespace {

QJsonObject buildCapabilities(const QString &proxy, bool headless)
{
    QJsonArray args;
    if (headless) {
        args.append(QStringLiteral("--headless=new"));
    }
    args.append(QStringLiteral("--disable-blink-features=AutomationControlled"));
    args.append(QStringLiteral("--no-sandbox"));
    args.append(QStringLiteral("--window-size=1280,900"));

    if (!proxy.isEmpty()) {
        QNetworkProxy parsed;
        if (ProxyManager::applyToRequest(parsed, proxy)) {
            const QString proxyServer = QStringLiteral("%1://%2:%3")
                                            .arg(parsed.type() == QNetworkProxy::Socks5Proxy
                                                     ? QStringLiteral("socks5")
                                                     : QStringLiteral("http"),
                                                 parsed.hostName())
                                            .arg(parsed.port());
            args.append(QStringLiteral("--proxy-server=") + proxyServer);
        }
    }

    QJsonObject chromeOptions;
    chromeOptions.insert(QStringLiteral("args"), args);

    QJsonObject alwaysMatch;
    alwaysMatch.insert(QStringLiteral("browserName"), QStringLiteral("chrome"));
    alwaysMatch.insert(QStringLiteral("goog:chromeOptions"), chromeOptions);

    QJsonObject capabilities;
    capabilities.insert(QStringLiteral("alwaysMatch"), alwaysMatch);
    return capabilities;
}

} // namespace

WebDriverBrowserDriver::WebDriverBrowserDriver(QString baseUrl)
    : m_client(std::move(baseUrl))
{
}

QString WebDriverBrowserDriver::backendName() const
{
    return QStringLiteral("WebDriver");
}

bool WebDriverBrowserDriver::start(const TaskManager::Settings &settings,
                                   const QString &proxy,
                                   QString *error)
{
    m_settings = settings;
    m_proxy = proxy;

    if (!m_client.isReachable(error)) {
        return false;
    }

    if (!m_sessionId.isEmpty()) {
        m_client.deleteSession(m_sessionId, nullptr);
        m_sessionId.clear();
    }

    return m_client.createSession(buildCapabilities(proxy, settings.headlessBrowser),
                                &m_sessionId,
                                error);
}

void WebDriverBrowserDriver::stop()
{
    if (!m_sessionId.isEmpty()) {
        m_client.deleteSession(m_sessionId, nullptr);
        m_sessionId.clear();
    }
}

bool WebDriverBrowserDriver::openSignupPage(QString *error)
{
    if (m_sessionId.isEmpty()) {
        if (error) {
            *error = QStringLiteral("WebDriver session is not started.");
        }
        return false;
    }

    for (const QString &url : SignupSelectors::signupUrls()) {
        if (!m_client.navigate(m_sessionId, url, error)) {
            continue;
        }

        QString currentUrl;
        if (!m_client.getCurrentUrl(m_sessionId, &currentUrl, error)) {
            continue;
        }

        if (currentUrl.contains(QStringLiteral("live.com"), Qt::CaseInsensitive)
            || currentUrl.contains(QStringLiteral("microsoft.com"), Qt::CaseInsensitive)) {
            return true;
        }
    }

    captureFailure(QStringLiteral("open_signup"));
    if (error) {
        *error = QStringLiteral("Unable to open Microsoft signup page.");
    }
    return false;
}

bool WebDriverBrowserDriver::fillRegistrationForm(const AccountProfile &profile, QString *error)
{
    m_lastProfile = profile;
    if (!SignupFlowRunner::runWizard(m_client, m_sessionId, profile, error)) {
        captureFailure(QStringLiteral("fill_wizard"));
        return false;
    }
    return true;
}

bool WebDriverBrowserDriver::submitRegistration(QString *error)
{
  if (!clickFirstMatching(SignupSelectors::submitButtons(), error)) {
        captureFailure(QStringLiteral("submit"));
        return false;
    }
    return true;
}

bool WebDriverBrowserDriver::handleVerification(const TaskManager::Settings &settings,
                                                QString *error)
{
    if (!settings.apiEnabled) {
        if (error) {
            *error = QString();
        }
        return true;
    }

    const QString endpoint = settings.tempMailEndpoint.trimmed().isEmpty()
        ? settings.apiKey
        : settings.tempMailEndpoint;

    EmailVerificationService verifier;
    QString otpCode;
    if (!verifier.pollInbox(endpoint,
                           settings.apiKey,
                           m_lastProfile.email,
                           m_settings.numericValue,
                           &otpCode,
                           error)) {
        captureFailure(QStringLiteral("verify_email"));
        return false;
    }

    if (!otpCode.isEmpty()) {
        if (!fillFirstMatchingInput(SignupSelectors::otpFields(), otpCode, error)) {
            captureFailure(QStringLiteral("fill_otp"));
            return false;
        }
        m_client.clickNextIfPresent(m_sessionId, SignupSelectors::nextStepButtons(), nullptr);
    }
    return true;
}

bool WebDriverBrowserDriver::handleCaptcha(QString *error)
{
    auto solver = CaptchaSolverFactory::create(m_settings);
    CaptchaContext context;
    context.client = &m_client;
    context.sessionId = m_sessionId;
    context.settings = m_settings;

    if (!solver->solve(context, error)) {
        captureFailure(QStringLiteral("captcha"));
        return false;
    }
    return true;
}

bool WebDriverBrowserDriver::fillFirstMatchingInput(const QStringList &selectors,
                                                    const QString &text,
                                                    QString *error)
{
    for (const QString &selector : selectors) {
        QString elementId;
        if (!m_client.waitForElement(m_sessionId,
                                     QStringLiteral("css selector"),
                                     selector,
                                     10000,
                                     &elementId,
                                     nullptr)) {
            continue;
        }
        if (m_client.typeText(m_sessionId, elementId, text, error)) {
            return true;
        }
        return false;
    }
    if (error) {
        *error = QStringLiteral("No matching input found.");
    }
    return false;
}

bool WebDriverBrowserDriver::clickFirstMatching(const QStringList &selectors, QString *error)
{
    for (const QString &selector : selectors) {
        QString elementId;
        if (!m_client.waitForElement(m_sessionId,
                                     QStringLiteral("css selector"),
                                     selector,
                                     5000,
                                     &elementId,
                                     nullptr)) {
            continue;
        }
        return m_client.clickElement(m_sessionId, elementId, error);
    }
    if (error) {
        *error = QStringLiteral("No clickable button found.");
    }
    return false;
}

void WebDriverBrowserDriver::captureFailure(const QString &tag) const
{
    QString savedPath;
    QString diagError;
    DiagnosticsStore::saveFailureDump(const_cast<WebDriverClient *>(&m_client),
                                      m_sessionId,
                                      tag,
                                      &savedPath,
                                      &diagError);
}

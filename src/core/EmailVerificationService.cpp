#include "core/EmailVerificationService.h"

#include "core/TempMailClient.h"

#include <QThread>
#include <QUrl>

bool EmailVerificationService::pollInbox(const QString &endpoint,
                                         const QString &apiKey,
                                         const QString &email,
                                         int timeoutSec,
                                         QString *otpCode,
                                         QString *error) const
{
    const QUrl url(endpoint);
    if (url.isValid() && !url.scheme().isEmpty()) {
        TempMailClient client;
        const TempMailResult result = client.pollCode(endpoint, apiKey, email, timeoutSec);
        if (!result.success) {
            if (error) {
                *error = result.message;
            }
            return false;
        }
        if (otpCode) {
            *otpCode = result.code;
        }
        if (error) {
            error->clear();
        }
        return true;
    }

    if (apiKey.isEmpty()) {
        if (error) {
            *error = QStringLiteral("API key is empty.");
        }
        return false;
    }

    const int waitMs = qBound(1000, timeoutSec > 0 ? timeoutSec * 1000 : 3000, 15000);
    QThread::msleep(static_cast<unsigned long>(waitMs));
    if (otpCode) {
        otpCode->clear();
    }
    if (error) {
        error->clear();
    }
    return true;
}

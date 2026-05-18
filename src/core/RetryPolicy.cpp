#include "core/RetryPolicy.h"

#include <QThread>

bool RetryPolicy::run(int maxAttempts, const std::function<bool(QString *)> &action, QString *error)
{
    const int attempts = qMax(1, maxAttempts);
    QString lastError;

    for (int i = 0; i < attempts; ++i) {
        QString stepError;
        if (action(&stepError)) {
            if (error) {
                error->clear();
            }
            return true;
        }
        lastError = stepError;
        if (i + 1 < attempts) {
            QThread::msleep(800);
        }
    }

    if (error) {
        *error = lastError;
    }
    return false;
}

#include "core/ResultStore.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

namespace {

QString storePath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/accounts.txt");
}

} // namespace

ResultStore::ResultStore(QObject *parent)
    : QObject(parent)
{
    reload();
}

QString ResultStore::filePath() const
{
    return storePath();
}

QVector<RegistrationResult> ResultStore::recentResults() const
{
    return m_results;
}

bool ResultStore::append(const RegistrationResult &result)
{
    QFile file(storePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    const QString status = result.success ? QStringLiteral("OK") : QStringLiteral("FAIL");
    out << status << '|'
        << result.email << '|'
        << result.password << '|'
        << registrationStageName(result.lastStage) << '|'
        << result.elapsedMs << '|'
        << result.message << '|'
        << QDateTime::currentDateTime().toString(Qt::ISODate) << '\n';

    m_results.append(result);
    emit resultsChanged();
    return true;
}

void ResultStore::reload()
{
    m_results.clear();

    QFile file(storePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit resultsChanged();
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        RegistrationResult parsed;
        if (parseLine(in.readLine(), &parsed)) {
            m_results.append(parsed);
        }
    }

    emit resultsChanged();
}

bool ResultStore::parseLine(const QString &line, RegistrationResult *out) const
{
    if (!out || line.trimmed().isEmpty()) {
        return false;
    }

    const QStringList parts = line.split(QLatin1Char('|'));
    if (parts.size() < 7) {
        return false;
    }

    out->success = parts.at(0) == QStringLiteral("OK");
    out->email = parts.at(1);
    out->password = parts.at(2);
    out->message = parts.at(5);
    out->elapsedMs = parts.at(4).toLongLong();
    return true;
}

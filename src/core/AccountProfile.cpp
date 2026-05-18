#include "core/AccountProfile.h"

#include <QDate>
#include <QRandomGenerator>
#include <QStringList>

namespace {

QString randomAlphaNumeric(int length)
{
    static const QString chars = QStringLiteral("abcdefghijklmnopqrstuvwxyz0123456789");
    QString out;
    out.reserve(length);
    for (int i = 0; i < length; ++i) {
        out.append(chars.at(QRandomGenerator::global()->bounded(chars.size())));
    }
    return out;
}

QString randomFrom(const QStringList &items)
{
    return items.at(QRandomGenerator::global()->bounded(items.size()));
}

QString emailDomain(const TaskManager::Settings &settings)
{
    if (settings.gmailEnabled) {
        return QStringLiteral("gmail.com");
    }
    if (settings.emailEnabled) {
        return QStringLiteral("outlook.com");
    }
    return QRandomGenerator::global()->bounded(2) == 0
        ? QStringLiteral("outlook.com")
        : QStringLiteral("hotmail.com");
}

} // namespace

AccountProfile AccountProfile::generate(int workerIndex, const TaskManager::Settings &settings)
{
    AccountProfile profile;
    profile.username = QStringLiteral("user_%1_%2")
                           .arg(workerIndex + 1)
                           .arg(randomAlphaNumeric(6));
    profile.email = QStringLiteral("%1@%2").arg(profile.username, emailDomain(settings));
    profile.password = QStringLiteral("%1%2!aA")
                           .arg(randomAlphaNumeric(8))
                           .arg(QRandomGenerator::global()->bounded(90, 99));
    profile.firstName = randomFrom({QStringLiteral("Alex"), QStringLiteral("Jordan"),
                                  QStringLiteral("Taylor"), QStringLiteral("Casey")});
    profile.lastName = randomFrom({QStringLiteral("Smith"), QStringLiteral("Lee"),
                                 QStringLiteral("Brown"), QStringLiteral("Kim")});
    profile.birthDate = QDate(1990, 1, 1).addDays(QRandomGenerator::global()->bounded(8000));
    profile.countryCode = QStringLiteral("US");
    return profile;
}

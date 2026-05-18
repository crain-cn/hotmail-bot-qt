#pragma once

#include "core/TaskManager.h"

#include <QDate>
#include <QString>

struct AccountProfile {
    QString username;
    QString email;
    QString password;
    QString firstName;
    QString lastName;
    QDate birthDate;
    QString countryCode;

    static AccountProfile generate(int workerIndex, const TaskManager::Settings &settings);
};

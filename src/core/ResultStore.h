#pragma once

#include "core/RegistrationResult.h"

#include <QObject>
#include <QString>
#include <QVector>

class ResultStore : public QObject
{
    Q_OBJECT

public:
    explicit ResultStore(QObject *parent = nullptr);

    QString filePath() const;
    QVector<RegistrationResult> recentResults() const;
    bool append(const RegistrationResult &result);
    void reload();

signals:
    void resultsChanged();

private:
    bool parseLine(const QString &line, RegistrationResult *out) const;

    QVector<RegistrationResult> m_results;
};

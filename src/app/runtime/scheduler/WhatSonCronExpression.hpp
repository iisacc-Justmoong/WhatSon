#pragma once

#include <QDateTime>
#include <QSet>
#include <QString>

class WhatSonCronExpression final
{
public:
    bool parse(const QString& expression, QString* errorMessage = nullptr);

    bool isValid() const noexcept;
    QString expression() const;
    bool matches(const QDateTime& localDateTime) const;

private:
    struct Field final
    {
        bool wildcard = false;
        QSet<int> allowedValues;

        bool matches(int value) const;
    };

    static bool parseField(
        const QString& fieldName,
        const QString& fieldSpec,
        int minValue,
        int maxValue,
        bool allowSundayAlias,
        Field* outField,
        QString* errorMessage);

    Field m_minutes;
    Field m_hours;
    Field m_daysOfMonth;
    Field m_months;
    Field m_daysOfWeek;

    QString m_expression;
    bool m_valid = false;
};

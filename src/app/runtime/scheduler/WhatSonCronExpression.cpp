#include "WhatSonCronExpression.hpp"

#include <QStringList>
#include <utility>

namespace
{
    bool parseIntInRange(
        const QString& token,
        int minValue,
        int maxValue,
        bool allowSundayAlias,
        int* outValue,
        QString* errorMessage)
    {
        bool ok = false;
        const int value = token.toInt(&ok);
        if (!ok)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Invalid integer token: '%1'.").arg(token);
            }
            return false;
        }

        if (allowSundayAlias && value == 7)
        {
            *outValue = 0;
            return true;
        }

        if (value < minValue || value > maxValue)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("Value out of range: '%1' not in [%2, %3].")
                                .arg(token)
                                .arg(minValue)
                                .arg(maxValue);
            }
            return false;
        }

        *outValue = value;
        return true;
    }
}

bool WhatSonCronExpression::parse(const QString& expression, QString* errorMessage)
{
    const QString normalizedExpression = expression.simplified();
    const QStringList fields =
        normalizedExpression.split(QLatin1Char(' '), Qt::SkipEmptyParts);

    if (fields.size() != 5)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Cron expression must have 5 fields: minute hour day month weekday.");
        }
        m_expression.clear();
        m_valid = false;
        return false;
    }

    Field minutes;
    Field hours;
    Field daysOfMonth;
    Field months;
    Field daysOfWeek;
    QString parseError;

    if (!parseField(
            QStringLiteral("minute"),
            fields.at(0),
            0,
            59,
            false,
            &minutes,
            &parseError)
        || !parseField(
            QStringLiteral("hour"),
            fields.at(1),
            0,
            23,
            false,
            &hours,
            &parseError)
        || !parseField(
            QStringLiteral("dayOfMonth"),
            fields.at(2),
            1,
            31,
            false,
            &daysOfMonth,
            &parseError)
        || !parseField(
            QStringLiteral("month"),
            fields.at(3),
            1,
            12,
            false,
            &months,
            &parseError)
        || !parseField(
            QStringLiteral("dayOfWeek"),
            fields.at(4),
            0,
            6,
            true,
            &daysOfWeek,
            &parseError))
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = parseError;
        }
        m_expression.clear();
        m_valid = false;
        return false;
    }

    m_minutes = std::move(minutes);
    m_hours = std::move(hours);
    m_daysOfMonth = std::move(daysOfMonth);
    m_months = std::move(months);
    m_daysOfWeek = std::move(daysOfWeek);
    m_expression = normalizedExpression;
    m_valid = true;
    return true;
}

bool WhatSonCronExpression::isValid() const noexcept
{
    return m_valid;
}

QString WhatSonCronExpression::expression() const
{
    return m_expression;
}

bool WhatSonCronExpression::matches(const QDateTime& localDateTime) const
{
    if (!m_valid || !localDateTime.isValid())
    {
        return false;
    }

    const QDate date = localDateTime.date();
    const QTime time = localDateTime.time();

    if (!m_minutes.matches(time.minute()) || !m_hours.matches(time.hour()) || !m_months.matches(date.month()))
    {
        return false;
    }

    const int cronDayOfWeek = date.dayOfWeek() % 7;
    const bool dayOfMonthMatches = m_daysOfMonth.matches(date.day());
    const bool dayOfWeekMatches = m_daysOfWeek.matches(cronDayOfWeek);

    if (m_daysOfMonth.wildcard && m_daysOfWeek.wildcard)
    {
        return true;
    }
    if (m_daysOfMonth.wildcard)
    {
        return dayOfWeekMatches;
    }
    if (m_daysOfWeek.wildcard)
    {
        return dayOfMonthMatches;
    }

    return dayOfMonthMatches || dayOfWeekMatches;
}

bool WhatSonCronExpression::Field::matches(int value) const
{
    if (wildcard)
    {
        return true;
    }

    return allowedValues.contains(value);
}

bool WhatSonCronExpression::parseField(
    const QString& fieldName,
    const QString& fieldSpec,
    int minValue,
    int maxValue,
    bool allowSundayAlias,
    Field* outField,
    QString* errorMessage)
{
    if (outField == nullptr)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("Output field is null.");
        }
        return false;
    }

    const QString trimmedSpec = fieldSpec.trimmed();
    if (trimmedSpec.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("%1 field is empty.").arg(fieldName);
        }
        return false;
    }

    const QStringList segments = trimmedSpec.split(QLatin1Char(','), Qt::KeepEmptyParts);
    QSet<int> values;
    const int maxInputValue = allowSundayAlias ? maxValue + 1 : maxValue;

    for (const QString& rawSegment : segments)
    {
        const QString segment = rawSegment.trimmed();
        if (segment.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("%1 field has an empty segment.").arg(fieldName);
            }
            return false;
        }

        const QStringList stepSplit = segment.split(QLatin1Char('/'), Qt::KeepEmptyParts);
        if (stepSplit.size() > 2)
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("%1 field has invalid step segment: '%2'.")
                    .arg(fieldName, segment);
            }
            return false;
        }

        const QString basePart = stepSplit.at(0).trimmed();
        int step = 1;
        if (stepSplit.size() == 2)
        {
            bool stepOk = false;
            step = stepSplit.at(1).trimmed().toInt(&stepOk);
            if (!stepOk || step <= 0)
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = QStringLiteral("%1 field has invalid step in segment '%2'.")
                        .arg(fieldName, segment);
                }
                return false;
            }
        }

        int start = minValue;
        int end = maxInputValue;

        if (basePart != QStringLiteral("*"))
        {
            const QStringList rangeSplit = basePart.split(QLatin1Char('-'), Qt::KeepEmptyParts);
            if (rangeSplit.size() == 2)
            {
                int rangeStart = 0;
                int rangeEnd = 0;
                QString rangeError;
                if (!parseIntInRange(
                        rangeSplit.at(0).trimmed(),
                        minValue,
                        maxInputValue,
                        false,
                        &rangeStart,
                        &rangeError)
                    || !parseIntInRange(
                        rangeSplit.at(1).trimmed(),
                        minValue,
                        maxInputValue,
                        false,
                        &rangeEnd,
                        &rangeError))
                {
                    if (errorMessage != nullptr)
                    {
                        *errorMessage = QStringLiteral("%1 field has invalid range '%2': %3")
                            .arg(fieldName, basePart, rangeError);
                    }
                    return false;
                }

                if (rangeStart > rangeEnd)
                {
                    if (errorMessage != nullptr)
                    {
                        *errorMessage = QStringLiteral("%1 field range start is greater than end: '%2'.")
                            .arg(fieldName, basePart);
                    }
                    return false;
                }

                start = rangeStart;
                end = rangeEnd;
            }
            else if (rangeSplit.size() == 1)
            {
                int singleValue = 0;
                QString valueError;
                if (!parseIntInRange(
                    basePart,
                    minValue,
                    maxInputValue,
                    false,
                    &singleValue,
                    &valueError))
                {
                    if (errorMessage != nullptr)
                    {
                        *errorMessage = QStringLiteral("%1 field has invalid value '%2': %3")
                            .arg(fieldName, basePart, valueError);
                    }
                    return false;
                }
                start = singleValue;
                end = stepSplit.size() == 2 ? maxInputValue : singleValue;
            }
            else
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = QStringLiteral("%1 field has invalid segment '%2'.")
                        .arg(fieldName, segment);
                }
                return false;
            }
        }

        for (int rawValue = start; rawValue <= end; rawValue += step)
        {
            int normalizedValue = rawValue;
            if (allowSundayAlias && rawValue == 7)
            {
                normalizedValue = 0;
            }

            if (normalizedValue < minValue || normalizedValue > maxValue)
            {
                if (errorMessage != nullptr)
                {
                    *errorMessage = QStringLiteral("%1 field value out of range after normalization: %2.")
                                    .arg(fieldName)
                                    .arg(normalizedValue);
                }
                return false;
            }

            values.insert(normalizedValue);
        }
    }

    if (values.isEmpty())
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("%1 field produced no values.").arg(fieldName);
        }
        return false;
    }

    outField->allowedValues = values;
    outField->wildcard = values.size() == (maxValue - minValue + 1);
    return true;
}

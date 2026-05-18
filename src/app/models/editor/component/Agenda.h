#pragma once

#include <QString>
#include <QStringList>

namespace WhatSon::EditorComponent
{
    struct AgendaStaticTag final
    {
        QString canonicalName;
        QString openingToken;
        QString closingToken;

        bool isValid() const noexcept;
    };

    class Agenda final
    {
    public:
        static QStringList staticTagNames();
        static AgendaStaticTag staticTagFor(const QString& tagName);
    };
} // namespace WhatSon::EditorComponent

#pragma once

#include <QVariantMap>
#include <QString>

namespace WhatSon::NoteBodyResourceTagGenerator
{
    QVariantMap normalizeImportedResourceDescriptor(const QVariantMap& resourceEntry);
    QString buildCanonicalResourceTag(const QVariantMap& resourceEntry);
} // namespace WhatSon::NoteBodyResourceTagGenerator

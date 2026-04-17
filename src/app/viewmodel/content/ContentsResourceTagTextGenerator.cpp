#include "ContentsResourceTagTextGenerator.hpp"

#include "file/note/WhatSonNoteBodyResourceTagGenerator.hpp"

#include <utility>

ContentsResourceTagTextGenerator::ContentsResourceTagTextGenerator(QObject* parent)
    : QObject(parent)
{
}

ContentsResourceTagTextGenerator::~ContentsResourceTagTextGenerator() = default;

QString ContentsResourceTagTextGenerator::lastGeneratedTagText() const
{
    return m_lastGeneratedTagText;
}

QVariantMap ContentsResourceTagTextGenerator::lastGeneratedDescriptor() const
{
    return m_lastGeneratedDescriptor;
}

QVariantMap ContentsResourceTagTextGenerator::normalizeImportedResourceEntry(const QVariant& resourceEntry) const
{
    return WhatSon::NoteBodyResourceTagGenerator::normalizeImportedResourceDescriptor(
        resourceEntry.toMap());
}

QString ContentsResourceTagTextGenerator::buildCanonicalResourceTag(const QVariant& resourceEntry)
{
    const QVariantMap normalizedDescriptor = normalizeImportedResourceEntry(resourceEntry);
    const QString tagText = WhatSon::NoteBodyResourceTagGenerator::buildCanonicalResourceTag(
        normalizedDescriptor);
    updateLastGeneratedDescriptor(normalizedDescriptor);
    updateLastGeneratedTagText(tagText);
    return tagText;
}

void ContentsResourceTagTextGenerator::updateLastGeneratedTagText(QString tagText)
{
    if (m_lastGeneratedTagText == tagText)
    {
        return;
    }

    m_lastGeneratedTagText = std::move(tagText);
    emit lastGeneratedTagTextChanged();
}

void ContentsResourceTagTextGenerator::updateLastGeneratedDescriptor(QVariantMap descriptor)
{
    if (m_lastGeneratedDescriptor == descriptor)
    {
        return;
    }

    m_lastGeneratedDescriptor = std::move(descriptor);
    emit lastGeneratedDescriptorChanged();
}

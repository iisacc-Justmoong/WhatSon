#include "ContentsPaperSelection.hpp"

ContentsPaperSelection::ContentsPaperSelection(QObject* parent)
    : QObject(parent)
{
}

ContentsPaperSelection::~ContentsPaperSelection() = default;

QString ContentsPaperSelection::paperStandardForKind(const PaperKind paperKind)
{
    switch (paperKind)
    {
    case A4:
        return QStringLiteral("A4");
    case Letter:
        return QStringLiteral("Letter");
    case Legal:
        return QStringLiteral("Legal");
    case A5:
        return QStringLiteral("A5");
    case B5:
        return QStringLiteral("B5");
    case Unknown:
    default:
        return QStringLiteral("Unknown");
    }
}

ContentsPaperSelection::PaperKind ContentsPaperSelection::normalizePaperKind(const int value) noexcept
{
    switch (value)
    {
    case A4:
        return A4;
    case Letter:
        return Letter;
    case Legal:
        return Legal;
    case A5:
        return A5;
    case B5:
        return B5;
    case Unknown:
    default:
        return Unknown;
    }
}

ContentsPaperSelection::PaperKind ContentsPaperSelection::selectedPaperKind() const noexcept
{
    return m_selectedPaperKind;
}

void ContentsPaperSelection::setSelectedPaperKind(const PaperKind selectedPaperKind)
{
    const PaperKind normalizedPaperKind = normalizePaperKind(selectedPaperKind);
    if (m_selectedPaperKind == normalizedPaperKind)
    {
        return;
    }

    m_selectedPaperKind = normalizedPaperKind;
    emit selectedPaperKindChanged();
}

QString ContentsPaperSelection::selectedPaperStandard() const
{
    return paperStandardForKind(selectedPaperKind());
}

QString ContentsPaperSelection::paperStandardForValue(const int value) const
{
    return paperStandardForKind(normalizePaperKind(value));
}

void ContentsPaperSelection::setSelectedPaperKindByValue(const int value)
{
    setSelectedPaperKind(normalizePaperKind(value));
}

void ContentsPaperSelection::requestRefresh()
{
    emit selectedPaperKindChanged();
}

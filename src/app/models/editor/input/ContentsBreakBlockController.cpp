#include "app/models/editor/input/ContentsBreakBlockController.hpp"

#include <QMetaObject>
#include <QVariantList>

ContentsBreakBlockController::ContentsBreakBlockController(QObject* parent)
    : QObject(parent)
{
}

QObject* ContentsBreakBlockController::breakBlock() const noexcept { return m_breakBlock; }
void ContentsBreakBlockController::setBreakBlock(QObject* value)
{
    if (m_breakBlock == value)
        return;
    m_breakBlock = value;
    emit breakBlockChanged();
}

QObject* ContentsBreakBlockController::divider() const noexcept { return m_divider; }
void ContentsBreakBlockController::setDivider(QObject* value)
{
    if (m_divider == value)
        return;
    m_divider = value;
    emit dividerChanged();
}

void ContentsBreakBlockController::selectBreakBlock() const
{
    if (!m_breakBlock)
        return;
    QMetaObject::invokeMethod(m_breakBlock, "forceActiveFocus");
    QMetaObject::invokeMethod(m_breakBlock, "activated");
}

bool ContentsBreakBlockController::applyFocusRequest(const QVariantMap& request) const
{
    if (!m_breakBlock)
        return false;
    bool ok = false;
    const int sourceOffset = request.value(QStringLiteral("sourceOffset")).toInt(&ok);
    if (!ok)
        return false;
    const int sourceStart = m_breakBlock->property("sourceStart").toInt();
    const int sourceEnd = m_breakBlock->property("sourceEnd").toInt();
    if (sourceOffset < sourceStart || sourceOffset > sourceEnd)
        return false;
    selectBreakBlock();
    return true;
}

QString ContentsBreakBlockController::visiblePlainText() const
{
    return {};
}

int ContentsBreakBlockController::representativeCharCount(const QVariant& lineText) const
{
    const QString text = lineText.isValid() && !lineText.isNull() ? lineText.toString() : QString{};
    return text.isEmpty() ? 8 : text.length();
}

QVariantList ContentsBreakBlockController::logicalLineLayoutEntries() const
{
    QVariantList entries;
    if (!m_breakBlock || !m_divider)
        return entries;

    QVariant mappedPoint;
    if (!QMetaObject::invokeMethod(m_divider, "mapToItem", Q_RETURN_ARG(QVariant, mappedPoint), Q_ARG(QVariant, QVariant::fromValue(m_breakBlock)), Q_ARG(QVariant, 0), Q_ARG(QVariant, 0)))
    {
        QVariantMap fallback;
        fallback.insert(QStringLiteral("x"), 0);
        fallback.insert(QStringLiteral("y"), qMax(0.0, (m_breakBlock->property("height").toDouble() - m_divider->property("height").toDouble()) / 2.0));
        mappedPoint = fallback;
    }
    const QVariantMap point = mappedPoint.toMap();
    QVariantMap entry;
    entry.insert(QStringLiteral("contentAvailableWidth"), qMax(1.0, m_breakBlock->property("width").toDouble()));
    entry.insert(QStringLiteral("contentHeight"), qMax(1.0, m_divider->property("height").toDouble()));
    entry.insert(QStringLiteral("contentWidth"), qMax(0.0, m_divider->property("width").toDouble()));
    entry.insert(QStringLiteral("contentY"), qMax(0.0, point.value(QStringLiteral("y")).toDouble()));
    QVariantList widths;
    widths.append(qMax(0.0, m_divider->property("width").toDouble()));
    entry.insert(QStringLiteral("visualRowWidths"), widths);
    entries.append(entry);
    return entries;
}

QVariantMap ContentsBreakBlockController::currentCursorRowRect() const
{
    const QVariantList entries = logicalLineLayoutEntries();
    if (!entries.isEmpty())
        return entries.first().toMap();
    return QVariantMap{{QStringLiteral("contentHeight"), 1}, {QStringLiteral("contentY"), 0}};
}

bool ContentsBreakBlockController::handleKeyPress(QObject* event) const
{
    Q_UNUSED(event)
    return false;
}

void ContentsBreakBlockController::handleTap(const int tapCount) const
{
    if (!m_breakBlock)
        return;
    if (tapCount >= 2)
    {
        QMetaObject::invokeMethod(m_breakBlock, "documentEndEditRequested");
        return;
    }
    selectBreakBlock();
}

#include "app/viewmodel/editor/display/ContentsActiveEditorSurfaceAdapter.hpp"

#include <QMetaObject>
#include <QVariantMap>
#include <QtGlobal>

namespace
{
    bool objectPropertyBool(QObject* object, const char* propertyName, const bool fallback)
    {
        if (object == nullptr || propertyName == nullptr)
        {
            return fallback;
        }
        const QVariant value = object->property(propertyName);
        return value.isValid() ? value.toBool() : fallback;
    }

    int objectPropertyInt(QObject* object, const char* propertyName, const int fallback)
    {
        if (object == nullptr || propertyName == nullptr)
        {
            return fallback;
        }
        const QVariant value = object->property(propertyName);
        return value.isValid() ? value.toInt() : fallback;
    }

    QObject* objectPropertyObject(QObject* object, const char* propertyName)
    {
        if (object == nullptr || propertyName == nullptr)
        {
            return nullptr;
        }
        return qvariant_cast<QObject*>(object->property(propertyName));
    }

    bool invokeNoArgumentMethod(QObject* object, const char* methodName)
    {
        return object != nullptr
            && methodName != nullptr
            && QMetaObject::invokeMethod(object, methodName);
    }

    bool invokeVariantMethod(QObject* object, const char* methodName, const QVariant& argument)
    {
        return object != nullptr
            && methodName != nullptr
            && QMetaObject::invokeMethod(
                object,
                methodName,
                Q_ARG(QVariant, argument));
    }

    QVariantMap requestMapFromVariant(const QVariant& request)
    {
        if (request.metaType().id() == QMetaType::QVariantMap)
        {
            return request.toMap();
        }
        return {};
    }
} // namespace

ContentsActiveEditorSurfaceAdapter::ContentsActiveEditorSurfaceAdapter(QObject* parent)
    : QObject(parent)
{
}

ContentsActiveEditorSurfaceAdapter::~ContentsActiveEditorSurfaceAdapter() = default;

QObject* ContentsActiveEditorSurfaceAdapter::structuredDocumentFlow() const noexcept
{
    return m_structuredDocumentFlow.data();
}

QObject* ContentsActiveEditorSurfaceAdapter::contentEditor() const noexcept
{
    return m_contentEditor.data();
}

bool ContentsActiveEditorSurfaceAdapter::structuredSurfaceActive() const noexcept
{
    return m_structuredSurfaceActive;
}

bool ContentsActiveEditorSurfaceAdapter::inlineSurfaceActive() const noexcept
{
    return m_inlineSurfaceActive;
}

QString ContentsActiveEditorSurfaceAdapter::activeSurfaceKind() const
{
    if (structuredSurfaceUsable())
    {
        return QStringLiteral("structured");
    }
    if (inlineSurfaceUsable())
    {
        return QStringLiteral("inline");
    }
    return QStringLiteral("none");
}

void ContentsActiveEditorSurfaceAdapter::setStructuredDocumentFlow(QObject* surface)
{
    if (m_structuredDocumentFlow == surface)
    {
        return;
    }

    m_structuredDocumentFlow = surface;
    emit structuredDocumentFlowChanged();
    emit activeSurfaceChanged();
}

void ContentsActiveEditorSurfaceAdapter::setContentEditor(QObject* surface)
{
    if (m_contentEditor == surface)
    {
        return;
    }

    m_contentEditor = surface;
    emit contentEditorChanged();
    emit activeSurfaceChanged();
}

void ContentsActiveEditorSurfaceAdapter::setStructuredSurfaceActive(const bool active)
{
    if (m_structuredSurfaceActive == active)
    {
        return;
    }

    m_structuredSurfaceActive = active;
    emit structuredSurfaceActiveChanged();
    emit activeSurfaceChanged();
}

void ContentsActiveEditorSurfaceAdapter::setInlineSurfaceActive(const bool active)
{
    if (m_inlineSurfaceActive == active)
    {
        return;
    }

    m_inlineSurfaceActive = active;
    emit inlineSurfaceActiveChanged();
    emit activeSurfaceChanged();
}

bool ContentsActiveEditorSurfaceAdapter::requestFocus(const QVariant& request)
{
    if (structuredSurfaceUsable() && requestStructuredFocus(request))
    {
        return true;
    }
    if (inlineSurfaceUsable())
    {
        return requestInlineFocus(request);
    }
    return false;
}

bool ContentsActiveEditorSurfaceAdapter::requestFocusAtSourceOffset(const int sourceOffset)
{
    QVariantMap request;
    request.insert(QStringLiteral("sourceOffset"), qMax(0, sourceOffset));
    request.insert(QStringLiteral("logicalCursorPosition"), qMax(0, sourceOffset));
    return requestFocus(request);
}

bool ContentsActiveEditorSurfaceAdapter::setLogicalCursorPosition(const int cursorPosition)
{
    const int normalizedCursorPosition = qMax(0, cursorPosition);
    if (structuredSurfaceUsable())
    {
        QVariantMap request;
        request.insert(QStringLiteral("sourceOffset"), normalizedCursorPosition);
        request.insert(QStringLiteral("logicalCursorPosition"), normalizedCursorPosition);
        return requestStructuredFocus(request);
    }
    return inlineSurfaceUsable() && setInlineCursorPosition(normalizedCursorPosition);
}

int ContentsActiveEditorSurfaceAdapter::currentLogicalCursorPosition() const
{
    if (inlineSurfaceUsable())
    {
        return qMax(0, objectPropertyInt(m_contentEditor, "cursorPosition", 0));
    }
    return 0;
}

bool ContentsActiveEditorSurfaceAdapter::isInputFocused() const
{
    if (structuredSurfaceUsable())
    {
        return objectPropertyBool(m_structuredDocumentFlow, "focused", false)
            || objectPropertyBool(m_structuredDocumentFlow, "activeFocus", false);
    }
    if (!inlineSurfaceUsable())
    {
        return false;
    }

    QObject* const editorItem = objectPropertyObject(m_contentEditor, "editorItem");
    QObject* const inputItem = objectPropertyObject(editorItem, "inputItem");
    return objectPropertyBool(m_contentEditor, "focused", false)
        || objectPropertyBool(m_contentEditor, "activeFocus", false)
        || objectPropertyBool(editorItem, "focused", false)
        || objectPropertyBool(editorItem, "activeFocus", false)
        || objectPropertyBool(inputItem, "focused", false)
        || objectPropertyBool(inputItem, "activeFocus", false);
}

bool ContentsActiveEditorSurfaceAdapter::supportsNativeInput() const
{
    return structuredSurfaceUsable() || inlineSurfaceUsable();
}

bool ContentsActiveEditorSurfaceAdapter::structuredSurfaceUsable() const
{
    return m_structuredSurfaceActive
        && m_structuredDocumentFlow != nullptr
        && objectPropertyBool(m_structuredDocumentFlow, "visible", true);
}

bool ContentsActiveEditorSurfaceAdapter::inlineSurfaceUsable() const
{
    return m_inlineSurfaceActive
        && m_contentEditor != nullptr
        && objectPropertyBool(m_contentEditor, "visible", true);
}

bool ContentsActiveEditorSurfaceAdapter::requestStructuredFocus(const QVariant& request)
{
    const bool focusRequested = invokeVariantMethod(m_structuredDocumentFlow, "requestFocus", request);
    const bool activeFocusRequested = invokeNoArgumentMethod(m_structuredDocumentFlow, "forceActiveFocus");
    return focusRequested || activeFocusRequested;
}

bool ContentsActiveEditorSurfaceAdapter::requestInlineFocus(const QVariant& request)
{
    const bool contentFocusRequested = invokeNoArgumentMethod(m_contentEditor, "forceActiveFocus");
    QObject* const editorItem = objectPropertyObject(m_contentEditor, "editorItem");
    const bool editorItemFocusRequested = invokeNoArgumentMethod(editorItem, "forceActiveFocus");

    const int cursorPosition = cursorPositionFromRequest(request);
    const bool cursorPositionApplied = cursorPosition >= 0 && setInlineCursorPosition(cursorPosition);
    return contentFocusRequested || editorItemFocusRequested || cursorPositionApplied;
}

bool ContentsActiveEditorSurfaceAdapter::setInlineCursorPosition(const int cursorPosition)
{
    const int normalizedCursorPosition = qMax(0, cursorPosition);
    if (invokeVariantMethod(
            m_contentEditor,
            "setCursorPositionPreservingNativeInput",
            QVariant(normalizedCursorPosition)))
    {
        return true;
    }
    return m_contentEditor != nullptr && m_contentEditor->setProperty("cursorPosition", normalizedCursorPosition);
}

int ContentsActiveEditorSurfaceAdapter::cursorPositionFromRequest(const QVariant& request) const
{
    const QVariantMap requestMap = requestMapFromVariant(request);
    for (const QString& key : {
             QStringLiteral("logicalCursorPosition"),
             QStringLiteral("cursorPosition"),
             QStringLiteral("sourceOffset")})
    {
        const QVariant value = requestMap.value(key);
        if (!value.isValid())
        {
            continue;
        }
        bool ok = false;
        const int cursorPosition = value.toInt(&ok);
        if (ok)
        {
            return qMax(0, cursorPosition);
        }
    }
    return -1;
}

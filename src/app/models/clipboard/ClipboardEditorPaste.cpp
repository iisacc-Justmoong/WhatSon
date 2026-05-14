#include "app/models/clipboard/ClipboardEditorPaste.h"

#include "app/models/clipboard/InAppClipboardManager.h"
#include "app/models/editor/NoteEditorDocumentSession.hpp"

#include <QEvent>
#include <QKeyEvent>
#include <QMetaObject>
#include <QVariantList>

namespace
{
    QVariantMap editorPasteResult(
        const bool valid,
        const bool nativePaste,
        const QString& stage,
        const QString& errorMessage = QString())
    {
        QVariantMap result;
        result.insert(QStringLiteral("valid"), valid);
        result.insert(QStringLiteral("changed"), false);
        result.insert(QStringLiteral("nativePaste"), nativePaste);
        result.insert(QStringLiteral("reloadSucceeded"), true);
        result.insert(QStringLiteral("stage"), stage.trimmed());
        result.insert(QStringLiteral("errorMessage"), errorMessage.trimmed());
        return result;
    }

    bool clipboardResourceIsImage(const InAppClipboardManager& clipboard)
    {
        return clipboard.resourceType().trimmed().compare(QStringLiteral("image"), Qt::CaseInsensitive) == 0;
    }

    QVariantMap nativePasteFallbackResult(const QString& stage, const QString& errorMessage)
    {
        return editorPasteResult(false, true, stage, errorMessage);
    }

    QVariantMap handledFailureResult(const QString& stage, const QString& errorMessage)
    {
        return editorPasteResult(false, false, stage, errorMessage);
    }
} // namespace

ClipboardEditorPaste::ClipboardEditorPaste(QObject* parent)
    : QObject(parent)
{
}

ClipboardEditorPaste::~ClipboardEditorPaste()
{
    clearEditorPasteOwner();
}

QVariantMap ClipboardEditorPaste::pasteImageResourceIntoEditor(
    QObject* clipboardObject,
    QObject* noteEditorSessionObject,
    const QString& editorDocumentText,
    const int cursorPosition,
    const int selectionLength)
{
    auto* clipboard = qobject_cast<InAppClipboardManager*>(clipboardObject);
    if (clipboard == nullptr)
    {
        const QString errorMessage = QStringLiteral("Clipboard editor paste requires InAppClipboardManager.");
        emit pasteFailed(errorMessage);
        return nativePasteFallbackResult(QStringLiteral("clipboard-manager"), errorMessage);
    }

    auto* noteEditorSession = qobject_cast<NoteEditorDocumentSession*>(noteEditorSessionObject);
    if (noteEditorSession == nullptr)
    {
        const QString errorMessage = QStringLiteral("Clipboard editor paste requires NoteEditorDocumentSession.");
        emit pasteFailed(errorMessage);
        return handledFailureResult(QStringLiteral("note-session"), errorMessage);
    }

    if (!clipboard->refreshClipboardResourceAvailabilitySnapshot())
    {
        const QString errorMessage = QStringLiteral("Clipboard does not contain an importable resource.");
        return nativePasteFallbackResult(QStringLiteral("capture"), errorMessage);
    }

    if (!clipboardResourceIsImage(*clipboard))
    {
        const QString errorMessage =
            QStringLiteral("Only image clipboard resources can be pasted into the editor for now.");
        return nativePasteFallbackResult(QStringLiteral("unsupported-resource"), errorMessage);
    }

    const QVariantList importedEntries = clipboard->importClipboardResourceForEditor();
    if (importedEntries.isEmpty())
    {
        const QString errorMessage = clipboard->lastError().trimmed().isEmpty()
            ? QStringLiteral("Failed to import the clipboard image resource.")
            : clipboard->lastError().trimmed();
        emit pasteFailed(errorMessage);
        return handledFailureResult(QStringLiteral("import"), errorMessage);
    }

    QVariantMap insertion = noteEditorSession->insertImportedResourcesIntoSource(
        editorDocumentText,
        cursorPosition,
        selectionLength,
        importedEntries);
    if (!insertion.value(QStringLiteral("valid")).toBool())
    {
        const QString sessionError = noteEditorSession->lastError().trimmed();
        const QString errorMessage = sessionError.isEmpty()
            ? QStringLiteral("Failed to insert the imported resource into the editor source.")
            : sessionError;
        emit pasteFailed(errorMessage);
        return handledFailureResult(QStringLiteral("source-insertion"), errorMessage);
    }

    insertion.insert(QStringLiteral("nativePaste"), false);
    insertion.insert(QStringLiteral("importedEntries"), importedEntries);

    const bool reloadSucceeded = clipboard->reloadImportedResources();
    insertion.insert(QStringLiteral("reloadSucceeded"), reloadSucceeded);
    insertion.insert(
        QStringLiteral("stage"),
        reloadSucceeded ? QStringLiteral("completed") : QStringLiteral("reload"));
    if (!reloadSucceeded)
    {
        insertion.insert(QStringLiteral("errorMessage"), clipboard->lastError().trimmed());
    }
    else if (!insertion.contains(QStringLiteral("errorMessage")))
    {
        insertion.insert(QStringLiteral("errorMessage"), QString());
    }

    emit pasteCompleted(insertion);
    return insertion;
}

bool ClipboardEditorPaste::attachEditorPasteOwner(
    QObject* editorItem,
    QObject* editorOwner,
    QObject* clipboardObject,
    QObject* noteEditorSessionObject)
{
    if (editorItem == nullptr
        || editorOwner == nullptr
        || clipboardObject == nullptr
        || noteEditorSessionObject == nullptr)
    {
        return false;
    }

    if (m_editorItem != editorItem)
    {
        clearEditorPasteOwner();
        m_editorItem = editorItem;
        m_editorItem->installEventFilter(this);
        m_editorItemDestroyedConnection = connect(
            m_editorItem,
            &QObject::destroyed,
            this,
            [this]() {
                clearDestroyedEditorPasteOwner();
            });
    }

    m_editorOwner = editorOwner;
    m_clipboardObject = clipboardObject;
    m_noteEditorSessionObject = noteEditorSessionObject;
    return true;
}

void ClipboardEditorPaste::detachEditorPasteOwner(QObject* editorItem)
{
    if (editorItem != nullptr && m_editorItem != editorItem)
    {
        return;
    }
    clearEditorPasteOwner();
}

bool ClipboardEditorPaste::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_editorItem && event != nullptr && event->type() == QEvent::KeyPress)
    {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (handleEditorPasteKeyEvent(*keyEvent))
        {
            event->accept();
            return true;
        }
    }

    return QObject::eventFilter(watched, event);
}

void ClipboardEditorPaste::clearEditorPasteOwner()
{
    if (m_editorItem)
    {
        m_editorItem->removeEventFilter(this);
    }
    if (m_editorItemDestroyedConnection)
    {
        disconnect(m_editorItemDestroyedConnection);
        m_editorItemDestroyedConnection = QMetaObject::Connection();
    }
    m_editorItem.clear();
    m_editorOwner.clear();
    m_clipboardObject.clear();
    m_noteEditorSessionObject.clear();
}

void ClipboardEditorPaste::clearDestroyedEditorPasteOwner()
{
    if (m_editorItemDestroyedConnection)
    {
        disconnect(m_editorItemDestroyedConnection);
        m_editorItemDestroyedConnection = QMetaObject::Connection();
    }
    m_editorItem.clear();
    m_editorOwner.clear();
    m_clipboardObject.clear();
    m_noteEditorSessionObject.clear();
}

bool ClipboardEditorPaste::editorPasteKeyMatches(const QKeyEvent& event) const
{
    if (event.isAutoRepeat() || event.key() != Qt::Key_V)
    {
        return false;
    }

    const Qt::KeyboardModifiers modifiers = event.modifiers();
    const bool commandModifier =
        modifiers.testFlag(Qt::MetaModifier) || modifiers.testFlag(Qt::ControlModifier);
    const bool disallowedModifiers =
        modifiers.testFlag(Qt::ShiftModifier) || modifiers.testFlag(Qt::AltModifier);
    return commandModifier && !disallowedModifiers;
}

bool ClipboardEditorPaste::handleEditorPasteKeyEvent(QKeyEvent& event)
{
    if (!editorPasteKeyMatches(event)
        || m_editorOwner.isNull()
        || m_clipboardObject.isNull()
        || m_noteEditorSessionObject.isNull())
    {
        return false;
    }

    const QString editorDocumentText =
        m_editorOwner->property("editorDocumentText").toString();
    const int selectionStart =
        m_editorOwner->property("editorSelectionStart").toInt();
    const int selectionLength =
        m_editorOwner->property("editorSelectionLength").toInt();

    const QVariantMap insertion = pasteImageResourceIntoEditor(
        m_clipboardObject.data(),
        m_noteEditorSessionObject.data(),
        editorDocumentText,
        selectionStart,
        selectionLength);
    if (insertion.value(QStringLiteral("nativePaste")).toBool())
    {
        return false;
    }
    if (!insertion.value(QStringLiteral("valid")).toBool())
    {
        return true;
    }

    if (!applyEditorPasteResultToOwner(insertion))
    {
        return true;
    }

    event.accept();
    return true;
}

bool ClipboardEditorPaste::applyEditorPasteResultToOwner(const QVariantMap& insertion)
{
    if (m_editorOwner.isNull())
    {
        return false;
    }

    const QString editorDocumentText =
        insertion.value(QStringLiteral("editorDocumentText")).isValid()
            ? insertion.value(QStringLiteral("editorDocumentText")).toString()
            : insertion.value(QStringLiteral("bodySourceText")).toString();
    QVariant replaceResult;
    const bool invoked = QMetaObject::invokeMethod(
        m_editorOwner.data(),
        "replaceEditorDocumentText",
        Q_RETURN_ARG(QVariant, replaceResult),
        Q_ARG(QVariant, QVariant(editorDocumentText)),
        Q_ARG(QVariant, QVariant(insertion.value(QStringLiteral("cursorPosition")).toInt())));
    return invoked && replaceResult.toBool();
}

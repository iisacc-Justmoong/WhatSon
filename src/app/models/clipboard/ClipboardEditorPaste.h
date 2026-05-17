#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantMap>

class QEvent;
class QKeyEvent;

class ClipboardEditorPaste final : public QObject
{
    Q_OBJECT

public:
    explicit ClipboardEditorPaste(QObject* parent = nullptr);
    ~ClipboardEditorPaste() override;

    Q_INVOKABLE QVariantMap pasteImageResourceIntoEditor(
        QObject* clipboardObject,
        QObject* noteEditorSessionObject,
        const QString& editorDocumentText,
        int cursorPosition,
        int selectionLength);
    Q_INVOKABLE bool attachEditorPasteOwner(
        QObject* editorItem,
        QObject* editorOwner,
        QObject* clipboardObject,
        QObject* noteEditorSessionObject);
    Q_INVOKABLE void detachEditorPasteOwner(QObject* editorItem);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

public slots:
    void requestControllerHook()
    {
        emit controllerHookRequested();
    }

signals:
    void pasteCompleted(const QVariantMap& result);
    void pasteFailed(const QString& message);
    void controllerHookRequested();

private:
    void clearEditorPasteOwner();
    void clearDestroyedEditorPasteOwner();
    bool editorPasteKeyMatches(const QKeyEvent& event) const;
    bool editorCalloutBoundaryKeyMatches(const QKeyEvent& event) const;
    bool handleEditorPasteKeyEvent(QKeyEvent& event);
    bool handleEditorCalloutBoundaryKeyEvent(QKeyEvent& event);
    bool applyEditorPasteResultToOwner(const QVariantMap& insertion);

    QPointer<QObject> m_editorItem;
    QPointer<QObject> m_editorOwner;
    QPointer<QObject> m_clipboardObject;
    QPointer<QObject> m_noteEditorSessionObject;
    QMetaObject::Connection m_editorItemDestroyedConnection;
};

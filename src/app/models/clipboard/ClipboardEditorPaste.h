#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

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

public slots:
    void requestControllerHook()
    {
        emit controllerHookRequested();
    }

signals:
    void pasteCompleted(const QVariantMap& result);
    void pasteFailed(const QString& message);
    void controllerHookRequested();
};

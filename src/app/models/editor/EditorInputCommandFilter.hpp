#pragma once

#include <QObject>
#include <QPointer>
#include <QVariantMap>

class QEvent;
class QKeyEvent;

class EditorInputCommandFilter final : public QObject
{
    Q_OBJECT

public:
    explicit EditorInputCommandFilter(QObject* parent = nullptr);
    ~EditorInputCommandFilter() override;

    Q_INVOKABLE bool attachEditorInputOwner(
        QObject* editorItem,
        QObject* editorOwner,
        QObject* clipboardEditorPasteObject,
        QObject* clipboardObject,
        QObject* noteEditorSessionObject);
    Q_INVOKABLE void detachEditorInputOwner(QObject* editorItem);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void clearEditorInputOwner();
    void clearDestroyedEditorInputOwner();
    bool editorPasteKeyMatches(const QKeyEvent& event) const;
    bool editorCalloutBoundaryKeyMatches(const QKeyEvent& event) const;
    bool handleEditorPasteKeyEvent(QKeyEvent& event);
    bool handleEditorCalloutBoundaryKeyEvent(QKeyEvent& event);
    bool applyEditorCommandResultToOwner(const QVariantMap& result);

    QPointer<QObject> m_editorItem;
    QPointer<QObject> m_editorOwner;
    QPointer<QObject> m_clipboardEditorPasteObject;
    QPointer<QObject> m_clipboardObject;
    QPointer<QObject> m_noteEditorSessionObject;
    QMetaObject::Connection m_editorItemDestroyedConnection;
};

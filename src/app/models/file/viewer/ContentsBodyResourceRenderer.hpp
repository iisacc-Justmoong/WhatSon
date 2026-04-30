#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantList>

class ContentsBodyResourceRenderer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* contentController READ contentController WRITE setContentController NOTIFY contentControllerChanged)
    Q_PROPERTY(QObject* fallbackContentController
                   READ fallbackContentController
                   WRITE setFallbackContentController
                   NOTIFY fallbackContentControllerChanged)
    Q_PROPERTY(QString noteId READ noteId WRITE setNoteId NOTIFY noteIdChanged)
    Q_PROPERTY(QString noteDirectoryPath READ noteDirectoryPath WRITE setNoteDirectoryPath NOTIFY noteDirectoryPathChanged)
    Q_PROPERTY(QVariantList documentBlocks READ documentBlocks WRITE setDocumentBlocks NOTIFY documentBlocksChanged)
    Q_PROPERTY(int maxRenderCount READ maxRenderCount WRITE setMaxRenderCount NOTIFY maxRenderCountChanged)
    Q_PROPERTY(QVariantList renderedResources READ renderedResources NOTIFY renderedResourcesChanged)
    Q_PROPERTY(int resourceCount READ resourceCount NOTIFY renderedResourcesChanged)
    Q_PROPERTY(bool hasRenderableResource READ hasRenderableResource NOTIFY renderedResourcesChanged)

public:
    explicit ContentsBodyResourceRenderer(QObject* parent = nullptr);
    ~ContentsBodyResourceRenderer() override;

    QObject* contentController() const noexcept;
    void setContentController(QObject* model);
    QObject* fallbackContentController() const noexcept;
    void setFallbackContentController(QObject* model);

    QString noteId() const;
    void setNoteId(const QString& noteId);

    QString noteDirectoryPath() const;
    void setNoteDirectoryPath(const QString& noteDirectoryPath);

    QVariantList documentBlocks() const;
    void setDocumentBlocks(const QVariantList& documentBlocks);

    int maxRenderCount() const noexcept;
    void setMaxRenderCount(int maxRenderCount);

    QVariantList renderedResources() const;
    int resourceCount() const noexcept;
    bool hasRenderableResource() const noexcept;

    Q_INVOKABLE void requestRenderRefresh();

signals:
    void contentControllerChanged();
    void fallbackContentControllerChanged();
    void noteIdChanged();
    void noteDirectoryPathChanged();
    void documentBlocksChanged();
    void maxRenderCountChanged();
    void renderedResourcesChanged();

private slots:
    void handleContentControllerDestroyed();
    void handleFallbackContentControllerDestroyed();
    void handleContentFilesystemMutated();

private:
    static bool hasInvokableMethod(const QObject* object, const char* methodSignature);

    void refreshRenderedResources();
    QVariantList buildRenderedResources(const QString& noteDirectoryPath, const QVariantList& documentBlocks) const;
    QString resolveNoteDirectoryPathFromController(QObject* controller) const;
    QString resolveNoteDirectoryPath() const;
    void disconnectContentController();
    void disconnectFallbackContentController();

    QPointer<QObject> m_contentController;
    QPointer<QObject> m_fallbackContentController;
    QString m_noteId;
    QString m_noteDirectoryPath;
    QVariantList m_documentBlocks;
    int m_maxRenderCount = 3;
    QVariantList m_renderedResources;
    QMetaObject::Connection m_contentControllerDestroyedConnection;
    QMetaObject::Connection m_hubFilesystemMutatedConnection;
    QMetaObject::Connection m_fallbackContentControllerDestroyedConnection;
    QMetaObject::Connection m_fallbackHubFilesystemMutatedConnection;
};

#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantList>

class ContentsBodyResourceRenderer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* contentViewModel READ contentViewModel WRITE setContentViewModel NOTIFY contentViewModelChanged)
    Q_PROPERTY(QString noteId READ noteId WRITE setNoteId NOTIFY noteIdChanged)
    Q_PROPERTY(int maxRenderCount READ maxRenderCount WRITE setMaxRenderCount NOTIFY maxRenderCountChanged)
    Q_PROPERTY(QVariantList renderedResources READ renderedResources NOTIFY renderedResourcesChanged)
    Q_PROPERTY(int resourceCount READ resourceCount NOTIFY renderedResourcesChanged)
    Q_PROPERTY(bool hasRenderableResource READ hasRenderableResource NOTIFY renderedResourcesChanged)

public:
    explicit ContentsBodyResourceRenderer(QObject* parent = nullptr);
    ~ContentsBodyResourceRenderer() override;

    QObject* contentViewModel() const noexcept;
    void setContentViewModel(QObject* model);

    QString noteId() const;
    void setNoteId(const QString& noteId);

    int maxRenderCount() const noexcept;
    void setMaxRenderCount(int maxRenderCount);

    QVariantList renderedResources() const;
    int resourceCount() const noexcept;
    bool hasRenderableResource() const noexcept;

    Q_INVOKABLE void requestRenderRefresh();

signals:
    void contentViewModelChanged();
    void noteIdChanged();
    void maxRenderCountChanged();
    void renderedResourcesChanged();

private slots:
    void handleContentViewModelDestroyed();
    void handleContentFilesystemMutated();

private:
    static bool hasInvokableMethod(const QObject* object, const char* methodSignature);

    void refreshRenderedResources();
    QVariantList buildRenderedResources(const QString& noteDirectoryPath) const;
    QString resolveNoteDirectoryPath() const;
    void disconnectContentViewModel();

    QPointer<QObject> m_contentViewModel;
    QString m_noteId;
    int m_maxRenderCount = 3;
    QVariantList m_renderedResources;
    QMetaObject::Connection m_contentViewModelDestroyedConnection;
    QMetaObject::Connection m_hubFilesystemMutatedConnection;
};

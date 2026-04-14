#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>

class ResourceBitmapViewer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant resourceEntry READ resourceEntry WRITE setResourceEntry NOTIFY resourceEntryChanged)
    Q_PROPERTY(QString openTarget READ openTarget NOTIFY viewerStateChanged)
    Q_PROPERTY(QString viewerSource READ viewerSource NOTIFY viewerStateChanged)
    Q_PROPERTY(QString normalizedFormat READ normalizedFormat NOTIFY viewerStateChanged)
    Q_PROPERTY(bool bitmapPreviewCandidate READ bitmapPreviewCandidate NOTIFY viewerStateChanged)
    Q_PROPERTY(bool bitmapFormatCompatible READ bitmapFormatCompatible NOTIFY viewerStateChanged)
    Q_PROPERTY(bool bitmapRenderable READ bitmapRenderable NOTIFY viewerStateChanged)
    Q_PROPERTY(QString incompatibilityReason READ incompatibilityReason NOTIFY viewerStateChanged)

public:
    explicit ResourceBitmapViewer(QObject* parent = nullptr);
    ~ResourceBitmapViewer() override;

    QVariant resourceEntry() const;
    void setResourceEntry(const QVariant& resourceEntry);

    QString openTarget() const;
    QString viewerSource() const;
    QString normalizedFormat() const;
    bool bitmapPreviewCandidate() const noexcept;
    bool bitmapFormatCompatible() const noexcept;
    bool bitmapRenderable() const noexcept;
    QString incompatibilityReason() const;

    Q_INVOKABLE void requestRefresh();

public slots:
    void requestViewerHook()
    {
        emit viewerHookRequested();
    }

signals:
    void resourceEntryChanged();
    void viewerStateChanged();
    void viewerHookRequested();

private:
    static QString toViewerTarget(const QString& pathOrUrl);
    static QString stringValue(const QVariantMap& map, const QString& key);
    void refreshState();

    QVariant m_resourceEntry;
    QString m_openTarget;
    QString m_viewerSource;
    QString m_normalizedFormat;
    bool m_bitmapPreviewCandidate = false;
    bool m_bitmapFormatCompatible = false;
    bool m_bitmapRenderable = false;
    QString m_incompatibilityReason;
};

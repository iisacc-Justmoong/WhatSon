#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

class UnusedResourcesSensor final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString hubPath READ hubPath WRITE setHubPath NOTIFY hubPathChanged)
    Q_PROPERTY(QVariantList unusedResources READ unusedResources NOTIFY unusedResourcesChanged)
    Q_PROPERTY(QStringList unusedResourcePaths READ unusedResourcePaths NOTIFY unusedResourcesChanged)
    Q_PROPERTY(int unusedResourceCount READ unusedResourceCount NOTIFY unusedResourcesChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit UnusedResourcesSensor(QObject* parent = nullptr);
    ~UnusedResourcesSensor() override;

    QString hubPath() const;
    void setHubPath(QString hubPath);

    QVariantList unusedResources() const;
    QStringList unusedResourcePaths() const;
    int unusedResourceCount() const noexcept;
    QString lastError() const;

    Q_INVOKABLE QVariantList scanUnusedResources(const QString& hubPath = QString());
    Q_INVOKABLE QStringList collectUnusedResourcePaths(const QString& hubPath = QString());

public slots:
    void refresh();

signals:
    void hubPathChanged();
    void unusedResourcesChanged();
    void lastErrorChanged();
    void scanCompleted(const QVariantList& unusedResources);

private:
    void setLastError(QString errorMessage);
    void setUnusedResources(QVariantList unusedResources);

    QString m_hubPath;
    QVariantList m_unusedResources;
    QString m_lastError;
};

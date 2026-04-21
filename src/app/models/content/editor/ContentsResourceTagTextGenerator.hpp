#pragma once

#include <QObject>
#include <QVariant>
#include <QVariantMap>
#include <QString>

class ContentsResourceTagTextGenerator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString lastGeneratedTagText READ lastGeneratedTagText NOTIFY lastGeneratedTagTextChanged)
    Q_PROPERTY(QVariantMap lastGeneratedDescriptor READ lastGeneratedDescriptor NOTIFY lastGeneratedDescriptorChanged)

public:
    explicit ContentsResourceTagTextGenerator(QObject* parent = nullptr);
    ~ContentsResourceTagTextGenerator() override;

    QString lastGeneratedTagText() const;
    QVariantMap lastGeneratedDescriptor() const;

public slots:
    QVariantMap normalizeImportedResourceEntry(const QVariant& resourceEntry) const;
    QString buildCanonicalResourceTag(const QVariant& resourceEntry);

signals:
    void lastGeneratedTagTextChanged();
    void lastGeneratedDescriptorChanged();

private:
    void updateLastGeneratedTagText(QString tagText);
    void updateLastGeneratedDescriptor(QVariantMap descriptor);

    QString m_lastGeneratedTagText;
    QVariantMap m_lastGeneratedDescriptor;
};

#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <qqmlregistration.h>

class ContentsEditorTagMutationBuilder : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ContentsEditorTagMutationBuilder)

public:
    explicit ContentsEditorTagMutationBuilder(QObject* parent = nullptr);

    Q_INVOKABLE QVariantMap buildTagInsertionPayload(
        const QVariant& sourceText,
        int selectionStart,
        int selectionEnd,
        const QVariant& tagName);
    Q_INVOKABLE QString normalizedTagName(const QVariant& tagName) const;
    Q_INVOKABLE QVariantMap buildWrappedTagInsertionPayload(
        const QVariant& sourceText,
        int selectionStart,
        int selectionEnd,
        const QVariant& tagName);

signals:
    void tagInsertionPayloadBuilt(const QVariantMap& payload);
};

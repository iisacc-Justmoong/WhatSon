#pragma once

#include <QObject>
#include <QString>

class ContentsPlainTextSourceMutator : public QObject
{
    Q_OBJECT

public:
    explicit ContentsPlainTextSourceMutator(QObject* parent = nullptr);
    ~ContentsPlainTextSourceMutator() override;

    Q_INVOKABLE QString applyPlainTextReplacementToSource(
        const QString& sourceText,
        int sourceStart,
        int sourceEnd,
        const QString& replacementText) const;
};

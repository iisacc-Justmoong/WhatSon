#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class ContentsLogicalTextBridge : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QVariantList logicalLineStartOffsets READ logicalLineStartOffsets NOTIFY logicalLineStartOffsetsChanged)
    Q_PROPERTY(int logicalLineCount READ logicalLineCount NOTIFY logicalLineCountChanged)

public:
    explicit ContentsLogicalTextBridge(QObject* parent = nullptr);
    ~ContentsLogicalTextBridge() override;

    QString text() const;
    void setText(const QString& text);

    QVariantList logicalLineStartOffsets() const;
    int logicalLineCount() const noexcept;

    Q_INVOKABLE int logicalLineNumberForOffset(int offset) const noexcept;
    Q_INVOKABLE int logicalLineStartOffsetAt(int index) const noexcept;
    Q_INVOKABLE int logicalLineCharacterCountAt(int index) const noexcept;

    signals  :



    void textChanged();
    void logicalLineStartOffsetsChanged();
    void logicalLineCountChanged();

private:
    static QVariantList buildLogicalLineOffsets(const QString& text);

    void refreshTextState();

    QString m_text;
    QVariantList m_logicalLineStartOffsets;
    int m_logicalLineCount = 1;
};

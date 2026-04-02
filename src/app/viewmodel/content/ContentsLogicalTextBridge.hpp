#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVector>

class ContentsLogicalTextBridge : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString logicalText READ logicalText NOTIFY logicalTextChanged)
    Q_PROPERTY(QVariantList logicalLineStartOffsets READ logicalLineStartOffsets NOTIFY logicalLineStartOffsetsChanged)
    Q_PROPERTY(int logicalLineCount READ logicalLineCount NOTIFY logicalLineCountChanged)

public:
    explicit ContentsLogicalTextBridge(QObject* parent = nullptr);
    ~ContentsLogicalTextBridge() override;

    QString text() const;
    void setText(const QString& text);
    QString logicalText() const;

    QVariantList logicalLineStartOffsets() const;
    int logicalLineCount() const noexcept;

    Q_INVOKABLE int logicalLineNumberForOffset(int offset) const noexcept;
    Q_INVOKABLE int logicalLineStartOffsetAt(int index) const noexcept;
    Q_INVOKABLE int logicalLineCharacterCountAt(int index) const noexcept;
    Q_INVOKABLE int sourceOffsetForLogicalOffset(int logicalOffset) const noexcept;

    signals  :



    void textChanged();
    void logicalTextChanged();
    void logicalLineStartOffsetsChanged();
    void logicalLineCountChanged();

private:
    static QString normalizeLogicalText(const QString& text);
    static QVariantList buildLogicalLineOffsets(const QString& text);
    static QVector<int> buildLogicalToSourceOffsets(const QString& text, int logicalTextLength);

    void refreshTextState();

    QString m_text;
    QString m_logicalText;
    QVariantList m_logicalLineStartOffsets;
    QVector<int> m_logicalToSourceOffsets;
    int m_logicalLineCount = 1;
};

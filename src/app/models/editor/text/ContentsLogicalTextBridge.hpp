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
    Q_PROPERTY(int logicalLineCount READ logicalLineCount NOTIFY logicalLineCountChanged)

public:
    explicit ContentsLogicalTextBridge(QObject* parent = nullptr);
    ~ContentsLogicalTextBridge() override;

    QString text() const;
    void setText(const QString& text);
    QString logicalText() const;

    int logicalLineCount() const noexcept;

    Q_INVOKABLE int logicalLengthForSourceText(const QString& text) const;
    QVariantList logicalToSourceOffsets() const;
    Q_INVOKABLE int logicalOffsetForSourceOffset(int sourceOffset) const;
    Q_INVOKABLE int logicalOffsetForSourceOffsetWithAffinity(int sourceOffset, bool preferAfter) const noexcept;
    Q_INVOKABLE int sourceOffsetForLogicalOffset(int logicalOffset) const noexcept;
    Q_INVOKABLE int sourceOffsetForVisibleLogicalOffset(int logicalOffset, int visibleLength) const noexcept;

    signals  :



    void textChanged();
    void logicalTextChanged();
    void logicalLineCountChanged();
    void logicalToSourceOffsetsChanged();

private:
    static QString normalizeLogicalText(const QString& text);
    static int countLogicalLines(const QString& text) noexcept;
    static QVector<int> buildLogicalToSourceOffsets(const QString& text, int logicalTextLength);
    static int logicalOffsetForSourceOffsetInText(const QString& text, int sourceOffset);

    void refreshTextState();

    QString m_text;
    QString m_logicalText;
    QVector<int> m_logicalToSourceOffsets;
    int m_logicalLineCount = 1;
};

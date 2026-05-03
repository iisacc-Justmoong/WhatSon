#pragma once

#include <QObject>
#include <QList>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class ContentsGutterMarkerGeometry : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool editorMounted READ editorMounted WRITE setEditorMounted NOTIFY editorMountedChanged)
    Q_PROPERTY(QString sourceText READ sourceText WRITE setSourceText NOTIFY sourceTextChanged)
    Q_PROPERTY(QString savedSourceText READ savedSourceText WRITE setSavedSourceText NOTIFY savedSourceTextChanged)
    Q_PROPERTY(QVariantList lineNumberEntries READ lineNumberEntries WRITE setLineNumberEntries NOTIFY lineNumberEntriesChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(int lineNumberBaseOffset READ lineNumberBaseOffset WRITE setLineNumberBaseOffset NOTIFY lineNumberBaseOffsetChanged)
    Q_PROPERTY(qreal markerHeight READ markerHeight WRITE setMarkerHeight NOTIFY markerHeightChanged)
    Q_PROPERTY(int cursorLineNumber READ cursorLineNumber NOTIFY cursorLineNumberChanged)
    Q_PROPERTY(QVariantList markerEntries READ markerEntries NOTIFY markerEntriesChanged)

public:
    explicit ContentsGutterMarkerGeometry(QObject* parent = nullptr);
    ~ContentsGutterMarkerGeometry() override;

    bool editorMounted() const noexcept;
    void setEditorMounted(bool value);
    QString sourceText() const;
    void setSourceText(const QString& value);
    QString savedSourceText() const;
    void setSavedSourceText(const QString& value);
    QVariantList lineNumberEntries() const;
    void setLineNumberEntries(const QVariantList& value);
    int cursorPosition() const noexcept;
    void setCursorPosition(int value);
    int lineNumberBaseOffset() const noexcept;
    void setLineNumberBaseOffset(int value);
    qreal markerHeight() const noexcept;
    void setMarkerHeight(qreal value);
    int cursorLineNumber() const noexcept;
    QVariantList markerEntries() const;

public slots:
    void refresh();

signals:
    void editorMountedChanged();
    void sourceTextChanged();
    void savedSourceTextChanged();
    void lineNumberEntriesChanged();
    void cursorPositionChanged();
    void lineNumberBaseOffsetChanged();
    void markerHeightChanged();
    void cursorLineNumberChanged();
    void markerEntriesChanged();

private:
    int sourceLineIndexForOffset(int offset) const noexcept;
    QList<int> unsavedLineNumbers() const;
    qreal effectiveMarkerHeight() const noexcept;
    qreal yForLineNumber(int lineNumber) const;
    qreal heightForLineSpan(int startLineNumber, int lineSpan) const;
    QVariantMap markerEntry(const QString& type, int lineNumber, int lineSpan) const;
    void rebuildMarkerEntries();

    bool m_editorMounted = false;
    QString m_sourceText;
    QString m_savedSourceText;
    QVariantList m_lineNumberEntries;
    int m_cursorPosition = 0;
    int m_lineNumberBaseOffset = 1;
    qreal m_markerHeight = 1.0;
    int m_cursorLineNumber = -1;
    QVariantList m_markerEntries;
};

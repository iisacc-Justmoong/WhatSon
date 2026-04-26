#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class ContentsDisplayStructuredFlowCoordinator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool structuredHostGeometryActive READ structuredHostGeometryActive WRITE setStructuredHostGeometryActive NOTIFY structuredHostGeometryActiveChanged)
    Q_PROPERTY(double editorLineHeight READ editorLineHeight WRITE setEditorLineHeight NOTIFY editorLineHeightChanged)
    Q_PROPERTY(double gutterViewportHeight READ gutterViewportHeight WRITE setGutterViewportHeight NOTIFY gutterViewportHeightChanged)
    Q_PROPERTY(double editorDocumentStartY READ editorDocumentStartY WRITE setEditorDocumentStartY NOTIFY editorDocumentStartYChanged)
    Q_PROPERTY(double editorContentOffsetY READ editorContentOffsetY WRITE setEditorContentOffsetY NOTIFY editorContentOffsetYChanged)
    Q_PROPERTY(int logicalLineCount READ logicalLineCount WRITE setLogicalLineCount NOTIFY logicalLineCountChanged)
    Q_PROPERTY(QString structuredGutterGeometrySignature READ structuredGutterGeometrySignature WRITE setStructuredGutterGeometrySignature NOTIFY structuredGutterGeometrySignatureChanged)

public:
    explicit ContentsDisplayStructuredFlowCoordinator(QObject* parent = nullptr);
    ~ContentsDisplayStructuredFlowCoordinator() override;

    bool structuredHostGeometryActive() const noexcept;
    void setStructuredHostGeometryActive(bool active);

    double editorLineHeight() const noexcept;
    void setEditorLineHeight(double value);

    double gutterViewportHeight() const noexcept;
    void setGutterViewportHeight(double value);

    double editorDocumentStartY() const noexcept;
    void setEditorDocumentStartY(double value);

    double editorContentOffsetY() const noexcept;
    void setEditorContentOffsetY(double value);

    int logicalLineCount() const noexcept;
    void setLogicalLineCount(int value);

    QString structuredGutterGeometrySignature() const;
    void setStructuredGutterGeometrySignature(const QString& signature);

    Q_INVOKABLE QVariantList normalizeStructuredLogicalLineEntries(const QVariant& rawEntries) const;
    Q_INVOKABLE QVariantMap evaluateStructuredLayoutState(const QVariant& rawEntries) const;
    Q_INVOKABLE QVariantList buildVisibleStructuredGutterLineEntries(const QVariantList& lineEntries, int fallbackFirstVisibleLine) const;

signals:
    void structuredHostGeometryActiveChanged();
    void editorLineHeightChanged();
    void gutterViewportHeightChanged();
    void editorDocumentStartYChanged();
    void editorContentOffsetYChanged();
    void logicalLineCountChanged();
    void structuredGutterGeometrySignatureChanged();

private:
    double editorViewportYForDocumentY(double documentY) const noexcept;
    QString buildStructuredGutterGeometrySignature(const QVariantList& lineEntries) const;

    bool m_structuredHostGeometryActive = false;
    double m_editorLineHeight = 0.0;
    double m_gutterViewportHeight = 0.0;
    double m_editorDocumentStartY = 0.0;
    double m_editorContentOffsetY = 0.0;
    int m_logicalLineCount = 1;
    QString m_structuredGutterGeometrySignature;
};

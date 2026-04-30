#pragma once

#include <QObject>
#include <QString>

class ContentsDisplayPresentationState : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int documentPresentationRefreshIntervalMs READ documentPresentationRefreshIntervalMs WRITE setDocumentPresentationRefreshIntervalMs NOTIFY stateChanged)
    Q_PROPERTY(QString renderedEditorHtml READ renderedEditorHtml WRITE setRenderedEditorHtml NOTIFY stateChanged)

public:
    explicit ContentsDisplayPresentationState(QObject* parent = nullptr);
    ~ContentsDisplayPresentationState() override;

    int documentPresentationRefreshIntervalMs() const noexcept;
    QString renderedEditorHtml() const;

public slots:
    void setDocumentPresentationRefreshIntervalMs(int value);
    void setRenderedEditorHtml(const QString& value);

signals:
    void stateChanged();

private:
    int m_documentPresentationRefreshIntervalMs = 120;
    QString m_renderedEditorHtml;
};

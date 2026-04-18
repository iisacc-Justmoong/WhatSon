#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

class WeeklyUnusedNote final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString hubPath READ hubPath WRITE setHubPath NOTIFY hubPathChanged)
    Q_PROPERTY(QVariantList unusedNotes READ unusedNotes NOTIFY unusedNotesChanged)
    Q_PROPERTY(QStringList unusedNoteIds READ unusedNoteIds NOTIFY unusedNotesChanged)
    Q_PROPERTY(int unusedNoteCount READ unusedNoteCount NOTIFY unusedNotesChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit WeeklyUnusedNote(QObject* parent = nullptr);
    ~WeeklyUnusedNote() override;

    QString hubPath() const;
    void setHubPath(QString hubPath);

    QVariantList unusedNotes() const;
    QStringList unusedNoteIds() const;
    int unusedNoteCount() const noexcept;
    QString lastError() const;

    Q_INVOKABLE QVariantList scanUnusedNotes(const QString& hubPath = QString());
    Q_INVOKABLE QStringList collectUnusedNoteIds(const QString& hubPath = QString());

public slots:
    void refresh();

signals:
    void hubPathChanged();
    void unusedNotesChanged();
    void lastErrorChanged();
    void scanCompleted(const QVariantList& unusedNotes);

private:
    void setLastError(QString errorMessage);
    void setUnusedNotes(QVariantList unusedNotes);

    QString m_hubPath;
    QVariantList m_unusedNotes;
    QString m_lastError;
};

#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantList>

class NoteListModelContractBridge : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* noteListModel READ noteListModel WRITE setNoteListModel NOTIFY noteListModelChanged)
    Q_PROPERTY(bool hasNoteListModel READ hasNoteListModel NOTIFY hasNoteListModelChanged)
    Q_PROPERTY(bool searchContractAvailable READ searchContractAvailable NOTIFY searchContractAvailableChanged)
    Q_PROPERTY(bool currentIndexContractAvailable READ currentIndexContractAvailable
                   NOTIFY currentIndexContractAvailableChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentNoteId READ currentNoteId NOTIFY currentNoteIdChanged)

public:
    explicit NoteListModelContractBridge(QObject* parent = nullptr);
    ~NoteListModelContractBridge() override;

    QObject* noteListModel() const noexcept;
    void setNoteListModel(QObject* model);

    bool hasNoteListModel() const noexcept;
    bool searchContractAvailable() const noexcept;
    bool currentIndexContractAvailable() const noexcept;
    int currentIndex() const;
    QString currentNoteId() const;

    Q_INVOKABLE bool applySearchText(const QString& searchText);
    Q_INVOKABLE int readCurrentIndex() const;
    Q_INVOKABLE QString readCurrentNoteId() const;
    Q_INVOKABLE QString readNoteIdAt(int index) const;
    Q_INVOKABLE QVariantList readAllRows() const;
    Q_INVOKABLE bool pushCurrentIndex(int index);

signals:
    void noteListModelChanged();
    void hasNoteListModelChanged();
    void searchContractAvailableChanged();
    void currentIndexContractAvailableChanged();
    void currentIndexChanged();
    void currentNoteIdChanged();

private slots:
    void handleCurrentIndexChanged();
    void handleCurrentNoteIdChanged();
    void handleNoteListDestroyed();

private:
    static bool hasReadableProperty(const QObject* object, const char* propertyName);
    static bool hasWritableProperty(const QObject* object, const char* propertyName);
    static bool hasInvokableMethod(const QObject* object, const char* methodSignature);
    static QString readStringProperty(const QObject* object, const char* propertyName);
    static int readIntProperty(const QObject* object, const char* propertyName, int fallbackValue);

    void refreshContracts();
    void disconnectNoteListModel();

    QPointer<QObject> m_noteListModel;
    bool m_searchContractAvailable = false;
    bool m_currentIndexContractAvailable = false;
    QMetaObject::Connection m_noteListDestroyedConnection;
    QMetaObject::Connection m_currentIndexChangedConnection;
    QMetaObject::Connection m_currentNoteIdChangedConnection;
};

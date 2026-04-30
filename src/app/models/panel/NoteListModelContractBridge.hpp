#pragma once

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class NoteListModelContractBridge : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* hierarchyController READ hierarchyController WRITE setHierarchyController NOTIFY hierarchyControllerChanged)
    Q_PROPERTY(QObject* noteListModel READ noteListModel WRITE setNoteListModel NOTIFY noteListModelChanged)
    Q_PROPERTY(bool hasNoteListModel READ hasNoteListModel NOTIFY hasNoteListModelChanged)
    Q_PROPERTY(bool searchContractAvailable READ searchContractAvailable NOTIFY searchContractAvailableChanged)
    Q_PROPERTY(bool currentIndexContractAvailable READ currentIndexContractAvailable
                   NOTIFY currentIndexContractAvailableChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QVariantMap currentNoteEntry READ currentNoteEntry NOTIFY currentNoteEntryChanged)
    Q_PROPERTY(QString currentNoteId READ currentNoteId NOTIFY currentNoteIdChanged)

public:
    explicit NoteListModelContractBridge(QObject* parent = nullptr);
    ~NoteListModelContractBridge() override;

    QObject* hierarchyController() const noexcept;
    void setHierarchyController(QObject* model);

    QObject* noteListModel() const noexcept;
    void setNoteListModel(QObject* model);

    bool hasNoteListModel() const noexcept;
    bool searchContractAvailable() const noexcept;
    bool currentIndexContractAvailable() const noexcept;
    int currentIndex() const;
    QVariantMap currentNoteEntry() const;
    QString currentNoteId() const;

    Q_INVOKABLE bool applySearchText(const QString& searchText);
    Q_INVOKABLE int readCurrentIndex() const;
    Q_INVOKABLE QVariantMap readCurrentNoteEntry() const;
    Q_INVOKABLE QString readCurrentNoteId() const;
    Q_INVOKABLE QString readNoteIdAt(int index) const;
    Q_INVOKABLE QVariantList readAllRows() const;
    Q_INVOKABLE QVariantList readAllRowsForModel(QObject* model) const;
    Q_INVOKABLE bool pushCurrentIndex(int index);

signals:
    void hierarchyControllerChanged();
    void noteListModelChanged();
    void hasNoteListModelChanged();
    void searchContractAvailableChanged();
    void currentIndexContractAvailableChanged();
    void currentIndexChanged();
    void currentNoteEntryChanged();
    void currentNoteIdChanged();

private slots:
    void handleCurrentIndexChanged();
    void handleCurrentNoteEntryChanged();
    void handleCurrentNoteIdChanged();
    void handleNoteListDestroyed();
    void handleHierarchyControllerDestroyed();

private:
    static bool hasReadableProperty(const QObject* object, const char* propertyName);
    static bool hasWritableProperty(const QObject* object, const char* propertyName);
    static bool hasInvokableMethod(const QObject* object, const char* methodSignature);
    static bool hasSignal(const QObject* object, const char* signalSignature);
    static QString readStringProperty(const QObject* object, const char* propertyName);
    static int readIntProperty(const QObject* object, const char* propertyName, int fallbackValue);
    static QObject* readObjectProperty(const QObject* object, const char* propertyName);
    static QObject* invokeObjectMethod(QObject* object, const char* methodName);
    static QObject* resolveNoteListModelFromHierarchyController(const QObject* hierarchyController);

    void refreshResolvedNoteListModel();
    void setResolvedNoteListModel(QObject* model);
    void refreshContracts();
    void disconnectHierarchyController();
    void disconnectNoteListModel();

    QPointer<QObject> m_hierarchyController;
    QPointer<QObject> m_explicitNoteListModel;
    QPointer<QObject> m_noteListModel;
    bool m_searchContractAvailable = false;
    bool m_currentIndexContractAvailable = false;
    QMetaObject::Connection m_hierarchyControllerDestroyedConnection;
    QMetaObject::Connection m_noteListDestroyedConnection;
    QMetaObject::Connection m_currentIndexChangedConnection;
    QMetaObject::Connection m_currentNoteEntryChangedConnection;
    QMetaObject::Connection m_currentNoteIdChangedConnection;
};

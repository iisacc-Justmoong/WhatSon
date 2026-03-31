#include "viewmodel/panel/NoteListModelContractBridge.hpp"

#include <QObject>
#include <QtTest/QtTest>

class FakePropertyNoteListModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentNoteId READ currentNoteId WRITE setCurrentNoteId NOTIFY currentNoteIdChanged)

public:
    QString searchText() const
    {
        return m_searchText;
    }

    void setSearchText(const QString& searchText)
    {
        if (m_searchText == searchText)
        {
            return;
        }

        m_searchText = searchText;
        emit searchTextChanged();
    }

    int currentIndex() const noexcept
    {
        return m_currentIndex;
    }

    void setCurrentIndex(int index)
    {
        if (m_currentIndex == index)
        {
            return;
        }

        m_currentIndex = index;
        emit currentIndexChanged();
    }

    QString currentNoteId() const
    {
        return m_currentNoteId;
    }

    void setCurrentNoteId(const QString& noteId)
    {
        const QString normalizedNoteId = noteId.trimmed();
        if (m_currentNoteId == normalizedNoteId)
        {
            return;
        }

        m_currentNoteId = normalizedNoteId;
        emit currentNoteIdChanged();
    }

signals:
    void searchTextChanged();
    void currentIndexChanged();
    void currentNoteIdChanged();

private:
    QString m_searchText;
    int m_currentIndex = -1;
    QString m_currentNoteId;
};

class FakeMethodOnlyNoteListModel final : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE void setSearchText(const QString& searchText)
    {
        m_searchText = searchText;
        emit searchTextChanged();
    }

    Q_INVOKABLE void setCurrentIndex(int index)
    {
        m_currentIndex = index;
        emit currentIndexChanged();
    }

    QString searchText() const
    {
        return m_searchText;
    }

    int currentIndex() const noexcept
    {
        return m_currentIndex;
    }

signals:
    void searchTextChanged();
    void currentIndexChanged();

private:
    QString m_searchText;
    int m_currentIndex = -1;
};

class NoteListModelContractBridgeTest final : public QObject
{
    Q_OBJECT

private slots:
    void propertyContracts_mustRoundTripSearchAndSelection();
    void methodContracts_mustStayUsableAndResetAfterModelDestroy();
};

void NoteListModelContractBridgeTest::propertyContracts_mustRoundTripSearchAndSelection()
{
    FakePropertyNoteListModel noteListModel;
    NoteListModelContractBridge bridge;
    bridge.setNoteListModel(&noteListModel);

    QVERIFY(bridge.hasNoteListModel());
    QVERIFY(bridge.searchContractAvailable());
    QVERIFY(bridge.currentIndexContractAvailable());

    QVERIFY(bridge.applySearchText(QStringLiteral("ready")));
    QCOMPARE(noteListModel.searchText(), QStringLiteral("ready"));

    QVERIFY(bridge.pushCurrentIndex(2));
    QCOMPARE(noteListModel.currentIndex(), 2);
    QCOMPARE(bridge.readCurrentIndex(), 2);

    QSignalSpy noteIdSpy(&bridge, &NoteListModelContractBridge::currentNoteIdChanged);
    noteListModel.setCurrentNoteId(QStringLiteral("note-ready"));
    QCOMPARE(bridge.readCurrentNoteId(), QStringLiteral("note-ready"));
    QCOMPARE(noteIdSpy.count(), 1);
}

void NoteListModelContractBridgeTest::methodContracts_mustStayUsableAndResetAfterModelDestroy()
{
    NoteListModelContractBridge bridge;

    QSignalSpy hasModelSpy(&bridge, &NoteListModelContractBridge::hasNoteListModelChanged);
    QSignalSpy searchContractSpy(&bridge, &NoteListModelContractBridge::searchContractAvailableChanged);
    QSignalSpy selectionContractSpy(&bridge, &NoteListModelContractBridge::currentIndexContractAvailableChanged);

    auto* methodOnlyModel = new FakeMethodOnlyNoteListModel();
    bridge.setNoteListModel(methodOnlyModel);
    QCOMPARE(hasModelSpy.count(), 1);
    QVERIFY(bridge.hasNoteListModel());
    QVERIFY(bridge.searchContractAvailable());
    QVERIFY(bridge.currentIndexContractAvailable());
    QVERIFY(searchContractSpy.count() >= 1);
    QVERIFY(selectionContractSpy.count() >= 1);

    QVERIFY(bridge.applySearchText(QStringLiteral("queued")));
    QVERIFY(bridge.pushCurrentIndex(7));
    QCOMPARE(methodOnlyModel->searchText(), QStringLiteral("queued"));
    QCOMPARE(methodOnlyModel->currentIndex(), 7);

    delete methodOnlyModel;
    QVERIFY(!bridge.hasNoteListModel());
    QVERIFY(!bridge.searchContractAvailable());
    QVERIFY(!bridge.currentIndexContractAvailable());
    QVERIFY(hasModelSpy.count() >= 2);
}

QTEST_MAIN(NoteListModelContractBridgeTest)

#include "test_note_list_model_contract_bridge.moc"

#include "viewmodel/panel/FocusedNoteDeletionBridge.hpp"

#include <QObject>
#include <QtTest/QtTest>

class FakeDeleteNoteListModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentNoteId READ currentNoteId WRITE setCurrentNoteId NOTIFY currentNoteIdChanged)

public:
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

    signals  :


    void currentNoteIdChanged();

private:
    QString m_currentNoteId;
};

class FakeDeleteTarget final : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE bool deleteNoteById(const QString& noteId)
    {
        lastDeletedNoteId = noteId;
        deleteCallCount += 1;
        return deleteResult;
    }

    QString lastDeletedNoteId;
    int deleteCallCount = 0;
    bool deleteResult = true;
};

class FocusedNoteDeletionBridgeTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void deleteFocusedNote_mustPreferImmediateViewFocusId();
    void deleteFocusedNote_mustFallbackToModelSelection();
    void deleteContract_mustTrackTargetAndFocusAvailability();
};

void FocusedNoteDeletionBridgeTest::deleteFocusedNote_mustPreferImmediateViewFocusId()
{
    FakeDeleteNoteListModel noteListModel;
    noteListModel.setCurrentNoteId(QStringLiteral("note-from-model"));

    FakeDeleteTarget deleteTarget;

    FocusedNoteDeletionBridge bridge;
    bridge.setNoteListModel(&noteListModel);
    bridge.setDeletionTarget(&deleteTarget);
    bridge.setFocusedNoteId(QStringLiteral("note-from-view"));

    QVERIFY(bridge.deleteFocusedNote());
    QCOMPARE(deleteTarget.deleteCallCount, 1);
    QCOMPARE(deleteTarget.lastDeletedNoteId, QStringLiteral("note-from-view"));
    QCOMPARE(bridge.focusedNoteId(), QString());
}

void FocusedNoteDeletionBridgeTest::deleteFocusedNote_mustFallbackToModelSelection()
{
    FakeDeleteNoteListModel noteListModel;
    noteListModel.setCurrentNoteId(QStringLiteral("note-current"));

    FakeDeleteTarget deleteTarget;

    FocusedNoteDeletionBridge bridge;
    bridge.setNoteListModel(&noteListModel);
    bridge.setDeletionTarget(&deleteTarget);

    QVERIFY(bridge.focusedNoteAvailable());
    QVERIFY(bridge.deleteFocusedNote());
    QCOMPARE(deleteTarget.deleteCallCount, 1);
    QCOMPARE(deleteTarget.lastDeletedNoteId, QStringLiteral("note-current"));
}

void FocusedNoteDeletionBridgeTest::deleteContract_mustTrackTargetAndFocusAvailability()
{
    FakeDeleteNoteListModel noteListModel;
    FakeDeleteTarget deleteTarget;

    FocusedNoteDeletionBridge bridge;
    QVERIFY(!bridge.deleteContractAvailable());
    QVERIFY(!bridge.focusedNoteAvailable());

    bridge.setNoteListModel(&noteListModel);
    bridge.setDeletionTarget(&deleteTarget);
    QVERIFY(bridge.deleteContractAvailable());
    QVERIFY(!bridge.focusedNoteAvailable());

    noteListModel.setCurrentNoteId(QStringLiteral("note-late"));
    QVERIFY(bridge.focusedNoteAvailable());

    bridge.setFocusedNoteId(QStringLiteral("note-now"));
    QVERIFY(bridge.focusedNoteAvailable());
}

QTEST_MAIN(FocusedNoteDeletionBridgeTest)

#include "test_focused_note_deletion_bridge.moc"

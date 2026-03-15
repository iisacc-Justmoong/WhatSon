#include "viewmodel/content/ContentsEditorSelectionBridge.hpp"
#include "viewmodel/content/ContentsGutterMarkerBridge.hpp"
#include "viewmodel/content/ContentsLogicalTextBridge.hpp"

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QtTest/QtTest>

#include <algorithm>

class FakeNoteListModel final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentNoteId READ currentNoteId WRITE setCurrentNoteId NOTIFY currentNoteIdChanged)
    Q_PROPERTY(QString currentBodyText READ currentBodyText WRITE setCurrentBodyText NOTIFY currentBodyTextChanged)
    Q_PROPERTY(int itemCount READ itemCount WRITE setItemCount NOTIFY itemCountChanged)

public:
    QString currentNoteId() const
    {
        return m_currentNoteId;
    }

    void setCurrentNoteId(const QString& noteId)
    {
        if (m_currentNoteId == noteId)
        {
            return;
        }
        m_currentNoteId = noteId;
        emit currentNoteIdChanged();
    }

    QString currentBodyText() const
    {
        return m_currentBodyText;
    }

    void setCurrentBodyText(const QString& bodyText)
    {
        if (m_currentBodyText == bodyText)
        {
            return;
        }
        m_currentBodyText = bodyText;
        emit currentBodyTextChanged();
    }

    int itemCount() const noexcept
    {
        return m_itemCount;
    }

    void setItemCount(int count)
    {
        const int normalizedCount = std::max(0, count);
        if (m_itemCount == normalizedCount)
        {
            return;
        }
        m_itemCount = normalizedCount;
        emit itemCountChanged();
    }

    signals  :


    void currentNoteIdChanged();
    void currentBodyTextChanged();
    void itemCountChanged();

private:
    QString m_currentNoteId;
    QString m_currentBodyText;
    int m_itemCount = 0;
};

class FakeContentViewModel final : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE bool saveBodyTextForNote(const QString& noteId, const QString& text)
    {
        lastSavedNoteId = noteId;
        lastSavedText = text;
        saveBodyTextForNoteCallCount += 1;
        return saveBodyTextForNoteResult;
    }

    Q_INVOKABLE bool saveCurrentBodyText(const QString& text)
    {
        lastSavedText = text;
        saveCurrentBodyTextCallCount += 1;
        return saveCurrentBodyTextResult;
    }

    QString lastSavedNoteId;
    QString lastSavedText;
    int saveBodyTextForNoteCallCount = 0;
    int saveCurrentBodyTextCallCount = 0;
    bool saveBodyTextForNoteResult = true;
    bool saveCurrentBodyTextResult = true;
};

class ContentsEditorAdapterTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void textMetrics_mustTrackLogicalOffsetsAndLineQueries();
    void noteContracts_mustMirrorNoteListSelectionState();
    void gutterMarkers_mustNormalizeSupportedMarkerTypes();
    void persistence_mustDelegateToContentViewModel();
};

void ContentsEditorAdapterTest::textMetrics_mustTrackLogicalOffsetsAndLineQueries()
{
    ContentsLogicalTextBridge bridge;
    bridge.setText(QStringLiteral("alpha\nbeta\n"));

    const QVariantList offsets = bridge.logicalLineStartOffsets();
    QCOMPARE(offsets.size(), 3);
    QCOMPARE(offsets.at(0).toInt(), 0);
    QCOMPARE(offsets.at(1).toInt(), 6);
    QCOMPARE(offsets.at(2).toInt(), 11);
    QCOMPARE(bridge.logicalLineCount(), 3);
    QCOMPARE(bridge.logicalLineNumberForOffset(0), 1);
    QCOMPARE(bridge.logicalLineNumberForOffset(6), 2);
    QCOMPARE(bridge.logicalLineNumberForOffset(11), 3);
    QCOMPARE(bridge.logicalLineStartOffsetAt(1), 6);
    QCOMPARE(bridge.logicalLineCharacterCountAt(0), 5);
    QCOMPARE(bridge.logicalLineCharacterCountAt(1), 4);
    QCOMPARE(bridge.logicalLineCharacterCountAt(2), 0);
}

void ContentsEditorAdapterTest::noteContracts_mustMirrorNoteListSelectionState()
{
    FakeNoteListModel noteListModel;
    noteListModel.setCurrentNoteId(QStringLiteral("note-42"));
    noteListModel.setCurrentBodyText(QStringLiteral("body text"));
    noteListModel.setItemCount(7);

    ContentsEditorSelectionBridge bridge;
    bridge.setNoteListModel(&noteListModel);

    QVERIFY(bridge.noteSelectionContractAvailable());
    QVERIFY(bridge.noteCountContractAvailable());
    QCOMPARE(bridge.selectedNoteId(), QStringLiteral("note-42"));
    QCOMPARE(bridge.selectedNoteBodyText(), QStringLiteral("body text"));
    QCOMPARE(bridge.visibleNoteCount(), 7);

    noteListModel.setCurrentNoteId(QStringLiteral("note-77"));
    noteListModel.setCurrentBodyText(QStringLiteral("updated body"));
    noteListModel.setItemCount(2);

    QCOMPARE(bridge.selectedNoteId(), QStringLiteral("note-77"));
    QCOMPARE(bridge.selectedNoteBodyText(), QStringLiteral("updated body"));
    QCOMPARE(bridge.visibleNoteCount(), 2);
}

void ContentsEditorAdapterTest::gutterMarkers_mustNormalizeSupportedMarkerTypes()
{
    ContentsGutterMarkerBridge bridge;
    bridge.setGutterMarkers(QVariantList{
        QVariantMap{
            {QStringLiteral("line"), 2},
            {QStringLiteral("type"), QStringLiteral("Changed")}
        },
        QVariantMap{
            {QStringLiteral("startLine"), 5},
            {QStringLiteral("endLine"), 7},
            {QStringLiteral("type"), QStringLiteral("conflict")}
        },
        QVariantMap{
            {QStringLiteral("line"), 9},
            {QStringLiteral("type"), QStringLiteral("ignored")}
        }
    });

    const QVariantList normalized = bridge.normalizedExternalGutterMarkers();
    QCOMPARE(normalized.size(), 2);

    const QVariantMap firstMarker = normalized.at(0).toMap();
    QCOMPARE(firstMarker.value(QStringLiteral("startLine")).toInt(), 2);
    QCOMPARE(firstMarker.value(QStringLiteral("lineSpan")).toInt(), 1);
    QCOMPARE(firstMarker.value(QStringLiteral("type")).toString(), QStringLiteral("changed"));

    const QVariantMap secondMarker = normalized.at(1).toMap();
    QCOMPARE(secondMarker.value(QStringLiteral("startLine")).toInt(), 5);
    QCOMPARE(secondMarker.value(QStringLiteral("lineSpan")).toInt(), 3);
    QCOMPARE(secondMarker.value(QStringLiteral("type")).toString(), QStringLiteral("conflict"));
}

void ContentsEditorAdapterTest::persistence_mustDelegateToContentViewModel()
{
    FakeContentViewModel contentViewModel;
    ContentsEditorSelectionBridge bridge;
    bridge.setContentViewModel(&contentViewModel);

    QVERIFY(bridge.contentPersistenceContractAvailable());
    QVERIFY(bridge.persistEditorTextForNote(QStringLiteral("note-a"), QStringLiteral("text-a")));
    QCOMPARE(contentViewModel.lastSavedNoteId, QStringLiteral("note-a"));
    QCOMPARE(contentViewModel.lastSavedText, QStringLiteral("text-a"));
    QCOMPARE(contentViewModel.saveBodyTextForNoteCallCount, 1);
    QCOMPARE(contentViewModel.saveCurrentBodyTextCallCount, 0);
}

QTEST_MAIN(ContentsEditorAdapterTest)

#include "test_contents_editor_bridge.moc"

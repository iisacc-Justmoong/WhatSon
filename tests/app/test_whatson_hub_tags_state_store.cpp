#include "hierarchy/tags/WhatSonHubTagsStateStore.hpp"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

class WhatSonHubTagsStateStoreTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void loadFromWshub_storesDepthEntriesPerHub();
    void setEntries_remove_clear_manageRuntimeState();
};

void WhatSonHubTagsStateStoreTest::loadFromWshub_storesDepthEntriesPerHub()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString hubPath = QDir(tempDir.path()).filePath(QStringLiteral("Runtime.wshub"));
    const QString contentsPath = QDir(hubPath).filePath(QStringLiteral("Runtime.wscontents"));
    QVERIFY(QDir().mkpath(contentsPath));

    const QString tagsPath = QDir(contentsPath).filePath(QStringLiteral("Tags.wstags"));
    QFile tagsFile(tagsPath);
    QVERIFY(tagsFile.open(QIODevice::WriteOnly | QIODevice::Text));
    tagsFile.write(
        "{\n"
        "  \"tags\": [\n"
        "    {\n"
        "      \"id\": \"alpha\",\n"
        "      \"label\": \"Alpha\",\n"
        "      \"children\": [\n"
        "        {\"id\": \"alpha/beta\", \"label\": \"Beta\"}\n"
        "      ]\n"
        "    }\n"
        "  ]\n"
        "}\n");
    tagsFile.close();

    WhatSonHubTagsStateStore store;
    QString errorMessage;
    QVERIFY2(store.loadFromWshub(hubPath, &errorMessage), qPrintable(errorMessage));
    QVERIFY(store.contains(hubPath));

    const QVector<WhatSonTagDepthEntry> entries = store.entries(hubPath);
    QCOMPARE(entries.size(), 2);
    QCOMPARE(entries.at(0).depth, 0);
    QCOMPARE(entries.at(1).depth, 0);
}

void WhatSonHubTagsStateStoreTest::setEntries_remove_clear_manageRuntimeState()
{
    WhatSonHubTagsStateStore store;
    const QString hubPath = QStringLiteral("/tmp/demo.wshub");

    QVector<WhatSonTagDepthEntry> entries;
    entries.push_back(WhatSonTagDepthEntry{QStringLiteral("one"), QStringLiteral("One"), 0});
    entries.push_back(WhatSonTagDepthEntry{QStringLiteral("one/two"), QStringLiteral("Two"), 1});

    store.setEntries(hubPath, entries);
    QVERIFY(store.contains(hubPath));
    QCOMPARE(store.entries(hubPath).size(), 2);

    store.remove(hubPath);
    QVERIFY(!store.contains(hubPath));

    store.setEntries(hubPath, entries);
    store.clear();
    QVERIFY(store.hubPaths().isEmpty());
}

QTEST_APPLESS_MAIN(WhatSonHubTagsStateStoreTest)

#include "test_whatson_hub_tags_state_store.moc"

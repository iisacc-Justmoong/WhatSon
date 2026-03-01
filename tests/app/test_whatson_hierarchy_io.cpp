#include "bookmarks/WhatSonBookmarksHierarchyCreator.hpp"
#include "bookmarks/WhatSonBookmarksHierarchyParser.hpp"
#include "bookmarks/WhatSonBookmarksHierarchyStore.hpp"
#include "event/WhatSonEventHierarchyCreator.hpp"
#include "event/WhatSonEventHierarchyParser.hpp"
#include "event/WhatSonEventHierarchyStore.hpp"
#include "library/WhatSonLibraryHierarchyCreator.hpp"
#include "library/WhatSonLibraryHierarchyParser.hpp"
#include "library/WhatSonLibraryHierarchyStore.hpp"
#include "preset/WhatSonPresetHierarchyCreator.hpp"
#include "preset/WhatSonPresetHierarchyParser.hpp"
#include "preset/WhatSonPresetHierarchyStore.hpp"
#include "progress/WhatSonProgressHierarchyCreator.hpp"
#include "progress/WhatSonProgressHierarchyParser.hpp"
#include "progress/WhatSonProgressHierarchyStore.hpp"
#include "projects/WhatSonProjectsHierarchyCreator.hpp"
#include "projects/WhatSonProjectsHierarchyParser.hpp"
#include "projects/WhatSonProjectsHierarchyStore.hpp"
#include "resources/WhatSonResourcesHierarchyCreator.hpp"
#include "resources/WhatSonResourcesHierarchyParser.hpp"
#include "resources/WhatSonResourcesHierarchyStore.hpp"
#include "tags/WhatSonTagDepthEntry.hpp"
#include "tags/WhatSonTagsHierarchyCreator.hpp"
#include "tags/WhatSonTagsHierarchyParser.hpp"
#include "tags/WhatSonTagsHierarchyStore.hpp"

#include <QDir>
#include <QFile>
#include <QtTest>

class WhatSonHierarchyIoTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void parse_blueprintFiles();
    void creatorParser_roundTrip();
    void libraryParser_parsesObjectStyleNotes();
    void bookmarksStore_hasNineHexCriteriaAndNormalizesInput();
};

namespace
{
    QString repositoryRootPath()
    {
        QDir dir(QStringLiteral(QT_TESTCASE_SOURCEDIR));
        if (dir.dirName() == QStringLiteral("tests"))
        {
            dir.cdUp();
        }
        return dir.absolutePath();
    }

    QString readUtf8File(const QString& absolutePath)
    {
        QFile file(absolutePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }
        return QString::fromUtf8(file.readAll());
    }
} // namespace

void WhatSonHierarchyIoTest::parse_blueprintFiles()
{
    const QString root = repositoryRootPath();
    const QString contentsRoot =
        QDir(root).filePath(QStringLiteral("blueprint/TestHub.wshub/TestHub.wscontents"));

    WhatSonBookmarksHierarchyStore bookmarksStore;
    WhatSonBookmarksHierarchyParser bookmarksParser;
    QString errorMessage;
    QVERIFY2(
        bookmarksParser.parse(
            readUtf8File(QDir(contentsRoot).filePath(QStringLiteral("Bookmarks.wsbookmarks"))),
            &bookmarksStore,
            &errorMessage),
        qPrintable(errorMessage));
    QVERIFY(bookmarksStore.bookmarkIds().isEmpty());

    WhatSonProjectsHierarchyStore projectsStore;
    WhatSonProjectsHierarchyParser projectsParser;
    QVERIFY2(
        projectsParser.parse(
            readUtf8File(QDir(contentsRoot).filePath(QStringLiteral("Folders.wsfolders"))),
            &projectsStore,
            &errorMessage),
        qPrintable(errorMessage));
    QCOMPARE(projectsStore.folderEntries().size(), 6);
    QCOMPARE(projectsStore.folderEntries().at(0).label, QStringLiteral("Research"));
    QCOMPARE(projectsStore.folderEntries().at(1).depth, 1);
    QCOMPARE(projectsStore.folderEntries().at(2).depth, 2);
    QVERIFY(projectsStore.projectNames().contains(QStringLiteral("Campaign")));

    WhatSonLibraryHierarchyStore libraryStore;
    WhatSonLibraryHierarchyParser libraryParser;
    QVERIFY2(
        libraryParser.parse(
            readUtf8File(
                QDir(contentsRoot).filePath(QStringLiteral("Library.wslibrary/index.wsnindex"))),
            &libraryStore,
            &errorMessage),
        qPrintable(errorMessage));
    QVERIFY(libraryStore.noteIds().isEmpty());

    WhatSonProgressHierarchyStore progressStore;
    WhatSonProgressHierarchyParser progressParser;
    QVERIFY2(
        progressParser.parse(
            readUtf8File(QDir(contentsRoot).filePath(QStringLiteral("Progress.wsprogress"))),
            &progressStore,
            &errorMessage),
        qPrintable(errorMessage));
    QCOMPARE(progressStore.progressValue(), 0);
    QCOMPARE(progressStore.progressStates().size(), 4);

    WhatSonTagsHierarchyStore tagsStore;
    WhatSonTagsHierarchyParser tagsParser;
    QVERIFY2(
        tagsParser.parse(
            readUtf8File(QDir(contentsRoot).filePath(QStringLiteral("Tags.wstags"))),
            &tagsStore,
            &errorMessage),
        qPrintable(errorMessage));
    QVERIFY(tagsStore.tagEntries().size() > 0);
}

void WhatSonHierarchyIoTest::creatorParser_roundTrip()
{
    QString errorMessage;

    {
        WhatSonBookmarksHierarchyStore store;
        store.setBookmarkIds({QStringLiteral("bookmark://A"), QStringLiteral("bookmark://B")});
        WhatSonBookmarksHierarchyCreator creator;
        const QString text = creator.createText(store);

        WhatSonBookmarksHierarchyStore decoded;
        WhatSonBookmarksHierarchyParser parser;
        QVERIFY2(parser.parse(text, &decoded, &errorMessage), qPrintable(errorMessage));
        QCOMPARE(decoded.bookmarkIds(), store.bookmarkIds());
    }

    {
        WhatSonProjectsHierarchyStore store;
        store.setProjectNames({QStringLiteral("Project Alpha"), QStringLiteral("Project Beta")});
        WhatSonProjectsHierarchyCreator creator;
        const QString text = creator.createText(store);

        WhatSonProjectsHierarchyStore decoded;
        WhatSonProjectsHierarchyParser parser;
        QVERIFY2(parser.parse(text, &decoded, &errorMessage), qPrintable(errorMessage));
        QCOMPARE(decoded.projectNames(), store.projectNames());
        QCOMPARE(decoded.folderEntries().size(), 2);
        QCOMPARE(decoded.folderEntries().at(0).depth, 0);
    }

    {
        WhatSonProjectsHierarchyStore store;
        store.setFolderEntries({
            {QStringLiteral("Brand"), QStringLiteral("Brand"), 0},
            {QStringLiteral("Brand/Social"), QStringLiteral("Social"), 1},
            {QStringLiteral("Brand/Social/YouTube"), QStringLiteral("YouTube"), 2}
        });
        WhatSonProjectsHierarchyCreator creator;
        const QString text = creator.createText(store);

        WhatSonProjectsHierarchyStore decoded;
        WhatSonProjectsHierarchyParser parser;
        QVERIFY2(parser.parse(text, &decoded, &errorMessage), qPrintable(errorMessage));
        QCOMPARE(decoded.folderEntries().size(), 3);
        QCOMPARE(decoded.folderEntries().at(1).id, QStringLiteral("Brand/Social"));
        QCOMPARE(decoded.folderEntries().at(2).depth, 2);
        QVERIFY(decoded.projectNames().contains(QStringLiteral("YouTube")));
    }

    {
        WhatSonLibraryHierarchyStore store;
        store.setNoteIds({QStringLiteral("note-01"), QStringLiteral("note-02")});
        WhatSonLibraryHierarchyCreator creator;
        const QString text = creator.createText(store);

        WhatSonLibraryHierarchyStore decoded;
        WhatSonLibraryHierarchyParser parser;
        QVERIFY2(parser.parse(text, &decoded, &errorMessage), qPrintable(errorMessage));
        QCOMPARE(decoded.noteIds(), store.noteIds());
    }

    {
        WhatSonResourcesHierarchyStore store;
        store.setResourcePaths({QStringLiteral("assets/logo.png"), QStringLiteral("assets/ui.css")});
        WhatSonResourcesHierarchyCreator creator;
        const QString text = creator.createText(store);

        WhatSonResourcesHierarchyStore decoded;
        WhatSonResourcesHierarchyParser parser;
        QVERIFY2(parser.parse(text, &decoded, &errorMessage), qPrintable(errorMessage));
        QCOMPARE(decoded.resourcePaths(), store.resourcePaths());
    }

    {
        WhatSonEventHierarchyStore store;
        store.setEventNames({QStringLiteral("Kickoff"), QStringLiteral("Review")});
        WhatSonEventHierarchyCreator creator;
        const QString text = creator.createText(store);

        WhatSonEventHierarchyStore decoded;
        WhatSonEventHierarchyParser parser;
        QVERIFY2(parser.parse(text, &decoded, &errorMessage), qPrintable(errorMessage));
        QCOMPARE(decoded.eventNames(), store.eventNames());
    }

    {
        WhatSonPresetHierarchyStore store;
        store.setPresetNames({QStringLiteral("Executive Summary"), QStringLiteral("Launch Brief")});
        WhatSonPresetHierarchyCreator creator;
        const QString text = creator.createText(store);

        WhatSonPresetHierarchyStore decoded;
        WhatSonPresetHierarchyParser parser;
        QVERIFY2(parser.parse(text, &decoded, &errorMessage), qPrintable(errorMessage));
        QCOMPARE(decoded.presetNames(), store.presetNames());
    }

    {
        WhatSonProgressHierarchyStore store;
        store.setProgressValue(2);
        store.setProgressStates(
            {QStringLiteral("Ready"), QStringLiteral("Pending"), QStringLiteral("InProgress"), QStringLiteral("Done")});
        WhatSonProgressHierarchyCreator creator;
        const QString text = creator.createText(store);

        WhatSonProgressHierarchyStore decoded;
        WhatSonProgressHierarchyParser parser;
        QVERIFY2(parser.parse(text, &decoded, &errorMessage), qPrintable(errorMessage));
        QCOMPARE(decoded.progressValue(), store.progressValue());
        QCOMPARE(decoded.progressStates(), store.progressStates());
    }

    {
        WhatSonTagsHierarchyStore store;
        store.setTagEntries({
            {QStringLiteral("business"), QStringLiteral("Business"), 0},
            {QStringLiteral("business/brand"), QStringLiteral("Brand"), 1}
        });
        WhatSonTagsHierarchyCreator creator;
        const QString text = creator.createText(store);

        WhatSonTagsHierarchyStore decoded;
        WhatSonTagsHierarchyParser parser;
        QVERIFY2(parser.parse(text, &decoded, &errorMessage), qPrintable(errorMessage));
        QCOMPARE(decoded.tagEntries().size(), 2);
        QCOMPARE(decoded.tagEntries().at(0).id, QStringLiteral("business"));
        QCOMPARE(decoded.tagEntries().at(1).depth, 1);
    }
}

void WhatSonHierarchyIoTest::libraryParser_parsesObjectStyleNotes()
{
    QString errorMessage;

    WhatSonLibraryHierarchyParser parser;
    WhatSonLibraryHierarchyStore store;

    const QJsonObject arrayRoot{
        {
            QStringLiteral("notes"), QJsonArray{
                QJsonObject{
                    {QStringLiteral("id"), QStringLiteral("note-a")},
                    {QStringLiteral("title"), QStringLiteral("Alpha")}
                },
                QJsonObject{
                    {QStringLiteral("noteId"), QStringLiteral("note-b")},
                    {QStringLiteral("title"), QStringLiteral("Beta")}
                }
            }
        }
    };

    QVERIFY2(
        parser.parse(
            QString::fromUtf8(QJsonDocument(arrayRoot).toJson(QJsonDocument::Compact)),
            &store,
            &errorMessage),
        qPrintable(errorMessage));
    const QStringList expectedArrayStyleIds{
        QStringLiteral("note-a"),
        QStringLiteral("note-b")
    };
    QCOMPARE(store.noteIds(), expectedArrayStyleIds);

    const QJsonObject objectRoot{
        {
            QStringLiteral("notes"), QJsonObject{
                {
                    QStringLiteral("note-c"), QJsonObject{
                        {QStringLiteral("title"), QStringLiteral("Gamma")}
                    }
                },
                {QStringLiteral("note-d"), QStringLiteral("Delta")}
            }
        }
    };

    QVERIFY2(
        parser.parse(
            QString::fromUtf8(QJsonDocument(objectRoot).toJson(QJsonDocument::Compact)),
            &store,
            &errorMessage),
        qPrintable(errorMessage));
    const QStringList expectedObjectStyleIds{
        QStringLiteral("note-c"),
        QStringLiteral("note-d")
    };
    QCOMPARE(store.noteIds(), expectedObjectStyleIds);
}

void WhatSonHierarchyIoTest::bookmarksStore_hasNineHexCriteriaAndNormalizesInput()
{
    WhatSonBookmarksHierarchyStore store;

    const QStringList defaultCriteria = store.bookmarkColorCriteriaHex();
    QCOMPARE(defaultCriteria.size(), 9);
    for (const QString& hex : defaultCriteria)
    {
        QVERIFY2(hex.startsWith(QLatin1Char('#')), qPrintable(hex));
        QCOMPARE(hex.size(), 7);
    }

    store.setBookmarkColorCriteriaHex({
        QStringLiteral("blue"),
        QStringLiteral("#ec4899"),
        QStringLiteral("invalid-color"),
        QStringLiteral("blue")
    });

    const QStringList normalized = store.bookmarkColorCriteriaHex();
    QCOMPARE(normalized.size(), 9);
    QCOMPARE(normalized.at(0), QStringLiteral("#3B82F6"));
    QCOMPARE(normalized.at(1), QStringLiteral("#EC4899"));
    QCOMPARE(normalized.at(2), QStringLiteral("#F59E0B"));

    const QStringList expectedPalette{
        QStringLiteral("#EF4444"),
        QStringLiteral("#F97316"),
        QStringLiteral("#F59E0B"),
        QStringLiteral("#EAB308"),
        QStringLiteral("#22C55E"),
        QStringLiteral("#14B8A6"),
        QStringLiteral("#3B82F6"),
        QStringLiteral("#8B5CF6"),
        QStringLiteral("#EC4899")
    };
    for (const QString& hex : expectedPalette)
    {
        QVERIFY2(normalized.contains(hex), qPrintable(hex));
    }
}

QTEST_APPLESS_MAIN(WhatSonHierarchyIoTest)

#include "test_whatson_hierarchy_io.moc"

#include "hierarchy/library/LibraryHierarchyViewModel.hpp"

#include <QtTest>

class LibraryHierarchyViewModelTest final : public QObject
{
    Q_OBJECT

private
    slots  :


    void defaultState_isEmptyAndCreatable();
    void setDepthItems_parsesDepthAndDpethKeys();
    void renameItem_updatesDisplayName();
    void createFolder_insertsAsChildOfSelectedSubtree();
    void deleteSelectedFolder_removesDescendantSubtree();
};

void LibraryHierarchyViewModelTest::defaultState_isEmptyAndCreatable()
{
    LibraryHierarchyViewModel viewModel;
    QCOMPARE(viewModel.itemModel()->rowCount(), 0);
    QCOMPARE(viewModel.selectedIndex(), -1);

    viewModel.createFolder();

    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
    QCOMPARE(viewModel.selectedIndex(), 0);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Folder1"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::DepthRole).toInt(),
        0);
}

void LibraryHierarchyViewModelTest::setDepthItems_parsesDepthAndDpethKeys()
{
    LibraryHierarchyViewModel viewModel;

    const QVariantList depthArray{
        QVariantMap{
            {"label", QStringLiteral("Root")},
            {"depth", 0}
        },
        QVariantMap{
            {"label", QStringLiteral("Child")},
            {"depth", 1}
        },
        QVariantMap{
            {"label", QStringLiteral("GrandChild")},
            {"dpeth", 2}
        }
    };

    viewModel.setDepthItems(depthArray);

    QCOMPARE(viewModel.itemModel()->rowCount(), 3);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(1, 0), LibraryHierarchyModel::IndentLevelRole)
                 .toInt(),
        1);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::DepthRole).toInt(),
        2);
}

void LibraryHierarchyViewModelTest::renameItem_updatesDisplayName()
{
    LibraryHierarchyViewModel viewModel;
    viewModel.setDepthItems(QVariantList{
        QVariantMap{
            {"label", QStringLiteral("Folder1")},
            {"depth", 0}
        }
    });

    QVERIFY(viewModel.renameItem(0, QStringLiteral("Renamed Folder")));
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("Renamed Folder"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Renamed Folder"));

    QVERIFY(!viewModel.renameItem(0, QStringLiteral("   ")));
    QCOMPARE(viewModel.itemLabel(0), QStringLiteral("Renamed Folder"));
}

void LibraryHierarchyViewModelTest::createFolder_insertsAsChildOfSelectedSubtree()
{
    LibraryHierarchyViewModel viewModel;
    viewModel.setDepthItems(QVariantList{
        QVariantMap{
            {"label", QStringLiteral("Root")},
            {"depth", 0}
        },
        QVariantMap{
            {"label", QStringLiteral("RootChild")},
            {"depth", 1}
        },
        QVariantMap{
            {"label", QStringLiteral("Sibling")},
            {"depth", 0}
        }
    });
    viewModel.setSelectedIndex(0);

    viewModel.createFolder();

    QCOMPARE(viewModel.itemModel()->rowCount(), 4);
    QCOMPARE(viewModel.selectedIndex(), 2);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Folder1"));
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(2, 0), LibraryHierarchyModel::DepthRole).toInt(),
        1);
}

void LibraryHierarchyViewModelTest::deleteSelectedFolder_removesDescendantSubtree()
{
    LibraryHierarchyViewModel viewModel;
    viewModel.setDepthItems(QVariantList{
        QVariantMap{
            {"label", QStringLiteral("Root")},
            {"depth", 0}
        },
        QVariantMap{
            {"label", QStringLiteral("Child")},
            {"depth", 1}
        },
        QVariantMap{
            {"label", QStringLiteral("GrandChild")},
            {"depth", 2}
        },
        QVariantMap{
            {"label", QStringLiteral("Sibling")},
            {"depth", 0}
        }
    });
    viewModel.setSelectedIndex(0);

    viewModel.deleteSelectedFolder();

    QCOMPARE(viewModel.itemModel()->rowCount(), 1);
    QCOMPARE(viewModel.selectedIndex(), 0);
    QCOMPARE(
        viewModel.itemModel()->data(viewModel.itemModel()->index(0, 0), LibraryHierarchyModel::LabelRole).toString(),
        QStringLiteral("Sibling"));
}

QTEST_APPLESS_MAIN(LibraryHierarchyViewModelTest)

#include "test_library_hierarchy_view_model.moc"

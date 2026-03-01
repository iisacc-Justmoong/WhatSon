#include "viewmodel/hierarchy/tags/TagsHierarchyViewModel.hpp"

#include <QtTest>

class TagsHierarchyViewModelTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void defaultState_isEmpty();
    void setTagDepthEntries_mapsDepthAndChevron();
    void setSelectedIndex_clampsRange();
};

void TagsHierarchyViewModelTest::defaultState_isEmpty()
{
    TagsHierarchyViewModel viewModel;
    QCOMPARE(viewModel.itemModel()->rowCount(), 0);
    QCOMPARE(viewModel.selectedIndex(), -1);
}

void TagsHierarchyViewModelTest::setTagDepthEntries_mapsDepthAndChevron()
{
    TagsHierarchyViewModel viewModel;

    viewModel.setTagDepthEntries({
        {QStringLiteral("product"), QStringLiteral("Product"), 0},
        {QStringLiteral("critical"), QStringLiteral("Critical"), 1},
        {QStringLiteral("design"), QStringLiteral("Design"), 1},
        {QStringLiteral("marketing"), QStringLiteral("Marketing"), 0}
    });

    QCOMPARE(viewModel.itemModel()->rowCount(), 4);

    const QModelIndex rootIndex = viewModel.itemModel()->index(0, 0);
    QCOMPARE(viewModel.itemModel()->data(rootIndex, TagsHierarchyModel::LabelRole).toString(),
             QStringLiteral("Product"));
    QCOMPARE(viewModel.itemModel()->data(rootIndex, TagsHierarchyModel::DepthRole).toInt(), 0);
    QCOMPARE(viewModel.itemModel()->data(rootIndex, TagsHierarchyModel::ShowChevronRole).toBool(), true);

    const QModelIndex childIndex = viewModel.itemModel()->index(1, 0);
    QCOMPARE(viewModel.itemModel()->data(childIndex, TagsHierarchyModel::IndentLevelRole).toInt(), 1);
    QCOMPARE(viewModel.itemModel()->data(childIndex, TagsHierarchyModel::ShowChevronRole).toBool(), false);
}

void TagsHierarchyViewModelTest::setSelectedIndex_clampsRange()
{
    TagsHierarchyViewModel viewModel;
    viewModel.setTagDepthEntries({
        {QStringLiteral("root"), QStringLiteral("Root"), 0}
    });

    viewModel.setSelectedIndex(999);
    QCOMPARE(viewModel.selectedIndex(), 0);

    viewModel.setSelectedIndex(-999);
    QCOMPARE(viewModel.selectedIndex(), -1);
}

QTEST_APPLESS_MAIN(TagsHierarchyViewModelTest)

#include "test_tags_hierarchy_view_model.moc"

#include "store/sidebar/ISidebarSelectionStore.hpp"
#include "viewmodel/content/ContentsEditorSelectionBridge.hpp"
#include "viewmodel/content/ContentsGutterMarkerBridge.hpp"
#include "viewmodel/content/ContentsLogicalTextBridge.hpp"
#include "viewmodel/hierarchy/IHierarchyCapabilities.hpp"
#include "viewmodel/hierarchy/IHierarchyViewModel.hpp"
#include "viewmodel/sidebar/HierarchyViewModelProvider.hpp"
#include "viewmodel/sidebar/IHierarchyViewModelProvider.hpp"
#include "viewmodel/sidebar/SidebarHierarchyViewModel.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaProperty>
#include <QVariant>
#include <QtTest/QtTest>

#include <type_traits>

namespace
{
    QString readQml(const QString& relativePath)
    {
        const QDir testsDir(QStringLiteral(QT_TESTCASE_SOURCEDIR));
        const QString qmlRoot = testsDir.absoluteFilePath(QStringLiteral("../src/app/qml"));
        QFile file(QDir(qmlRoot).absoluteFilePath(relativePath));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return QString();
        }

        return QString::fromUtf8(file.readAll());
    }

    QString sourcePath(const QString& relativePath)
    {
        const QDir testsDir(QStringLiteral(QT_TESTCASE_SOURCEDIR));
        return testsDir.absoluteFilePath(QStringLiteral("../src/app/") + relativePath);
    }

    bool hasProperty(const QObject& object, const char* propertyName)
    {
        return object.metaObject()->indexOfProperty(propertyName) >= 0;
    }
} // namespace

class FakeSidebarSelectionStore final : public ISidebarSelectionStore
{
    Q_OBJECT

public:
    explicit FakeSidebarSelectionStore(QObject* parent = nullptr)
        : ISidebarSelectionStore(parent)
    {
    }

    int selectedHierarchyIndex() const noexcept override
    {
        return m_selectedHierarchyIndex;
    }

    void setSelectedHierarchyIndex(int index) override
    {
        if (m_selectedHierarchyIndex == index)
        {
            return;
        }

        m_selectedHierarchyIndex = index;
        emit selectedHierarchyIndexChanged();
    }

private:
    int m_selectedHierarchyIndex = 0;
};

class FakeHierarchyViewModelProvider final : public IHierarchyViewModelProvider
{
    Q_OBJECT

public:
    explicit FakeHierarchyViewModelProvider(QObject* parent = nullptr)
        : IHierarchyViewModelProvider(parent)
    {
    }

    IHierarchyViewModel* hierarchyViewModel(int hierarchyIndex) const override
    {
        switch (hierarchyIndex)
        {
        case 0:
            return m_libraryViewModel;
        case 2:
            return m_bookmarksViewModel;
        default:
            return nullptr;
        }
    }

    QObject* noteListModel(int hierarchyIndex) const override
    {
        switch (hierarchyIndex)
        {
        case 0:
            return m_libraryNoteListModel;
        case 2:
            return m_bookmarksNoteListModel;
        default:
            return nullptr;
        }
    }

    IHierarchyViewModel* m_libraryViewModel = nullptr;
    QObject* m_libraryNoteListModel = nullptr;
    IHierarchyViewModel* m_bookmarksViewModel = nullptr;
    QObject* m_bookmarksNoteListModel = nullptr;
};

class FakeHierarchyViewModel final : public IHierarchyViewModel,
                                     public IHierarchyRenameCapability,
                                     public IHierarchyCrudCapability
{
    Q_OBJECT
    Q_INTERFACES(IHierarchyRenameCapability IHierarchyCrudCapability)

public:
    explicit FakeHierarchyViewModel(QObject* parent = nullptr)
        : IHierarchyViewModel(parent)
    {
        initializeHierarchyInterfaceSignalBridge();
    }

    QObject* itemModel() noexcept override { return nullptr; }
    QObject* noteListModel() noexcept override { return m_noteListModel; }
    int selectedIndex() const noexcept override { return m_selectedIndex; }
    void setSelectedIndex(int index) override
    {
        if (m_selectedIndex == index)
        {
            return;
        }
        m_selectedIndex = index;
        emit selectedIndexChanged();
    }
    int itemCount() const noexcept override { return 0; }
    bool loadSucceeded() const noexcept override { return true; }
    QString lastLoadError() const override { return {}; }
    QVariantList hierarchyModel() const override { return {}; }
    QString itemLabel(int index) const override { Q_UNUSED(index); return {}; }
    bool canRenameItem(int index) const override { Q_UNUSED(index); return false; }
    bool renameItem(int index, const QString& displayName) override
    {
        Q_UNUSED(index);
        Q_UNUSED(displayName);
        return false;
    }
    void createFolder() override {}
    void deleteSelectedFolder() override {}
    bool renameEnabled() const noexcept override { return false; }
    bool createFolderEnabled() const noexcept override { return false; }
    bool deleteFolderEnabled() const noexcept override { return false; }

    QObject* m_noteListModel = nullptr;

signals:
    void selectedIndexChanged();
    void hierarchyModelChanged();
    void itemCountChanged();
    void loadStateChanged();

private:
    int m_selectedIndex = -1;
};

static_assert(std::is_base_of_v<IHierarchyViewModelProvider, HierarchyViewModelProvider>);

class SolidArchitectureContractsTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void sidebarState_mustStaySingleSourcedAndInterfaceDriven();
    void editorAdapters_mustStayResponsibilitySeparated();
    void qmlAssembly_mustKeepDedicatedResponsibilityObjects();
};

void SolidArchitectureContractsTest::sidebarState_mustStaySingleSourcedAndInterfaceDriven()
{
    FakeSidebarSelectionStore selectionStore;
    FakeHierarchyViewModelProvider provider;
    FakeHierarchyViewModel libraryViewModel;
    QObject libraryNoteListModel;
    FakeHierarchyViewModel bookmarksViewModel;
    QObject bookmarksNoteListModel;

    provider.m_libraryViewModel = &libraryViewModel;
    provider.m_libraryNoteListModel = &libraryNoteListModel;
    provider.m_bookmarksViewModel = &bookmarksViewModel;
    provider.m_bookmarksNoteListModel = &bookmarksNoteListModel;
    libraryViewModel.m_noteListModel = &libraryNoteListModel;
    bookmarksViewModel.m_noteListModel = &bookmarksNoteListModel;

    SidebarHierarchyViewModel sidebarViewModel;
    sidebarViewModel.setSelectionStore(&selectionStore);
    sidebarViewModel.setViewModelProvider(&provider);

    QCOMPARE(sidebarViewModel.selectionStore(), static_cast<ISidebarSelectionStore*>(&selectionStore));
    QCOMPARE(sidebarViewModel.viewModelProvider(), static_cast<IHierarchyViewModelProvider*>(&provider));

    sidebarViewModel.setActiveHierarchyIndex(2);
    QCOMPARE(selectionStore.selectedHierarchyIndex(), 2);
    QCOMPARE(sidebarViewModel.activeHierarchyIndex(), 2);
    QCOMPARE(sidebarViewModel.activeHierarchyViewModel(), &bookmarksViewModel);
    QCOMPARE(sidebarViewModel.activeNoteListModel(), &bookmarksNoteListModel);

    selectionStore.setSelectedHierarchyIndex(0);
    QCOMPARE(sidebarViewModel.activeHierarchyIndex(), 0);
    QCOMPARE(sidebarViewModel.activeHierarchyViewModel(), &libraryViewModel);
    QCOMPARE(sidebarViewModel.activeNoteListModel(), &libraryNoteListModel);

    const QString bodyLayout = readQml(QStringLiteral("view/panels/BodyLayout.qml"));
    const QString hierarchySidebarLayout = readQml(QStringLiteral("view/panels/HierarchySidebarLayout.qml"));
    QVERIFY(!bodyLayout.isEmpty());
    QVERIFY(!hierarchySidebarLayout.isEmpty());
    QVERIFY(!bodyLayout.contains(QStringLiteral("function resolveActiveHierarchyIndex")));
    QVERIFY(!bodyLayout.contains(QStringLiteral("function resolveHierarchyViewModel")));
    QVERIFY(!bodyLayout.contains(QStringLiteral("function resolveNoteListModel")));
    QVERIFY(!hierarchySidebarLayout.contains(QStringLiteral("function resolveActiveHierarchyIndex")));
    QVERIFY(!hierarchySidebarLayout.contains(QStringLiteral("function resolveHierarchyViewModel")));
    QVERIFY(!hierarchySidebarLayout.contains(QStringLiteral("function resolveNoteListModel")));
    QVERIFY(bodyLayout.contains(QStringLiteral(
        "readonly property int activeHierarchyIndex: hStack.sidebarHierarchyViewModel ? hStack.sidebarHierarchyViewModel.resolvedActiveHierarchyIndex : 0")));
    QVERIFY(hierarchySidebarLayout.contains(QStringLiteral(
        "readonly property string resolvedHierarchyViewModelKey: hierarchyView.hierarchyViewModelKeyForIndex(hierarchyView.currentHierarchy)")));
    QVERIFY(hierarchySidebarLayout.contains(QStringLiteral("const activeHierarchyViewModel = hierarchyView.sidebarHierarchyViewModel")));
    QVERIFY(hierarchySidebarLayout.contains(QStringLiteral("LV.ViewModels.getForView(hierarchyView.hierarchyViewId)")));
}

void SolidArchitectureContractsTest::editorAdapters_mustStayResponsibilitySeparated()
{
    ContentsEditorSelectionBridge selectionBridge;
    ContentsLogicalTextBridge logicalTextBridge;
    ContentsGutterMarkerBridge gutterMarkerBridge;

    QVERIFY(hasProperty(selectionBridge, "noteListModel"));
    QVERIFY(hasProperty(selectionBridge, "contentViewModel"));
    QVERIFY(hasProperty(selectionBridge, "selectedNoteId"));
    QVERIFY(hasProperty(selectionBridge, "selectedNoteBodyText"));
    QVERIFY(hasProperty(selectionBridge, "visibleNoteCount"));
    QVERIFY(!hasProperty(selectionBridge, "text"));
    QVERIFY(!hasProperty(selectionBridge, "logicalLineStartOffsets"));
    QVERIFY(!hasProperty(selectionBridge, "normalizedExternalGutterMarkers"));

    QVERIFY(hasProperty(logicalTextBridge, "text"));
    QVERIFY(hasProperty(logicalTextBridge, "logicalLineStartOffsets"));
    QVERIFY(hasProperty(logicalTextBridge, "logicalLineCount"));
    QVERIFY(!hasProperty(logicalTextBridge, "noteListModel"));
    QVERIFY(!hasProperty(logicalTextBridge, "contentViewModel"));
    QVERIFY(!hasProperty(logicalTextBridge, "gutterMarkers"));

    QVERIFY(hasProperty(gutterMarkerBridge, "gutterMarkers"));
    QVERIFY(hasProperty(gutterMarkerBridge, "normalizedExternalGutterMarkers"));
    QVERIFY(!hasProperty(gutterMarkerBridge, "text"));
    QVERIFY(!hasProperty(gutterMarkerBridge, "selectedNoteId"));
    QVERIFY(!hasProperty(gutterMarkerBridge, "visibleNoteCount"));

    QVERIFY(!QFileInfo::exists(sourcePath(QStringLiteral("viewmodel/content/ContentsEditorBridge.hpp"))));
    QVERIFY(!QFileInfo::exists(sourcePath(QStringLiteral("viewmodel/content/ContentsEditorBridge.cpp"))));
}

void SolidArchitectureContractsTest::qmlAssembly_mustKeepDedicatedResponsibilityObjects()
{
    const QString mainQml = readQml(QStringLiteral("Main.qml"));
    const QString bodyLayout = readQml(QStringLiteral("view/panels/BodyLayout.qml"));
    const QString hierarchySidebarLayout = readQml(QStringLiteral("view/panels/HierarchySidebarLayout.qml"));
    const QString sidebarHierarchyView = readQml(QStringLiteral("view/panels/sidebar/SidebarHierarchyView.qml"));
    const QString contentsDisplayView = readQml(QStringLiteral("view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!mainQml.isEmpty());
    QVERIFY(!bodyLayout.isEmpty());
    QVERIFY(!hierarchySidebarLayout.isEmpty());
    QVERIFY(!sidebarHierarchyView.isEmpty());
    QVERIFY(!contentsDisplayView.isEmpty());

    QVERIFY(mainQml.contains(QStringLiteral("MainWindowInteractionController {")));
    QVERIFY(bodyLayout.contains(QStringLiteral("PanelEdgeSplitter {")));
    QVERIFY(hierarchySidebarLayout.contains(QStringLiteral("HierarchyDragDropBridge {")));
    QVERIFY(hierarchySidebarLayout.contains(QStringLiteral("HierarchyInteractionBridge {")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("LV.Hierarchy {")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("onListItemMoved: function")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("SidebarHierarchyRenameController {")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("SidebarHierarchyNoteDropController {")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("SidebarHierarchyBookmarkPaletteController {")));
    QVERIFY(!sidebarHierarchyView.contains(QStringLiteral("SidebarHierarchyLvrsAdapter {")));
    QVERIFY(!sidebarHierarchyView.contains(QStringLiteral("SidebarHierarchyInteractionController {")));
    QVERIFY(contentsDisplayView.contains(QStringLiteral("ContentsEditorSelectionBridge {")));
    QVERIFY(contentsDisplayView.contains(QStringLiteral("ContentsLogicalTextBridge {")));
    QVERIFY(contentsDisplayView.contains(QStringLiteral("ContentsGutterMarkerBridge {")));
    QVERIFY(contentsDisplayView.contains(QStringLiteral("ContentsEditorSession {")));
    QVERIFY(!contentsDisplayView.contains(QStringLiteral("ContentsEditorBridge {")));
}

QTEST_APPLESS_MAIN(SolidArchitectureContractsTest)

#include "test_solid_architecture_contracts.moc"

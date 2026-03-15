#include "store/sidebar/ISidebarSelectionStore.hpp"
#include "viewmodel/content/ContentsEditorSelectionBridge.hpp"
#include "viewmodel/content/ContentsGutterMarkerBridge.hpp"
#include "viewmodel/content/ContentsLogicalTextBridge.hpp"
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

    QObject* hierarchyViewModel(int hierarchyIndex) const override
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

    QObject* m_libraryViewModel = nullptr;
    QObject* m_libraryNoteListModel = nullptr;
    QObject* m_bookmarksViewModel = nullptr;
    QObject* m_bookmarksNoteListModel = nullptr;
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
    QObject libraryViewModel;
    QObject libraryNoteListModel;
    QObject bookmarksViewModel;
    QObject bookmarksNoteListModel;

    provider.m_libraryViewModel = &libraryViewModel;
    provider.m_libraryNoteListModel = &libraryNoteListModel;
    provider.m_bookmarksViewModel = &bookmarksViewModel;
    provider.m_bookmarksNoteListModel = &bookmarksNoteListModel;

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
        "readonly property int activeHierarchyIndex: hStack.sidebarHierarchyViewModel.resolvedActiveHierarchyIndex")));
    QVERIFY(hierarchySidebarLayout.contains(QStringLiteral(
        "readonly property var resolvedHierarchyViewModel: hierarchyView.sidebarHierarchyViewModel.resolvedHierarchyViewModel")));
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
    const QString sidebarHierarchyView = readQml(QStringLiteral("view/panels/sidebar/SidebarHierarchyView.qml"));
    const QString contentsDisplayView = readQml(QStringLiteral("view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!mainQml.isEmpty());
    QVERIFY(!bodyLayout.isEmpty());
    QVERIFY(!sidebarHierarchyView.isEmpty());
    QVERIFY(!contentsDisplayView.isEmpty());

    QVERIFY(mainQml.contains(QStringLiteral("MainWindowInteractionController {")));
    QVERIFY(bodyLayout.contains(QStringLiteral("PanelEdgeSplitter {")));
    QVERIFY(sidebarHierarchyView.contains(QStringLiteral("SidebarHierarchyInteractionController {")));
    QVERIFY(contentsDisplayView.contains(QStringLiteral("ContentsEditorSelectionBridge {")));
    QVERIFY(contentsDisplayView.contains(QStringLiteral("ContentsLogicalTextBridge {")));
    QVERIFY(contentsDisplayView.contains(QStringLiteral("ContentsGutterMarkerBridge {")));
    QVERIFY(contentsDisplayView.contains(QStringLiteral("ContentsEditorSession {")));
    QVERIFY(!contentsDisplayView.contains(QStringLiteral("ContentsEditorBridge {")));
}

QTEST_APPLESS_MAIN(SolidArchitectureContractsTest)

#include "test_solid_architecture_contracts.moc"

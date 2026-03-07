#include "viewmodel/panel/PanelViewModelRegistry.hpp"

#include <QDir>
#include <QFile>
#include <QtTest>

namespace
{
    QString readQml(const QString& relativePath)
    {
        const QDir testsDir(QStringLiteral(QT_TESTCASE_SOURCEDIR));
        const QString qmlRoot = testsDir.absoluteFilePath(QStringLiteral("../src/app/qml"));
        const QString qmlPath = QDir(qmlRoot).absoluteFilePath(relativePath);
        QFile file(qmlPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return QString();
        }

        return QString::fromUtf8(file.readAll());
    }
} // namespace

class NavigationQmlFramesTest final : public QObject
{
    Q_OBJECT

private
    slots  :



    void navigationPanels_mustExposeFrameScopedPanelKeys();
    void mainQml_mustBindTabShortcutForNavigationModeCycling();
    void navigationBar_mustComposePropertiesFrame();
    void navigationBar_mustSwitchApplicationBarsByNavigationMode();
    void navigationPropertiesFrame_mustComposeSeparatedChildFrames();
    void navigationChildFrames_mustBindPanelViewModelContracts();
};

void NavigationQmlFramesTest::navigationPanels_mustExposeFrameScopedPanelKeys()
{
    PanelViewModelRegistry registry;

    QVERIFY(registry.containsPanel(QStringLiteral("navigation.NavigationPropertiesBar")));
    QVERIFY(registry.containsPanel(QStringLiteral("navigation.NavigationInformationBar")));
    QVERIFY(registry.containsPanel(QStringLiteral("navigation.NavigationModeBar")));
    QVERIFY(registry.containsPanel(QStringLiteral("navigation.NavigationEditorViewBar")));
    QVERIFY(registry.containsPanel(QStringLiteral("navigation.NavigationApplicationViewBar")));
    QVERIFY(registry.containsPanel(QStringLiteral("navigation.NavigationApplicationEditBar")));
    QVERIFY(registry.containsPanel(QStringLiteral("navigation.NavigationApplicationControlBar")));
    QVERIFY(registry.containsPanel(QStringLiteral("navigation.NavigationApplicationPresentationBar")));
}

void NavigationQmlFramesTest::mainQml_mustBindTabShortcutForNavigationModeCycling()
{
    const QString mainQml = readQml(QStringLiteral("Main.qml"));

    QVERIFY(!mainQml.isEmpty());
    QVERIFY(mainQml.contains(QStringLiteral("function hasFocusedTextInput()")));
    QVERIFY(mainQml.contains(QStringLiteral("current.text !== undefined")));
    QVERIFY(mainQml.contains(QStringLiteral("current.cursorPosition !== undefined")));
    QVERIFY(mainQml.contains(QStringLiteral("current.selectedText !== undefined")));
    QVERIFY(mainQml.contains(QStringLiteral("function cycleNavigationModeFromShortcut()")));
    QVERIFY(mainQml.contains(QStringLiteral("sequence: \"Tab\"")));
    QVERIFY(mainQml.contains(QStringLiteral("context: Qt.ApplicationShortcut")));
    QVERIFY(mainQml.contains(QStringLiteral("enabled: !applicationWindow.hasFocusedTextInput()")));
    QVERIFY(mainQml.contains(QStringLiteral("applicationWindow.navigationModeVm.requestNextMode();")));
}

void NavigationQmlFramesTest::navigationBar_mustComposePropertiesFrame()
{
    const QString navigationBarLayout = readQml(QStringLiteral("view/panels/NavigationBarLayout.qml"));

    QVERIFY(!navigationBarLayout.isEmpty());
    QVERIFY(navigationBarLayout.contains(QStringLiteral("NavigationView.NavigationPropertiesBar {")));
}

void NavigationQmlFramesTest::navigationBar_mustSwitchApplicationBarsByNavigationMode()
{
    const QString navigationBarLayout = readQml(QStringLiteral("view/panels/NavigationBarLayout.qml"));

    QVERIFY(!navigationBarLayout.isEmpty());
    QVERIFY(navigationBarLayout.contains(QStringLiteral("readonly property string activeNavigationModeName")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("NavigationView.NavigationApplicationViewBar {")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("NavigationView.NavigationApplicationEditBar {")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("NavigationView.NavigationApplicationControlBar {")));
    QVERIFY(navigationBarLayout.contains(QStringLiteral("NavigationView.NavigationApplicationPresentationBar {")));
}

void NavigationQmlFramesTest::navigationPropertiesFrame_mustComposeSeparatedChildFrames()
{
    const QString propertiesBar = readQml(QStringLiteral("view/panels/navigation/NavigationPropertiesBar.qml"));

    QVERIFY(!propertiesBar.isEmpty());
    QVERIFY(propertiesBar.contains(QStringLiteral("NavigationInformationBar {")));
    QVERIFY(propertiesBar.contains(QStringLiteral("NavigationModeBar {")));
    QVERIFY(propertiesBar.contains(QStringLiteral("NavigationEditorViewBar {")));
}

void NavigationQmlFramesTest::navigationChildFrames_mustBindPanelViewModelContracts()
{
    const QString propertiesBar = readQml(QStringLiteral("view/panels/navigation/NavigationPropertiesBar.qml"));
    const QString informationBar = readQml(QStringLiteral("view/panels/navigation/NavigationInformationBar.qml"));
    const QString modeBar = readQml(QStringLiteral("view/panels/navigation/NavigationModeBar.qml"));
    const QString editorViewBar = readQml(QStringLiteral("view/panels/navigation/NavigationEditorViewBar.qml"));
    const QString applicationViewBar = readQml(
        QStringLiteral("view/panels/navigation/NavigationApplicationViewBar.qml"));
    const QString applicationEditBar = readQml(
        QStringLiteral("view/panels/navigation/NavigationApplicationEditBar.qml"));
    const QString applicationControlBar = readQml(
        QStringLiteral("view/panels/navigation/NavigationApplicationControlBar.qml"));
    const QString applicationPresentationBar = readQml(
        QStringLiteral("view/panels/navigation/NavigationApplicationPresentationBar.qml"));

    QVERIFY(propertiesBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationPropertiesBar\")")));
    QVERIFY(informationBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationInformationBar\")")));
    QVERIFY(modeBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationModeBar\")")));
    QVERIFY(editorViewBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationEditorViewBar\")")));
    QVERIFY(applicationViewBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationApplicationViewBar\")")));
    QVERIFY(applicationEditBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationApplicationEditBar\")")));
    QVERIFY(applicationControlBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationApplicationControlBar\")")));
    QVERIFY(applicationPresentationBar.contains(QStringLiteral(
        "panelViewModelRegistry.panelViewModel(\"navigation.NavigationApplicationPresentationBar\")")));
}

QTEST_APPLESS_MAIN(NavigationQmlFramesTest)

#include "test_navigation_qml_frames.moc"

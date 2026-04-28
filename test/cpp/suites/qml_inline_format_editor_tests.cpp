#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>

#include <memory>

namespace
{
    QString qmlInlineFormatEditorRepositoryRootPath()
    {
        QDir repositoryRoot(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
        repositoryRoot.cdUp();
        repositoryRoot.cdUp();
        repositoryRoot.cdUp();
        return repositoryRoot.absolutePath();
    }

    QString qmlInlineFormatEditorErrorString(const QList<QQmlError>& errors)
    {
        QStringList messages;
        messages.reserve(errors.size());
        for (const QQmlError& error : errors)
        {
            messages.push_back(error.toString());
        }
        return messages.join(QLatin1Char('\n'));
    }

    void addWhatSonInlineFormatEditorQmlImportPaths(QQmlEngine& engine, const QString& repositoryRoot)
    {
        const QStringList candidatePaths{
            repositoryRoot + QStringLiteral("/src/app/qml"),
            repositoryRoot + QStringLiteral("/build/src/app"),
            repositoryRoot + QStringLiteral("/build/src/app/cmake/runtime/lvrs_runtime_qml"),
            repositoryRoot + QStringLiteral("/build/src/app/lvrs_runtime_qml"),
        };
        for (const QString& candidatePath : candidatePaths)
        {
            if (QFileInfo::exists(candidatePath))
            {
                engine.addImportPath(candidatePath);
            }
        }
    }

} // namespace

void WhatSonCppRegressionTests::qmlInlineFormatEditor_keepsNativeTextEditInputUncovered()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("nativeTouchScrollGuardActive")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("MouseArea {")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("TapHandler {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Keys.onPressed: function (event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("key !== Qt.Key_Backspace")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function eventRequestsPasteShortcut(event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function eventRequestsInlineFormatShortcut(event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("event.matches(StandardKey.Paste)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("!control.eventRequestsPasteShortcut(event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("!control.eventRequestsInlineFormatShortcut(event)")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("property var shortcutKeyPressHandler")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("property var modifierVerticalNavigationHandler")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("handleMacModifierVerticalNavigation")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("activateInputAtPoint")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("insertTabIndentAsSpaces")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("activeFocusOnPress: control.autoFocusOnPress")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property int inputMethodHints: Qt.ImhNone")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property int mouseSelectionMode: TextEdit.SelectCharacters")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property bool overwriteMode: false")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property bool persistentSelection: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("selectByMouse: control.selectByMouse")));
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_forwardsInlineFormatShortcutsToTagManagementHook()
{
    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonInlineFormatEditorQmlImportPaths(engine, repositoryRoot);

    const QString editorImportUrl =
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/src/app/qml/view/content/editor")).toString();
    const QByteArray qmlSource = QStringLiteral(R"QML(
import QtQuick
import "%1" as EditorView

Item {
    id: root
    objectName: "inlineFormatShortcutHarness"
    property bool handled: false
    property int handledKey: -1
    property int handledModifiers: 0
    width: 360
    height: 96

    EditorView.ContentsInlineFormatEditor {
        id: editor
        objectName: "inlineFormatEditorUnderTest"
        anchors.fill: parent
        text: "Alpha beta"
        tagManagementKeyPressHandler: function (event) {
            root.handled = true;
            root.handledKey = Number(event.key) || -1;
            root.handledModifiers = Number(event.modifiers) || 0;
            event.accepted = true;
            return true;
        }
    }
}
)QML").arg(editorImportUrl).toUtf8();

    QQmlComponent component(&engine);
    component.setData(
        qmlSource,
        QUrl::fromLocalFile(repositoryRoot + QStringLiteral("/test/cpp/InlineFormatShortcutHarness.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    std::unique_ptr<QObject> rootObject(component.create());
    if (!rootObject)
    {
        QFAIL(qPrintable(qmlInlineFormatEditorErrorString(component.errors())));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(rootObject.get());
    QVERIFY(rootItem != nullptr);

    QQuickWindow window;
    window.resize(360, 96);
    rootItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* inlineEditor = rootObject->findChild<QObject*>(QStringLiteral("inlineFormatEditorUnderTest"));
    QVERIFY(inlineEditor != nullptr);
    QVERIFY(QMetaObject::invokeMethod(inlineEditor, "forceActiveFocus"));
    QTRY_VERIFY(inlineEditor->property("focused").toBool());

    QTest::keyClick(&window, Qt::Key_B, Qt::ControlModifier);

    QTRY_VERIFY(rootObject->property("handled").toBool());
    QCOMPARE(rootObject->property("handledKey").toInt(), static_cast<int>(Qt::Key_B));
    QVERIFY(rootObject->property("handledModifiers").toInt() & static_cast<int>(Qt::ControlModifier));
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_keepsKeyboardSelectionAndOsImeNative()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"));
    const QString inlineEditorControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsInlineFormatEditorController.qml"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString eventPumpSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayEventPump.qml"));
    const QString typingControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsEditorTypingController.qml"));
    const QString surfaceGuardSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/resource/ContentsEditorSurfaceGuardController.qml"));
    const QString resourceImportControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/resource/ContentsResourceImportController.qml"));

    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(!inlineEditorControllerSource.isEmpty());
    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!eventPumpSource.isEmpty());
    QVERIFY(!typingControllerSource.isEmpty());
    QVERIFY(!surfaceGuardSource.isEmpty());
    QVERIFY(!resourceImportControllerSource.isEmpty());
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property bool selectByKeyboard: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("inputMethodHints: control.inputMethodHints")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("mouseSelectionMode: control.mouseSelectionMode")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("overwriteMode: control.overwriteMode")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("persistentSelection: control.persistentSelection")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("selectByKeyboard: control.selectByKeyboard")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("function onSelectionEndChanged() {")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("function onSelectionStartChanged() {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool inputMethodComposing: textInput.inputMethodComposing")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property string preeditText: textInput.preeditText")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function nativeCompositionActive()")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("function shouldRejectFocusedProgrammaticTextSync(nextText)")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("property bool localSelectionInteractionSinceFocus: false")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("property bool localTextEditSinceFocus: false")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("const hadLocalTextEditSinceFocus = controller.localTextEditSinceFocus;")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("controller.localSelectionInteractionSinceFocus = true;")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("controller.localSelectionInteractionSinceFocus = false;")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("if (controller.control && controller.control.preferNativeInputHandling && hadLocalTextEditSinceFocus)\n                        controller.clearDeferredProgrammaticText();")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("function setCursorPositionPreservingNativeInput(position)")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("if (!controller.textInput || controller.nativeCompositionActive())\n            return false;")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Keys.onPressed: function (event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("key !== Qt.Key_Backspace")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.eventRequestsPasteShortcut(event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.eventRequestsInlineFormatShortcut(event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("event.matches(StandardKey.Paste)")));
    QVERIFY(typingControllerSource.contains(QStringLiteral("property bool pendingCursorPositionRequest: false")));
    QVERIFY(typingControllerSource.contains(QStringLiteral("function applyPendingCursorPositionIfInputSettled()")));
    QVERIFY(typingControllerSource.contains(QStringLiteral("if (controller.nativeCompositionActive()) {\n                controller.pendingCursorPosition = targetPosition;")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("retryCount < 6")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("scheduleCursorPosition(targetPosition, retryCount + 1)")));
    QVERIFY(surfaceGuardSource.contains(QStringLiteral("property bool pendingEditorSurfaceRestore: false")));
    QVERIFY(surfaceGuardSource.contains(QStringLiteral("function restorePendingEditorSurfaceFromPresentationIfInputSettled()")));
    QVERIFY(surfaceGuardSource.contains(QStringLiteral("shouldRejectFocusedProgrammaticTextSync(nextSurfaceText)")));
    QVERIFY(resourceImportControllerSource.contains(QStringLiteral("function restorePendingEditorSurfaceFromPresentationIfInputSettled()")));
    QVERIFY(!eventPumpSource.contains(QStringLiteral("function onInputMethodComposingChanged()")));
    QVERIFY(!eventPumpSource.contains(QStringLiteral("function onPreeditTextChanged()")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("Qt.inputMethod")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("InputMethod.")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("notifyInputMethod")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("notifyQInputMethod")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("inputMethodVisible")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("Qt.inputMethod &&")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("Qt.inputMethod.visible !== undefined")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("Qt.ImQuery")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("setCursorPositionPreservingInputMethod")));

    const QDir repositoryRoot(qmlInlineFormatEditorRepositoryRootPath());
    QVERIFY(repositoryRoot.exists());

    const QStringList forbiddenInputMethodPatterns{
        QStringLiteral("Qt.inputMethod"),
        QStringLiteral("InputMethod."),
        QStringLiteral("notifyInputMethod"),
        QStringLiteral("notifyQInputMethod"),
        QStringLiteral("inputMethodVisible"),
        QStringLiteral("Qt.inputMethod &&"),
        QStringLiteral("Qt.inputMethod.visible !== undefined"),
        QStringLiteral("Qt.ImQuery"),
    };
    const QStringList qmlRoots{
        QStringLiteral("src/app/qml"),
        QStringLiteral("src/app/models/editor"),
        QStringLiteral("src/app/viewmodel/editor"),
    };
    for (const QString& qmlRoot : qmlRoots)
    {
        QDirIterator qmlFiles(
            repositoryRoot.filePath(qmlRoot),
            QStringList{QStringLiteral("*.qml")},
            QDir::Files | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);
        while (qmlFiles.hasNext())
        {
            const QString absolutePath = qmlFiles.next();
            QFile sourceFile(absolutePath);
            QVERIFY2(sourceFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(absolutePath));
            const QString qmlSource = QString::fromUtf8(sourceFile.readAll());
            for (const QString& forbiddenPattern : forbiddenInputMethodPatterns)
            {
                QVERIFY2(
                    !qmlSource.contains(forbiddenPattern),
                    qPrintable(QStringLiteral("%1 contains forbidden input-method pattern: %2")
                                   .arg(repositoryRoot.relativeFilePath(absolutePath), forbiddenPattern)));
            }
        }
    }
}

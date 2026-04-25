#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QDirIterator>

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
} // namespace

void WhatSonCppRegressionTests::qmlInlineFormatEditor_keepsNativeTextEditInputUncovered()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"));

    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("nativeTouchScrollGuardActive")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("MouseArea {")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("TapHandler {")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("Keys.onPressed")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("property var shortcutKeyPressHandler")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("property var modifierVerticalNavigationHandler")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("handleMacModifierVerticalNavigation")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("activateInputAtPoint")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("insertTabIndentAsSpaces")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("activeFocusOnPress: control.autoFocusOnPress")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("selectByMouse: control.selectByMouse")));
}

void WhatSonCppRegressionTests::qmlInlineFormatEditor_keepsKeyboardSelectionAndOsImeNative()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString typingControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsEditorTypingController.qml"));
    const QString surfaceGuardSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsEditorSurfaceGuardController.qml"));
    const QString resourceImportControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsResourceImportController.qml"));

    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!typingControllerSource.isEmpty());
    QVERIFY(!surfaceGuardSource.isEmpty());
    QVERIFY(!resourceImportControllerSource.isEmpty());
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property bool selectByKeyboard: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("selectByKeyboard: control.selectByKeyboard")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("persistentSelection: true")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("onSelectionEndChanged: {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("onSelectionStartChanged: {")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property bool inputMethodComposing: textInput.inputMethodComposing")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("readonly property string preeditText: textInput.preeditText")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function nativeCompositionActive()")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function shouldRejectFocusedProgrammaticTextSync(nextText)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("property bool _localTextEditSinceFocus: false")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("const hadLocalTextEditSinceFocus = control._localTextEditSinceFocus;")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("if (control.preferNativeInputHandling && hadLocalTextEditSinceFocus)\n                control.clearDeferredProgrammaticText();")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function setCursorPositionPreservingNativeInput(position)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("if (control.nativeCompositionActive())\n            return false;")));
    QVERIFY(typingControllerSource.contains(QStringLiteral("property bool pendingCursorPositionRequest: false")));
    QVERIFY(typingControllerSource.contains(QStringLiteral("function applyPendingCursorPositionIfInputSettled()")));
    QVERIFY(typingControllerSource.contains(QStringLiteral("if (controller.nativeCompositionActive()) {\n                controller.pendingCursorPosition = targetPosition;")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("retryCount < 6")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("scheduleCursorPosition(targetPosition, retryCount + 1)")));
    QVERIFY(surfaceGuardSource.contains(QStringLiteral("property bool pendingEditorSurfaceRestore: false")));
    QVERIFY(surfaceGuardSource.contains(QStringLiteral("function restorePendingEditorSurfaceFromPresentationIfInputSettled()")));
    QVERIFY(surfaceGuardSource.contains(QStringLiteral("shouldRejectFocusedProgrammaticTextSync(nextSurfaceText)")));
    QVERIFY(resourceImportControllerSource.contains(QStringLiteral("function restorePendingEditorSurfaceFromPresentationIfInputSettled()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function onInputMethodComposingChanged()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("editorTypingController.applyPendingCursorPositionIfInputSettled();")));
    QVERIFY(displayViewSource.contains(QStringLiteral("resourceImportController.restorePendingEditorSurfaceFromPresentationIfInputSettled();")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function onPreeditTextChanged()")));
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
    QDirIterator qmlFiles(
        repositoryRoot.filePath(QStringLiteral("src/app/qml")),
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

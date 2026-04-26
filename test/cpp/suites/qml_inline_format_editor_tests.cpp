#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QDirIterator>
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

    QString qmlErrorString(const QList<QQmlError>& errors)
    {
        QStringList messages;
        messages.reserve(errors.size());
        for (const QQmlError& error : errors)
        {
            messages.push_back(error.toString());
        }
        return messages.join(QLatin1Char('\n'));
    }

    void addWhatSonQmlImportPaths(QQmlEngine& engine, const QString& repositoryRoot)
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

    QObject* objectProperty(QObject* object, const char* propertyName)
    {
        if (object == nullptr)
        {
            return nullptr;
        }
        return qvariant_cast<QObject*>(object->property(propertyName));
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
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Keys.priority: Keys.BeforeItem")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("Keys.priority: Keys.AfterItem")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Keys.onPressed: function (event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("control.handleMacOsOptionWordNavigation(event);")));
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

void WhatSonCppRegressionTests::qmlInlineFormatEditor_keepsKeyboardSelectionAndOsImeNative()
{
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString eventPumpSource = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplayEventPump.qml"));
    const QString typingControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsEditorTypingController.qml"));
    const QString surfaceGuardSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/resource/ContentsEditorSurfaceGuardController.qml"));
    const QString resourceImportControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/resource/ContentsResourceImportController.qml"));

    QVERIFY(!inlineEditorSource.isEmpty());
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
    QVERIFY(inlineEditorSource.contains(QStringLiteral("function handleMacOsOptionWordNavigation(event)")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Qt.platform.os !== \"osx\"")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("Keys.priority: Keys.BeforeItem")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("(modifiers & (Qt.AltModifier | Qt.ShiftModifier)) !== modifiers")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("textInput.moveCursorSelection(target, TextEdit.SelectCharacters);")));
    QVERIFY(inlineEditorSource.contains(QStringLiteral("event.accepted = true;")));
    QVERIFY(typingControllerSource.contains(QStringLiteral("property bool pendingCursorPositionRequest: false")));
    QVERIFY(typingControllerSource.contains(QStringLiteral("function applyPendingCursorPositionIfInputSettled()")));
    QVERIFY(typingControllerSource.contains(QStringLiteral("if (controller.nativeCompositionActive()) {\n                controller.pendingCursorPosition = targetPosition;")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("retryCount < 6")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("scheduleCursorPosition(targetPosition, retryCount + 1)")));
    QVERIFY(surfaceGuardSource.contains(QStringLiteral("property bool pendingEditorSurfaceRestore: false")));
    QVERIFY(surfaceGuardSource.contains(QStringLiteral("function restorePendingEditorSurfaceFromPresentationIfInputSettled()")));
    QVERIFY(surfaceGuardSource.contains(QStringLiteral("shouldRejectFocusedProgrammaticTextSync(nextSurfaceText)")));
    QVERIFY(resourceImportControllerSource.contains(QStringLiteral("function restorePendingEditorSurfaceFromPresentationIfInputSettled()")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("function onInputMethodComposingChanged()")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("editorTypingController.applyPendingCursorPositionIfInputSettled();")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("resourceImportController.restorePendingEditorSurfaceFromPresentationIfInputSettled();")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("function onPreeditTextChanged()")));
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

void WhatSonCppRegressionTests::qmlInlineFormatEditor_handlesMacOptionWordNavigationAtRuntime()
{
#ifndef Q_OS_MACOS
    QSKIP("macOS Option word navigation is platform-specific.");
#else
    const QString repositoryRoot = qmlInlineFormatEditorRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonQmlImportPaths(engine, repositoryRoot);

    QQmlComponent component(
        &engine,
        QUrl::fromLocalFile(
            repositoryRoot
            + QStringLiteral("/src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlErrorString(component.errors())));
    }

    std::unique_ptr<QObject> editorObject(component.create());
    if (!editorObject)
    {
        QFAIL(qPrintable(qmlErrorString(component.errors())));
    }

    auto* editorItem = qobject_cast<QQuickItem*>(editorObject.get());
    QVERIFY(editorItem != nullptr);
    editorItem->setWidth(420);
    editorItem->setHeight(80);
    editorObject->setProperty("fieldMinHeight", 80);
    editorObject->setProperty("showRenderedOutput", false);
    editorObject->setProperty("text", QStringLiteral("alpha beta gamma"));

    QQuickWindow window;
    window.resize(420, 80);
    editorItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QObject* editorShell = objectProperty(editorObject.get(), "editorItem");
    QVERIFY(editorShell != nullptr);
    QObject* inputItem = objectProperty(editorShell, "inputItem");
    QVERIFY(inputItem != nullptr);
    QTRY_COMPARE(inputItem->property("text").toString(), QStringLiteral("alpha beta gamma"));

    QVERIFY(QMetaObject::invokeMethod(inputItem, "forceActiveFocus"));
    QTRY_VERIFY(inputItem->property("activeFocus").toBool());

    inputItem->setProperty("cursorPosition", 0);
    QTest::keyClick(&window, Qt::Key_Right, Qt::AltModifier);
    QTRY_COMPARE(inputItem->property("cursorPosition").toInt(), 5);
    QCOMPARE(inputItem->property("selectionStart").toInt(), 5);
    QCOMPARE(inputItem->property("selectionEnd").toInt(), 5);

    QTest::keyClick(&window, Qt::Key_Right, Qt::AltModifier | Qt::ShiftModifier);
    QTRY_COMPARE(inputItem->property("cursorPosition").toInt(), 10);
    QCOMPARE(inputItem->property("selectionStart").toInt(), 5);
    QCOMPARE(inputItem->property("selectionEnd").toInt(), 10);

    inputItem->setProperty("cursorPosition", 5);
    QTest::keyClick(&window, Qt::Key_Right, Qt::AltModifier | Qt::ShiftModifier | Qt::KeypadModifier);
    QTRY_COMPARE(inputItem->property("cursorPosition").toInt(), 10);
    QCOMPARE(inputItem->property("selectionStart").toInt(), 5);
    QCOMPARE(inputItem->property("selectionEnd").toInt(), 10);

    inputItem->setProperty("cursorPosition", 16);
    QTest::keyClick(&window, Qt::Key_Left, Qt::AltModifier);
    QTRY_COMPARE(inputItem->property("cursorPosition").toInt(), 11);
    QCOMPARE(inputItem->property("selectionStart").toInt(), 11);
    QCOMPARE(inputItem->property("selectionEnd").toInt(), 11);

    inputItem->setProperty("cursorPosition", 16);
    QTest::keyClick(&window, Qt::Key_Left, Qt::AltModifier | Qt::ShiftModifier);
    QTRY_COMPARE(inputItem->property("cursorPosition").toInt(), 11);
    QCOMPARE(inputItem->property("selectionStart").toInt(), 11);
    QCOMPARE(inputItem->property("selectionEnd").toInt(), 16);
#endif
}

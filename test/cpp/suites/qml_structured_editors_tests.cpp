#include "test/cpp/whatson_cpp_regression_tests.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QtQml/qqml.h>

#include <memory>

namespace
{
    QString qmlStructuredEditorsRepositoryRootPath()
    {
        QDir repositoryRoot(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());
        repositoryRoot.cdUp();
        repositoryRoot.cdUp();
        repositoryRoot.cdUp();
        return repositoryRoot.absolutePath();
    }

    QString qmlStructuredEditorErrorString(const QList<QQmlError>& errors)
    {
        QStringList messages;
        messages.reserve(errors.size());
        for (const QQmlError& error : errors)
        {
            messages.push_back(error.toString());
        }
        return messages.join(QLatin1Char('\n'));
    }

    void addWhatSonStructuredEditorQmlImportPaths(QQmlEngine& engine, const QString& repositoryRoot)
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

    void registerStructuredEditorRuntimeQmlTypes()
    {
        static const int textFormatRendererTypeId = qmlRegisterType<ContentsTextFormatRenderer>(
            "WhatSon.App.Internal",
            1,
            0,
            "ContentsTextFormatRenderer");
        Q_UNUSED(textFormatRendererTypeId);
    }
} // namespace

void WhatSonCppRegressionTests::qmlStructuredEditors_bindPaperPaletteIntoPagePrintMode()
{
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));
    const QString documentBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentBlock.qml"));
    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString textBlockControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsDocumentTextBlockController.qml"));
    const QString agendaBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaBlock.qml"));
    const QString calloutBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutBlock.qml"));
    const QString agendaLayerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaLayer.qml"));
    const QString calloutLayerSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutLayer.qml"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));

    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(!documentBlockSource.isEmpty());
    QVERIFY(!textBlockSource.isEmpty());
    QVERIFY(!agendaBlockSource.isEmpty());
    QVERIFY(!calloutBlockSource.isEmpty());
    QVERIFY(!agendaLayerSource.isEmpty());
    QVERIFY(!calloutLayerSource.isEmpty());
    QVERIFY(!displayViewSource.isEmpty());

    QVERIFY(structuredFlowSource.contains(QStringLiteral("property bool paperPaletteEnabled: false")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("paperPaletteEnabled: documentFlow.paperPaletteEnabled")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("property bool paperPaletteEnabled: false")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("paperPaletteEnabled: documentBlock.paperPaletteEnabled")));
    QVERIFY(textBlockSource.contains(
        QStringLiteral("readonly property color editorTextColor: paperPaletteEnabled ? \"#111111\" : LV.Theme.bodyColor")));
    QVERIFY(textBlockSource.contains(QStringLiteral("paperPaletteEnabled: textBlock.paperPaletteEnabled")));
    QVERIFY(textBlockSource.contains(QStringLiteral("textColor: textBlock.editorTextColor")));

    QVERIFY(!agendaBlockSource.contains(QStringLiteral("textColor: \"#FFFFFF\"")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("color: \"#80FFFFFF\"")));
    QVERIFY(agendaBlockSource.contains(
        QStringLiteral("readonly property color taskTextColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));
    QVERIFY(calloutBlockSource.contains(
        QStringLiteral("readonly property color bodyTextColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("textColor: \"#FFFFFF\"")));
    QVERIFY(!agendaLayerSource.contains(QStringLiteral("color: \"#FFFFFF\"")));
    QVERIFY(agendaLayerSource.contains(
        QStringLiteral("readonly property color taskTextColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));
    QVERIFY(calloutLayerSource.contains(
        QStringLiteral("readonly property color textColor: paperPaletteEnabled ? \"#111111\" : \"#FFFFFF\"")));

    QVERIFY(displayViewSource.contains(QStringLiteral("paperPaletteEnabled: contentsView.showPrintEditorLayout")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_clipInlineResourceCardsToMeasuredBlockBounds()
{
    const QString resourceBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsResourceBlock.qml"));
    const QString resourceCardSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsResourceRenderCard.qml"));

    QVERIFY(!resourceBlockSource.isEmpty());
    QVERIFY(!resourceCardSource.isEmpty());

    QVERIFY(resourceBlockSource.contains(QStringLiteral("implicitHeight: resourceCard.implicitHeight")));
    QVERIFY(resourceBlockSource.contains(QStringLiteral("clip: true")));
    QVERIFY(resourceCardSource.contains(QStringLiteral("clip: true")));
    QVERIFY(!resourceCardSource.contains(QStringLiteral("clip: false")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_renderInlineStyleOverlayAtRuntime()
{
    registerStructuredEditorRuntimeQmlTypes();

    const QString repositoryRoot = qmlStructuredEditorsRepositoryRootPath();
    QQmlEngine engine;
    addWhatSonStructuredEditorQmlImportPaths(engine, repositoryRoot);

    QQmlComponent component(
        &engine,
        QUrl::fromLocalFile(
            repositoryRoot
            + QStringLiteral("/src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml")));
    if (component.status() == QQmlComponent::Error)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    const QString sourceText =
        QStringLiteral("Plain <bold>strong</bold> and <highlight>glow</highlight>");
    QVariantMap blockData;
    blockData.insert(QStringLiteral("sourceStart"), 0);
    blockData.insert(QStringLiteral("sourceEnd"), sourceText.size());
    blockData.insert(QStringLiteral("sourceText"), sourceText);

    std::unique_ptr<QObject> textBlockObject(component.createWithInitialProperties({
        {QStringLiteral("blockData"), blockData},
    }));
    if (!textBlockObject)
    {
        QFAIL(qPrintable(qmlStructuredEditorErrorString(component.errors())));
    }

    auto* textBlockItem = qobject_cast<QQuickItem*>(textBlockObject.get());
    QVERIFY(textBlockItem != nullptr);
    textBlockItem->setWidth(420);
    textBlockItem->setHeight(80);

    QQuickWindow window;
    window.resize(420, 80);
    textBlockItem->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QTRY_VERIFY(textBlockObject->property("sourceContainsInlineStyleTags").toBool());
    QTRY_VERIFY(textBlockObject->property("inlineStyleOverlayVisible").toBool());

    QObject* inlineEditor = textBlockObject->findChild<QObject*>(QStringLiteral("contentsInlineFormatEditor"));
    QVERIFY(inlineEditor != nullptr);

    QTRY_VERIFY(inlineEditor->property("showRenderedOutput").toBool());
    const QString renderedText = inlineEditor->property("renderedText").toString();
    QVERIFY(renderedText.contains(QStringLiteral("<strong style=\"font-weight:900;\">strong</strong>")));
    QVERIFY(renderedText.contains(QStringLiteral("background-color:#8A4B00")));
    QCOMPARE(inlineEditor->property("text").toString(), QStringLiteral("Plain strong and glow"));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_consumeRendererNormalizedBlocksWithoutLocalFlattening()
{
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(structuredFlowSource.contains(
        QStringLiteral("documentHost.documentBlocks = documentHost.collectionPolicy.normalizeEntries(documentFlow.documentBlocks)")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function flattenedInteractiveBlocks()")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function normalizedParsedBlocks()")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function buildFlattenedInteractiveTextGroup(")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function implicitTextBlockInteractiveFlattenCandidate(")));
    QVERIFY(structuredFlowSource.contains(
        QStringLiteral("function snapshotTokenForLogicalLine(blockEntry, logicalLines, lineIndex)")));
    QVERIFY(structuredFlowSource.contains(
        QStringLiteral("\"snapshotToken\": documentFlow.snapshotTokenForLogicalLine(blockEntry, logicalLines, index)")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_rejectStaleSourceRangeMutations()
{
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));

    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function sourceRangeMatchesCurrentSnapshot(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function replaceSourceRangeIfCurrent(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("sourceRangeMatchesCurrentSnapshot.rejected")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("safeBlock.sourceText")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"reason\": \"text-block\"")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("const expectedTaskSourceText = StructuredCursorSupport.replacementSourceText(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"reason\": \"agenda-task\"")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("const expectedCalloutSourceText = StructuredCursorSupport.replacementSourceText(")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"reason\": \"callout-text\"")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("\"allowBlankAnchor\": true")));
}

void WhatSonCppRegressionTests::qmlStructuredEditors_preserveNativeMobileInputDuringFocusedEdits()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"));
    const QString inlineEditorControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsInlineFormatEditorController.qml"));
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));
    const QString documentBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentBlock.qml"));
    const QString documentBlockControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsDocumentBlockController.qml"));
    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString textBlockControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsDocumentTextBlockController.qml"));
    const QString agendaBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaBlock.qml"));
    const QString agendaTaskRowControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsAgendaTaskRowController.qml"));
    const QString calloutBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutBlock.qml"));
    const QString calloutBlockControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsCalloutBlockController.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(!inlineEditorControllerSource.isEmpty());
    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(!documentBlockSource.isEmpty());
    QVERIFY(!documentBlockControllerSource.isEmpty());
    QVERIFY(!textBlockSource.isEmpty());
    QVERIFY(!textBlockControllerSource.isEmpty());
    QVERIFY(!agendaBlockSource.isEmpty());
    QVERIFY(!agendaTaskRowControllerSource.isEmpty());
    QVERIFY(!calloutBlockSource.isEmpty());
    QVERIFY(!calloutBlockControllerSource.isEmpty());

    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool nativeTextInputPriority: true")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("contentsView.mobileHost || Qt.platform.os === \"ios\"")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool noteDocumentShortcutSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool noteDocumentTagManagementShortcutSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function nativeTextInputSessionOwnsKeyboard()")));
    QVERIFY(displayViewSource.contains(QStringLiteral("structuredDocumentFlow.nativeCompositionActive()")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("context: Qt.WindowShortcut\n                        enabled: contentsView.noteDocumentCommandSurfaceEnabled")));

    QVERIFY(inlineEditorSource.contains(QStringLiteral("EditorInputModel.ContentsInlineFormatEditorController")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("function shouldRejectFocusedProgrammaticTextSync(nextText)")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("property bool localTextEditSinceFocus: false")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("property var shortcutKeyPressHandler")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("property var modifierVerticalNavigationHandler")));

    QVERIFY(structuredFlowSource.contains(QStringLiteral("property bool nativeTextInputPriority: false")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("if (documentFlow.nativeTextInputPriority)\n            return")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function nativeCompositionActive()")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function noteActiveBlockCursorInteraction(blockIndex)")));
    QVERIFY(structuredFlowSource.contains(
        QStringLiteral("onCursorInteraction: documentFlow.noteActiveBlockCursorInteraction(blockHost.blockIndex)")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("expectedPreviousSourceText")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("sourceStart + expectedSourceText.length")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("expectedPreviousText")));

    QVERIFY(documentBlockSource.contains(QStringLiteral("property bool nativeTextInputPriority: false")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("readonly property bool inputMethodComposing")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("readonly property string preeditText")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("nativeTextInputPriority: documentBlock.nativeTextInputPriority")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("signal cursorInteraction()")));
    QVERIFY(documentBlockControllerSource.contains(QStringLiteral("function onCursorInteraction()")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("EditorInputModel.ContentsDocumentBlockController")));
    QVERIFY(documentBlockControllerSource.contains(QStringLiteral("function handleAtomicTagManagementKeyPress(event)")));

    QVERIFY(textBlockSource.contains(QStringLiteral("property bool nativeTextInputPriority: false")));
    QVERIFY(textBlockSource.contains(QStringLiteral("EditorInputModel.ContentsDocumentTextBlockController")));
    QVERIFY(textBlockControllerSource.contains(QStringLiteral("property string liveEditSourceText: \"\"")));
    QVERIFY(textBlockControllerSource.contains(QStringLiteral("syncLiveEditSnapshotFromHost")));
    QVERIFY(textBlockControllerSource.contains(QStringLiteral("readonly property bool sourceContainsInlineStyleTags")));
    QVERIFY(textBlockSource.contains(
        QStringLiteral("sourceText: textBlock.sourceContainsInlineStyleTags ? textBlock.authoritativeSourceText() : \"\"")));
    QVERIFY(!textBlockSource.contains(
        QStringLiteral("sourceText: textBlock.inlineStyleOverlayVisible ? textBlock.authoritativeSourceText() : \"\"")));
    QVERIFY(textBlockSource.contains(QStringLiteral("preferNativeInputHandling: true")));
    QVERIFY(textBlockSource.contains(QStringLiteral("inputMethodHints: Qt.ImhNone")));
    QVERIFY(textBlockSource.contains(QStringLiteral("mouseSelectionMode: TextEdit.SelectCharacters")));
    QVERIFY(textBlockSource.contains(QStringLiteral("overwriteMode: false")));
    QVERIFY(textBlockSource.contains(QStringLiteral("persistentSelection: true")));
    QVERIFY(textBlockSource.contains(QStringLiteral("selectByKeyboard: true")));
    QVERIFY(textBlockSource.contains(QStringLiteral("signal cursorInteraction()")));
    QVERIFY(textBlockControllerSource.contains(QStringLiteral("controller.textBlock.cursorInteraction();")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("onCursorPositionChanged: {\n            if (focused)\n                textBlock.activated()")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("modifierVerticalNavigationHandler")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("shortcutKeyPressHandler: function")));

    QVERIFY(agendaBlockSource.contains(QStringLiteral("property bool nativeTextInputPriority: false")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("EditorInputModel.ContentsAgendaTaskRowController")));
    QVERIFY(agendaTaskRowControllerSource.contains(QStringLiteral("property string liveTaskText: \"\"")));
    QVERIFY(agendaTaskRowControllerSource.contains(QStringLiteral("syncLiveTaskTextFromHost")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("signal taskTextChanged(var taskData, string text, int cursorPosition, string expectedPreviousText)")));
    QVERIFY(agendaTaskRowControllerSource.contains(QStringLiteral("controller.agendaBlock.taskTextChanged(")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("preferNativeInputHandling: true")));
    QVERIFY(agendaBlockSource.contains(QStringLiteral("signal cursorInteraction()")));
    QVERIFY(agendaTaskRowControllerSource.contains(QStringLiteral("controller.agendaBlock.cursorInteraction();")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("modifierVerticalNavigationHandler")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("shortcutKeyPressHandler: function")));

    QVERIFY(calloutBlockSource.contains(QStringLiteral("property bool nativeTextInputPriority: false")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("EditorInputModel.ContentsCalloutBlockController")));
    QVERIFY(calloutBlockControllerSource.contains(QStringLiteral("property string liveText: \"\"")));
    QVERIFY(calloutBlockControllerSource.contains(QStringLiteral("syncLiveTextFromHost")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("signal textChanged(string text, int cursorPosition, string expectedPreviousText)")));
    QVERIFY(calloutBlockControllerSource.contains(QStringLiteral("controller.calloutBlock.textChanged(")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("preferNativeInputHandling: true")));
    QVERIFY(calloutBlockSource.contains(QStringLiteral("signal cursorInteraction()")));
    QVERIFY(calloutBlockControllerSource.contains(QStringLiteral("controller.calloutBlock.cursorInteraction();")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("modifierVerticalNavigationHandler")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("shortcutKeyPressHandler: function")));
}

void WhatSonCppRegressionTests::qmlEditorInputPolicyAdapter_centralizesNativeInputDecisions()
{
    const QString policyAdapterSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsEditorInputPolicyAdapter.qml"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString mutationControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayMutationController.qml"));
    const QString inlineEditorSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml"));
    const QString inlineEditorControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsInlineFormatEditorController.qml"));
    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString textBlockControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsDocumentTextBlockController.qml"));

    QVERIFY(!policyAdapterSource.isEmpty());
    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!mutationControllerSource.isEmpty());
    QVERIFY(!inlineEditorSource.isEmpty());
    QVERIFY(!inlineEditorControllerSource.isEmpty());
    QVERIFY(!textBlockSource.isEmpty());
    QVERIFY(!textBlockControllerSource.isEmpty());

    QVERIFY(policyAdapterSource.contains(QStringLiteral("readonly property bool nativeCompositionActive")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("readonly property bool nativeTextInputSessionActive")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("|| adapter.editorInputFocused")));
    QVERIFY(!policyAdapterSource.contains(
        QStringLiteral("|| (adapter.nativeTextInputPriority\n                                                             && adapter.editorInputFocused)")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("readonly property bool shortcutSurfaceEnabled")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("readonly property bool tagManagementShortcutSurfaceEnabled")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("&& adapter.noteDocumentCommandSurfaceEnabled")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("&& !adapter.nativeCompositionActive")));
    QVERIFY(!policyAdapterSource.contains(QStringLiteral("&& adapter.shortcutSurfaceEnabled")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("readonly property bool contextMenuLongPressEnabled")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("readonly property bool contextMenuSurfaceEnabled")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("function programmaticTextSyncPolicy(")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("function shouldDeferProgrammaticTextSync(")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("function shouldApplyProgrammaticTextSync(")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("function shouldRestoreFocusForMutation(")));
    QVERIFY(policyAdapterSource.contains(QStringLiteral("reason === \"text-edit\"")));

    QVERIFY(displayViewSource.contains(QStringLiteral("EditorInputModel.ContentsEditorInputPolicyAdapter {\n        id: editorInputPolicyAdapter")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool contextMenuLongPressEnabled: editorInputPolicyAdapter.contextMenuLongPressEnabled")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentShortcutSurfaceEnabled: editorInputPolicyAdapter.shortcutSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentTagManagementShortcutSurfaceEnabled: editorInputPolicyAdapter.tagManagementShortcutSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentContextMenuSurfaceEnabled: editorInputPolicyAdapter.contextMenuSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("return editorInputPolicyAdapter.nativeTextInputSessionActive;")));
    QVERIFY(mutationControllerSource.contains(
        QStringLiteral("controller.editorInputPolicyAdapter.shouldRestoreFocusForMutation(")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("readonly property bool contextMenuLongPressEnabled: contentsView.nativeTextInputPriority")));
    QVERIFY(!displayViewSource.contains(
        QStringLiteral("return contentsView.nativeTextInputPriority && contentsView.editorInputFocused;")));

    QVERIFY(inlineEditorSource.contains(QStringLiteral("EditorInputModel.ContentsInlineFormatEditorController")));
    QVERIFY(!inlineEditorSource.contains(QStringLiteral("EditorInputModel.ContentsEditorInputPolicyAdapter {\n        id: inlineInputPolicyAdapter")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("ContentsEditorInputPolicyAdapter {\n        id: inlineInputPolicyAdapter")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("function programmaticTextSyncPolicy(nextText)")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("inlineInputPolicyAdapter.programmaticTextSyncPolicy(")));
    QVERIFY(inlineEditorControllerSource.contains(QStringLiteral("const syncPolicy = controller.programmaticTextSyncPolicy(normalizedText);")));
    QVERIFY(!inlineEditorControllerSource.contains(QStringLiteral("return control.nativeCompositionActive();")));
    QVERIFY(!inlineEditorControllerSource.contains(QStringLiteral("control._localTextEditSinceFocus = false;\n            return false;")));

    QVERIFY(textBlockControllerSource.contains(QStringLiteral("\"reason\": \"text-edit\"")));
}

void WhatSonCppRegressionTests::qmlEditorViewDirectory_containsOnlyViewSurfaceFiles()
{
    const QDir viewDirectory(QStringLiteral("src/app/qml/view/content/editor"));
    QVERIFY(viewDirectory.exists());

    QStringList actualFiles = viewDirectory.entryList(
        QStringList{QStringLiteral("*.qml"), QStringLiteral("*.js")},
        QDir::Files,
        QDir::Name);
    const QStringList expectedFiles{
        QStringLiteral("ContentsAgendaBlock.qml"),
        QStringLiteral("ContentsAgendaLayer.qml"),
        QStringLiteral("ContentsBreakBlock.qml"),
        QStringLiteral("ContentsCalloutBlock.qml"),
        QStringLiteral("ContentsCalloutLayer.qml"),
        QStringLiteral("ContentsDisplayView.qml"),
        QStringLiteral("ContentsDocumentBlock.qml"),
        QStringLiteral("ContentsDocumentTextBlock.qml"),
        QStringLiteral("ContentsGutterLayer.qml"),
        QStringLiteral("ContentsImageResourceFrame.qml"),
        QStringLiteral("ContentsInlineFormatEditor.qml"),
        QStringLiteral("ContentsMinimapLayer.qml"),
        QStringLiteral("ContentsResourceBlock.qml"),
        QStringLiteral("ContentsResourceEditorView.qml"),
        QStringLiteral("ContentsResourceLayer.qml"),
        QStringLiteral("ContentsResourceRenderCard.qml"),
        QStringLiteral("ContentsResourceViewer.qml"),
        QStringLiteral("ContentsStructuredDocumentFlow.qml"),
    };
    QCOMPARE(actualFiles, expectedFiles);

    const QString editorCmakeSource = readUtf8SourceFile(QStringLiteral("src/app/models/editor/CMakeLists.txt"));
    QVERIFY(editorCmakeSource.contains(QStringLiteral("whatson_app_register_directory_qml")));

    const QString viewmodelCmakeSource = readUtf8SourceFile(QStringLiteral("src/app/viewmodel/CMakeLists.txt"));
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString eventPumpSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayEventPump.qml"));
    const QString inputCommandSurfaceSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayInputCommandSurface.qml"));
    const QString mutationControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayMutationController.qml"));
    const QString geometryControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/display/ContentsDisplayGeometryController.qml"));
    const QString displayControllerBridgeHeader = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplayControllerBridgeViewModel.hpp"));
    const QString mutationViewModelHeader = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplayMutationViewModel.hpp"));
    const QString mutationViewModelCpp = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplayMutationViewModel.cpp"));
    const QString presentationViewModelHeader = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplayPresentationViewModel.hpp"));
    const QString presentationViewModelCpp = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplayPresentationViewModel.cpp"));
    const QString selectionMountViewModelHeader = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplaySelectionMountViewModel.hpp"));
    const QString selectionMountViewModelCpp = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplaySelectionMountViewModel.cpp"));
    const QString geometryViewModelHeader = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplayGeometryViewModel.hpp"));
    const QString geometryViewModelCpp = readUtf8SourceFile(
        QStringLiteral("src/app/viewmodel/editor/display/ContentsDisplayGeometryViewModel.cpp"));
    QVERIFY(!displayControllerBridgeHeader.isEmpty());
    QVERIFY(!mutationViewModelHeader.isEmpty());
    QVERIFY(!mutationViewModelCpp.isEmpty());
    QVERIFY(!presentationViewModelHeader.isEmpty());
    QVERIFY(!presentationViewModelCpp.isEmpty());
    QVERIFY(!selectionMountViewModelHeader.isEmpty());
    QVERIFY(!selectionMountViewModelCpp.isEmpty());
    QVERIFY(!geometryViewModelHeader.isEmpty());
    QVERIFY(!geometryViewModelCpp.isEmpty());
    QVERIFY(!viewmodelCmakeSource.contains(QStringLiteral("whatson_app_register_directory_qml")));
    QVERIFY(displayControllerBridgeHeader.contains(QStringLiteral("Q_PROPERTY(QObject* controller READ controller WRITE setController NOTIFY controllerChanged)")));
    QVERIFY(displayControllerBridgeHeader.contains(QStringLiteral("public slots:")));
    QVERIFY(displayControllerBridgeHeader.contains(QStringLiteral("void controllerChanged();")));
    QVERIFY(mutationViewModelHeader.contains(QStringLiteral("class ContentsDisplayMutationViewModel")));
    QVERIFY(mutationViewModelHeader.contains(QStringLiteral("public slots:")));
    QVERIFY(mutationViewModelCpp.contains(QStringLiteral("invokeControllerBool(\"applyDocumentSourceMutation\"")));
    QVERIFY(presentationViewModelHeader.contains(QStringLiteral("class ContentsDisplayPresentationViewModel")));
    QVERIFY(presentationViewModelHeader.contains(QStringLiteral("public slots:")));
    QVERIFY(presentationViewModelCpp.contains(QStringLiteral("invokeControllerVoid(\"scheduleDocumentPresentationRefresh\"")));
    QVERIFY(selectionMountViewModelHeader.contains(QStringLiteral("class ContentsDisplaySelectionMountViewModel")));
    QVERIFY(selectionMountViewModelHeader.contains(QStringLiteral("public slots:")));
    QVERIFY(selectionMountViewModelCpp.contains(QStringLiteral("invokeControllerVoid(\"scheduleSelectionModelSync\"")));
    QVERIFY(geometryViewModelHeader.contains(QStringLiteral("class ContentsDisplayGeometryViewModel")));
    QVERIFY(geometryViewModelHeader.contains(QStringLiteral("public slots:")));
    QVERIFY(geometryViewModelCpp.contains(QStringLiteral("invokeControllerVoid(\"refreshMinimapSnapshot\")")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayEventPump")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayInputCommandSurface")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayPresentationController")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplayPresentationViewModel")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayMutationController")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplayMutationViewModel")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplaySelectionMountController")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplaySelectionMountViewModel")));
    QVERIFY(displayViewSource.contains(QStringLiteral("EditorDisplayModel.ContentsDisplayGeometryController")));
    QVERIFY(displayViewSource.contains(QStringLiteral("ContentsDisplayGeometryViewModel")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("Timer {")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("Connections {")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("Shortcut {")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("MouseArea {")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("LV.ContextMenu")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("Timer {")));
    QVERIFY(eventPumpSource.contains(QStringLiteral("Connections {")));
    QVERIFY(inputCommandSurfaceSource.contains(QStringLiteral("Shortcut {")));
    QVERIFY(inputCommandSurfaceSource.contains(QStringLiteral("LV.ContextMenu")));
    QVERIFY(mutationControllerSource.contains(QStringLiteral("function applyDocumentSourceMutation(")));
    QVERIFY(geometryControllerSource.contains(QStringLiteral("function refreshMinimapSnapshot()")));

    const QDir repositoryRoot(qmlStructuredEditorsRepositoryRootPath());
    QDirIterator viewmodelQmlFiles(
        repositoryRoot.filePath(QStringLiteral("src/app/viewmodel")),
        QStringList{QStringLiteral("*.qml")},
        QDir::Files | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories);
    QVERIFY(!viewmodelQmlFiles.hasNext());
}

void WhatSonCppRegressionTests::qmlStructuredEditors_lockCustomInputToTagManagementOnly()
{
    const QString displayViewSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDisplayView.qml"));
    const QString structuredFlowSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsStructuredDocumentFlow.qml"));
    const QString documentBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentBlock.qml"));
    const QString documentBlockControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsDocumentBlockController.qml"));
    const QString breakBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsBreakBlock.qml"));
    const QString breakBlockControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsBreakBlockController.qml"));
    const QString textBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml"));
    const QString agendaBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsAgendaBlock.qml"));
    const QString calloutBlockSource = readUtf8SourceFile(
        QStringLiteral("src/app/qml/view/content/editor/ContentsCalloutBlock.qml"));
    const QString selectionControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsEditorSelectionController.qml"));
    const QString typingControllerSource = readUtf8SourceFile(
        QStringLiteral("src/app/models/editor/input/ContentsEditorTypingController.qml"));

    QVERIFY(!displayViewSource.isEmpty());
    QVERIFY(!structuredFlowSource.isEmpty());
    QVERIFY(!documentBlockSource.isEmpty());
    QVERIFY(!documentBlockControllerSource.isEmpty());
    QVERIFY(!breakBlockSource.isEmpty());
    QVERIFY(!breakBlockControllerSource.isEmpty());
    QVERIFY(!textBlockSource.isEmpty());
    QVERIFY(!agendaBlockSource.isEmpty());
    QVERIFY(!calloutBlockSource.isEmpty());
    QVERIFY(!selectionControllerSource.isEmpty());
    QVERIFY(!typingControllerSource.isEmpty());

    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool editorCustomTextInputEnabled: false")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool editorTagManagementInputEnabled: true")));
    QVERIFY(displayViewSource.contains(QStringLiteral("readonly property bool noteDocumentTagManagementShortcutSurfaceEnabled")));
    QVERIFY(displayViewSource.contains(QStringLiteral("function handleTagManagementShortcutKeyPress(event)")));
    QVERIFY(displayViewSource.contains(QStringLiteral("tagManagementShortcutKeyPressHandler: function (event)")));
    QVERIFY(displayViewSource.contains(
        QStringLiteral("readonly property bool noteDocumentTagManagementShortcutSurfaceEnabled: editorInputPolicyAdapter.tagManagementShortcutSurfaceEnabled")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("function handleInlineFormatShortcutKeyPress(event)")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("handlePlainEnterKeyPress(event)")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("handleTagAwareDeleteKeyPress(event)")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("queueMarkdownListMutation")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("sequence: \"Meta+Shift+7\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("sequence: \"Alt+Shift+7\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("sequence: \"Meta+Shift+8\"")));
    QVERIFY(!displayViewSource.contains(QStringLiteral("sequence: \"Alt+Shift+8\"")));

    QVERIFY(structuredFlowSource.contains(QStringLiteral("property var tagManagementShortcutKeyPressHandler: null")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function blockEntryIsTagManagedAtomicBlock(blockEntry)")));
    QVERIFY(structuredFlowSource.contains(QStringLiteral("function handleActiveTagManagementKeyPress(event)")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("property var shortcutKeyPressHandler")));
    QVERIFY(!structuredFlowSource.contains(QStringLiteral("function handleActiveBlockDeleteKeyPress(event)")));

    QVERIFY(documentBlockSource.contains(QStringLiteral("property var tagManagementShortcutKeyPressHandler: null")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("function handleAtomicTagManagementKeyPress(event)")));
    QVERIFY(documentBlockSource.contains(QStringLiteral("function handleAtomicTagDeleteKeyPress(event)")));
    QVERIFY(documentBlockControllerSource.contains(QStringLiteral("modifiers !== Qt.NoModifier")));
    QVERIFY(documentBlockControllerSource.contains(QStringLiteral("modifiers === Qt.MetaModifier")));
    QVERIFY(!documentBlockSource.contains(QStringLiteral("Qt.AltModifier")));
    QVERIFY(!documentBlockSource.contains(QStringLiteral("macModifierVerticalNavigation")));
    QVERIFY(!documentBlockSource.contains(QStringLiteral("(Qt.AltModifier | Qt.MetaModifier) !== 0")));
    QVERIFY(!documentBlockControllerSource.contains(QStringLiteral("return !!blockItem.handleDeleteKeyPress(event)")));

    QVERIFY(breakBlockSource.contains(QStringLiteral("property var tagManagementShortcutKeyPressHandler: null")));
    QVERIFY(breakBlockControllerSource.contains(QStringLiteral("modifiers !== Qt.NoModifier")));
    QVERIFY(breakBlockControllerSource.contains(QStringLiteral("modifiers === Qt.MetaModifier")));
    QVERIFY(!breakBlockSource.contains(QStringLiteral("Qt.AltModifier")));
    QVERIFY(!breakBlockSource.contains(QStringLiteral("macModifierVerticalNavigation")));
    QVERIFY(!breakBlockSource.contains(QStringLiteral("(Qt.AltModifier | Qt.MetaModifier) !== 0")));
    QVERIFY(!breakBlockSource.contains(QStringLiteral("property var shortcutKeyPressHandler")));

    QVERIFY(!textBlockSource.contains(QStringLiteral("Keys.onPressed")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("function handleDeleteKeyPress(event)")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("function handleAtomicBlockBoundaryKeyPress(event)")));
    QVERIFY(!textBlockSource.contains(QStringLiteral("shortcutKeyPressHandler")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("function handleBoundaryKeyPress(event)")));
    QVERIFY(!agendaBlockSource.contains(QStringLiteral("shortcutKeyPressHandler")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("function handleBoundaryKeyPress(event)")));
    QVERIFY(!calloutBlockSource.contains(QStringLiteral("shortcutKeyPressHandler")));

    QVERIFY(!selectionControllerSource.contains(QStringLiteral("handleInlineFormatShortcutKeyPress")));
    QVERIFY(!selectionControllerSource.contains(QStringLiteral("queueStructuredShortcutMutation")));
    QVERIFY(!selectionControllerSource.contains(QStringLiteral("queuedStructuredShortcutMutationKeys")));
    QVERIFY(!selectionControllerSource.contains(QStringLiteral("queueMarkdownListMutation")));
    QVERIFY(!selectionControllerSource.contains(QStringLiteral("queuedMarkdownListMutationKeys")));

    QVERIFY(!typingControllerSource.contains(QStringLiteral("handlePlainEnterKeyPress")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("handleTagAwareDeleteKeyPress")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("applyDirectRawSourceReplacement")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("continuedListInsertion")));
    QVERIFY(!typingControllerSource.contains(QStringLiteral("markerOnlyListBreakInsertion")));
}
